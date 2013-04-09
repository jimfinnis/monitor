/**
 * \file
 * Implementation of configuration file parsing.
 */

#include <QList>
#include <QString>
#include <QDesktopWidget>
#include <QFile>

#include "tokeniser.h"
#include "tokens.h"
#include "config.h"
#include "exception.h"
#include "app.h"

#include "expr.h"

#include "widgets/gauge.h"
#include "widgets/graph.h"
#include "widgets/compass.h"
#include "widgets/status.h"
#include "widgets/number.h"
#include "widgets/map.h"
#include "widgets/switch.h"

#include "datamgr.h"

int ConfigManager::port = -1;
int ConfigManager::udpSendPort = 33333;
char ConfigManager::udpSendAddr[256];
float ConfigManager::sendInterval = 2;

bool ConfigManager::inverse=false;


/// a structure to hold variable names and types for linkage
struct LinkedVarEntry {
    char name[64];
    int type;
    float minVal,maxVal;
    
    LinkedVarEntry(const char *n,int t,float mn,float mx){
        strcpy(name,n);
        type = t;
        minVal=mn;
        maxVal=mx;
    }
};

/// used to hold a list of linked var names
static QList<LinkedVarEntry> linkedVars;

/// tokeniser object
static Tokeniser tok;


class MyTError : public ITokeniserErrorHandler{
    virtual void HandleTokeniserError(class Tokeniser *t){
        throw UnexpException(t);
    }
};

static MyTError tokerrorhandler;

/// handy function for creating a buffer of any type - mn and mx will be cast from float or just ignored,
/// whatever is appropriate for the type.
static RawDataBuffer *createVar(int type, const char *s, int size,float mn=0,float mx=1){
    
    switch(type){
    case T_NAMEFLOAT:
        if(DataManager::findFloatBuffer(s))
            throw Exception().set("variable %s already exists at line %d of config file",s,tok.getline());
        else {
            printf("Adding var %s, size %d, range (%f,%f)\n",s,size,mn,mx);
            return DataManager::createFloatBuffer(s,size,mn,mx);
        }
        break;
    default:
        throw Exception().set("unsupported type for variable %s at line %d of config file",s,tok.getline());
    }
    return NULL;
}

/// parse a set of variable definitions of the form 
/// \code
/// var <type> <name> <bufsize>, <type> <name> <bufsize>...
/// \endcode
/// also permits the creation of linked variables:
/// \code
/// var linked (<type> <name>,<type> <name>...) <bufsize>
/// \endcode

static void parseVars(){
    char buf[256];
    int size,type;
    RawDataBuffer *linkvar,*b;
    tok.getnextcheck(T_OCURLY);
    for(;;){
        bool autoRange=false;
        float mn=0,mx=0;
        
        int t = tok.getnext();
        switch(t){
        case T_NAMEFLOAT:
            // it's a valid type
            tok.getnextident(buf); // get name
            size = tok.getnextint(); // and size
            // now get the range
            tok.getnextcheck(T_RANGE);
            switch(tok.getnext()){
            case T_INT:
            case T_FLOAT:
                mn = tok.getfloat();
                tok.getnextcheck(T_TO);
                mx = tok.getnextfloat();
                break;
            case T_AUTO:
                autoRange=true;
                break;
            default:
                throw UnexpException(&tok,"number or 'auto'");
                break;
            }
            b=createVar(T_NAMEFLOAT,buf,size,mn,mx);
            if(autoRange)
                ((DataBuffer<float>*)b)->setAutoRange();
            break;
        case T_NAMEBOOL:
            throw Exception("bools not currently supported");
            // it's a valid type
            tok.getnextident(buf); // get name
            size = tok.getnextint(); // and size
            createVar(T_NAMEBOOL,buf,size);
            break;
        case T_LINKED:
            if(tok.getnext()!=T_OPREN)
                throw UnexpException(&tok,"( after linked");
            linkedVars.clear();
            for(;;){
                // get type
                type = tok.getnext();
                // get name
                tok.getnextident(buf);
                // get range if a float
                if(type==T_NAMEFLOAT){
                    tok.getnextcheck(T_RANGE);
                    mn = tok.getnextfloat();
                    tok.getnextcheck(T_TO);
                    mx = tok.getnextfloat();
                } else {
                    mn = 0;
                    mx = 1;
                }

                // add to a list!
                linkedVars.append(LinkedVarEntry(buf,type,mn,mx));
                if(tok.getnext()!=T_COMMA){
                    tok.rewind();
                    break;
                }
            }
            if(tok.getnext()!=T_CPREN)
                throw UnexpException(&tok,") after linked var list");
            if(tok.getnext()!=T_INT)
                throw UnexpException(&tok,"buffer size after linked var list");
            size = tok.getint();

            linkvar = NULL;
            for(int i=0;i<linkedVars.size();i++){
                b = createVar(linkedVars[i].type,
                              linkedVars[i].name,
                              size,
                              linkedVars[i].minVal,
                              linkedVars[i].maxVal);
                if(linkvar)
                    linkvar->link(b);
                else
                    linkvar = b;
            }
            break;
        case T_CCURLY:
            return;
        default:
            throw UnexpException(&tok);
        }
    }
}

static void parseFrame(QWidget *parent);

static void parseContainer(QWidget *parent){
    // now start reading widgets
    
    for(;;){
        switch(tok.getnext()){
        case T_FRAME:
            parseFrame(parent);
            break;
        case T_GAUGE:
            new Gauge(parent,&tok);
            break;
        case T_NUMBER:
            new Number(parent,&tok);
            break;
        case T_COMPASS:
            new Compass(parent,&tok);
            break;
        case T_GRAPH:
            new Graph(parent,&tok);
            break;
        case T_MAP:
            new MapWidget(parent,&tok);
            break;
        case T_STATUS:
            new StatusBlockWrapper(parent,&tok);
            break;
        case T_SWITCH:
            new Switch(parent,&tok);
            break;
        case T_CCURLY:
            return;
        default:
            throw UnexpException(&tok,"widget name");
        }
    }
}

static void parseFrame(QWidget *parent){
    // first parse the pos block
    
    ConfigRect pos = ConfigManager::parseRect();
    
    // followed by some optional stuff
    bool borderless=false;
    int spacing=0;
    bool done=false;
    
    while(!done){
        switch(tok.getnext()){
        case T_BORDERLESS:
            borderless=true;
            break;
        case T_SPACING:
            spacing=tok.getnextint();
            break;
        case T_OCURLY:
            done = true;
            break;
        default:
            throw UnexpException(&tok,"frame option or {");
        }
    }
    
    // create frame and layout
    QFrame *f = new QFrame;

    f->setFrameStyle(borderless?
                     QFrame::NoFrame:
                     QFrame::Panel);
    QGridLayout *l = new QGridLayout;
    l->setSpacing(spacing);
    f->setLayout(l);
    
    
    parseContainer(f);
    
    // add to the parent's layout
    ((QGridLayout*)parent->layout())->addWidget(f,pos.y,pos.x,pos.h,pos.w);
}

static void parseWindow(){
    // option defaults
    bool fullScreen = false; // should it be fullscreen?
    // what size? (default is fit around widgets. Ignored for fullscreen.)
    int width=-1,height=-1; 
    // if set, move the window to a screen of the given dimensions
    int swidth=-1,sheight=-1;
    // title if any
    char title[256];
    title[0]=0;
    
    // set this window to not inverse
    ConfigManager::inverse=false;
    
    // get window options
    bool done = false;
    while(!done){
        switch(tok.getnext()){
        case T_OCURLY:
            done = true;
            break;
        case T_TITLE:
            tok.getnextstring(title);
            break;
        case T_INVERSE:
            ConfigManager::inverse=true;
            break;
        case T_FULLSCREEN:
            fullScreen = true;
            break;
        case T_SIZE: // size of window if not fullscreen
            width = tok.getnextint();
            tok.getnextcheck(T_COMMA);
            height = tok.getnextint();
            break;
        case T_SCREEN: // move to a screen of given dimensions
            swidth = tok.getnextint();
            tok.getnextcheck(T_COMMA);
            sheight = tok.getnextint();
            break;
        }
    }
        
    // create a window
    QMainWindow *w = getApp()->createWindow();
    ConfigManager::setStyle(w);
    // and parse the contents
    parseContainer(w->centralWidget());
    
    if(*title){
        w->setWindowTitle(title);
    }
    
    // move the window if we want to
    if(swidth>0){
        QDesktopWidget *dt = QApplication::desktop();
        QRect r;
        int i;
        for(i=0;i<dt->screenCount();i++){
            r = dt->screenGeometry(i);
            printf("Found display : %d x %d\n",r.width(),r.height());
            if(r.width() == swidth && r.height()==sheight)
                break;
        }
        if(i==dt->screenCount())
            throw Exception().set("could not find display of %d x %d",swidth,sheight);
        w->move(r.topLeft());
    }
    
    
    // finally show the window and resize if required
    if(fullScreen)
        w->showFullScreen();
    else {
        if(width>0)
            w->resize(width,height);
        w->show();
    }
}

ConfigRect ConfigManager::parseRect(){
    ConfigRect r;
    r.x = tok.getnextint(); // either just x,y..
    tok.getnextcheck(T_COMMA);
    r.y = tok.getnextint();
    if(tok.getnext()==T_COMMA){ // or x,y,w,h
        r.w = tok.getnextint();
        tok.getnextcheck(T_COMMA);
        r.h = tok.getnextint();
    }else{
        tok.rewind();
        r.w = 1; r.h = 1;
    }
    return r;
}

DataBuffer<float> *ConfigManager::parseFloatSource(){
    
    DataBuffer<float> *b;
    char buf[256];
    
    switch(tok.getnext()){
    case T_VAR:
        tok.getnextident(buf);
        b = DataManager::findFloatBuffer(buf);
        if(!b)
            throw Exception().set("undefined variable '%s'",buf);
        break;
    case T_EXPR:
        tok.getnextstring(buf);
        // OK, we're going to lose a reference to this, but such is life.
        // In this version we never delete expressions anyway.
        b = (new Expression(buf))->buffer;
        // now parse the extra bits
        tok.getnextcheck(T_RANGE);
        float mn,mx;
        switch(tok.getnext()){
        case T_INT:
        case T_FLOAT:
            mn = tok.getfloat();
            tok.getnextcheck(T_TO);
            mx = tok.getnextfloat();
            b->setMinMax(mn,mx);
            break;
        case T_AUTO:
            b->setAutoRange();
            break;
        default:
            throw UnexpException(&tok,"expected number or 'auto'");
        }
        break;
    default:
        throw UnexpException(&tok,"'var' or 'expr'");
    }
    return b;
}

void ConfigManager::parseFile(QString fname){
    
    strcpy(udpSendAddr,"127.0.0.1");
    tok.init();
    
    tok.seterrorhandler(&tokerrorhandler);
    tok.settokens(tokens);
    tok.setcommentlinesequence("#");
    
    // read the entire file!
    QFile file(fname);
    if(!file.open(QIODevice::ReadOnly))
        throw Exception().set("could not open config file");
    QByteArray b = file.readAll();
    if(b.isNull() || b.isEmpty())
        throw Exception().set("could not read config file");
    b.append((char)0);
    file.close();
    
    const char *data = b.constData();
    
    tok.reset(data);
    
    bool done = false;
    while(!done){
        // at the top level we parse frames and
        // variables
        
        int t = tok.getnext();
        switch(t){
        case T_VAR:
            parseVars();
            break;
        case T_WINDOW:
            parseWindow();
            break;
        case T_END:
            done=true;
            break;
        case T_PORT:
            port = tok.getnextint();
            break;
        case T_SENDPORT:
            udpSendPort = tok.getnextint();
            break;
        case T_SENDADDR:
            tok.getnextstring(udpSendAddr);
            break;
        case T_VALIDTIME:
            DataManager::dataValidInterval = tok.getnextfloat();
            break;
        case T_SENDINTERVAL:
            sendInterval = tok.getnextfloat();
            break;
        default:
            throw UnexpException(&tok,"'var', 'frame' or end of file");
        }
    }
    
}

QColor ConfigManager::parseColour(QColor deflt){
    switch(tok.getnext()){
    case T_IDENT:
    case T_STRING:
    case T_RED:
    case T_BLUE:
    case T_GREEN:
    case T_YELLOW:
    case T_BLACK:
        return QColor(tok.getstring());
        break;
    case T_DEFAULT:
        return deflt;
    default:
        throw UnexpException(&tok,"colour name or \"#rgb\"");
    }
}


void ConfigManager::setStyle(QWidget *w){
    if(inverse)
        w->setStyleSheet("background-color: white; color:black;");
    else
        w->setStyleSheet("background-color: black; color:white;");
}    
