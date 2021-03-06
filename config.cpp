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
#include "widgets/momentary.h"
#include "widgets/slider.h"

#include "waypoint/waypoint.h"

#if DIAMOND
#include "diamond.h"
#endif

#include "datamgr.h"

int ConfigManager::port = -1;
int ConfigManager::udpSendPort = DEFAULT_SENDPORT;

char ConfigManager::udpSendAddr[256];
float ConfigManager::sendInterval = 2;
int ConfigManager::graphicalUpdateInterval=2000;

bool ConfigManager::inverse=false;

#if DIAMOND
QMap<DiamondTopicKey,QString> diamondMap;
QSet<QString> diamondSet; // set of topics subscribed
#endif

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


extern bool traceTokeniser;

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
            throw Exception(tok.getline()).set("variable %s already exists",s);
        else {
            printf("Adding var %s, size %d, range (%f,%f)\n",s,size,mn,mx);
            return DataManager::createFloatBuffer(s,size,mn,mx);
        }
        break;
    default:
        throw ParseException(&tok).set("unsupported type for variable %s",s);
    }
    return NULL;
}

static void parseAVar(bool diamond){
    // it's a valid type
    char buf[256];
    bool autoRange=false;
    float mn=0,mx=0;
    int size;
    RawDataBuffer *b;
    tok.getnextident(buf); // get name
    
    // if diamond, get topic and datum index
    char tname[256];
    int idx;
#if DIAMOND
    if(diamond){
        tok.getnextcheck(T_TOPIC);
        tok.getnextstring(tname);
        tok.getnextcheck(T_COMMA);
        idx = tok.getnextint();
        diamondMap[DiamondTopicKey(tname,idx)]=QString(buf);
        diamondapparatus::subscribe(tname);
        if(!diamondSet.contains(tname))
            diamondSet.insert(tname);
    }
#endif    
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
}

void parseLinkedVars(bool diamond){
    char buf[256];
    int size,type;
    RawDataBuffer *linkvar,*b;
    if(tok.getnext()!=T_OPREN)
        throw UnexpException(&tok,"( after linked");
    linkedVars.clear();
    for(;;){
        double mn,mx;
        char buf[256];
        // get type
        type = tok.getnext();
        // get name
        tok.getnextident(buf);
        // if diamond, get topic and datum index
        char tname[256];
        int idx;
#if DIAMOND
        if(diamond){
            tok.getnextcheck(T_TOPIC);
            tok.getnextstring(tname);
            tok.getnextcheck(T_COMMA);
            idx = tok.getnextint();
            diamondMap[DiamondTopicKey(tname,idx)]=QString(buf);
            if(!diamondSet.contains(tname))
                diamondSet.insert(tname);
            diamondapparatus::subscribe(tname);
        }
#endif
        // now the range
        tok.getnextcheck(T_RANGE);
        mn = tok.getnextfloat();
        tok.getnextcheck(T_TO);
        mx = tok.getnextfloat();
        
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
    tok.getnextcheck(T_OCURLY);
    for(;;){
        int t = tok.getnext();
        switch(t){
        case T_DIAMOND:
            // diamond variables
#if !DIAMOND
            throw ParseException(&tok,"diamond not supported in this build");
#else
            if(!diamondapparatus::isRunning()){
                diamondapparatus::init();
            }
            switch(t=tok.getnext()){
            case T_NAMEFLOAT:
                parseAVar(true);
                break;
            case T_LINKED:
                parseLinkedVars(true);
                break;
            default:
                throw UnexpException(&tok,"'float' or 'linked'");
                break;
            }
#endif
            break;
        case T_NAMEFLOAT:
            parseAVar(false);
            break;
        case T_BOOL:
            throw ParseException(&tok,"bools not currently supported");
            break;
        case T_LINKED:
            parseLinkedVars(false);
            break;
        case T_CCURLY:
            return;
        default:printf("%d\n",tok.getcurrent());
            throw UnexpException(&tok);
        }
    }
}

static void parseAudio(){
    char warning[1024];
    bool speech;
    
    DataBuffer<float> *buf = ConfigManager::parseFloatSource();
    
    switch(tok.getnext()){
    case T_SAMPLE:speech=false;break;
    case T_SPEECH:speech=true;break;
    default:
        throw UnexpException(&tok,"'speech' or 'sample'");
    }
    
    tok.getnextstring(warning);
    
    getApp()->addAudio(warning,buf,speech);
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
#if MARBLE
            new MapWidget(parent,&tok);
#else
            throw ParseException(&tok,"map not supported in this build");
#endif
            break;
        case T_STATUS:
            new StatusBlockWrapper(parent,&tok);
            break;
        case T_SWITCH:
            new Switch(parent,&tok);
            break;
        case T_MOMENTARY:
            new Momentary(parent,&tok);
            break;
        case T_SLIDER:
            new Slider(parent,&tok);
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
    int spacing=2;
    bool done=false;
    
    char label[256];
    bool hasLabel=false;
    
    while(!done){
        switch(tok.getnext()){
        case T_BORDERLESS:
            borderless=true;
            break;
        case T_LABEL:
            if(!tok.getnextstring(label))
                throw UnexpException(&tok,"frame label");
            hasLabel=true;
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
    
    // if there's a label, we need a containing vbox
    if(hasLabel){
        QFrame *cont = new QFrame;
        QVBoxLayout *bl = new QVBoxLayout;
        cont->setLayout(bl);
        QLabel *lab = new QLabel(label);
        lab->setMaximumSize(10000,20);
        bl->addWidget(lab);
        bl->addWidget(f);
        
        f=cont;
    }
    
    
    // add to the parent's layout
    ((QGridLayout*)parent->layout())->addWidget(f,pos.y,pos.x,pos.h,pos.w);
}

static void parseWindow(){
    // option defaults
    bool fullScreen = false; // should it be fullscreen?
    bool disabled = false;
    // what size? (default is fit around widgets. Ignored for fullscreen.)
    int width=-1,height=-1; 
    // if set, move the window to a screen of the given dimensions
    int swidth=-1,sheight=-1;
    // title if any
    char title[256];
    // "tab" number - used to generate a shortcut to pull this window
    // to the front
    int number=-1;
    
    title[0]=0;
    int screensetline=-1;
    
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
        case T_NUMBER:
            number = tok.getnextint();
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
            screensetline = tok.getline();
            sheight = tok.getnextint();
            break;
        case T_DISABLE: // the window is disabled and should be immediately closed
            disabled=true;
            break;
        }
    }
    
    // create a window
    Window *w = getApp()->createWindow();
    if(number>=0)
        getApp()->setWindowKey(number,w);
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
            throw Exception(screensetline).set("could not find display of %d x %d",swidth,sheight);
        w->move(r.topLeft());
    }
    
    
    // finally show the window and resize if required
    if(disabled){
        w->hide(); // marked "disabled" in the config
    } else {
        w->setWindowState(Qt::WindowActive);
        w->raise();
        w->activateWindow();
        if(fullScreen){
            w->showFullScreen();
        } else {
            if(width>0)
                w->resize(width,height);
            w->showNormal();
        }
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
    if(tok.getnext() == T_SIZE){
        r.minsizex = tok.getnextint();
        if(tok.getnext()==T_COMMA){
            r.minsizey = tok.getnextint();
        }else{
            r.minsizey = r.minsizex;
            tok.rewind();
        }
    }else
          tok.rewind();
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
            throw ParseException(&tok).set("undefined variable '%s'",buf);
        break;
    case T_EXPR:
        tok.getnextstring(buf);
        // OK, we're going to lose a reference to this, but such is life.
        // In this version we never delete expressions anyway.
        try {
            b = (new Expression(buf))->buffer;
        } catch(Exception& e){
            throw Exception(e,tok.getline());
        }
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

static void parseWaypoint(){
    tok.getnextcheck(T_OCURLY);
    
    wpResetDefinitions();
    
    for(;;){
        char buf[256];
        tok.getnextcheck(T_IDENT);
        strcpy(buf,tok.getstring());
        wpAddField(buf,tok.getnextfloat());
        int t = tok.getnext();
        if(t==T_CCURLY)
            break;
        else if(t!=T_COMMA)
            throw UnexpException(&tok,"comma or '}'");
    }
    
    
}

void ConfigManager::parseFile(QString fname){
    
    tok.init();
    if(traceTokeniser)
        tok.settrace(true);
    
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
            // the UDP client now sets its address from the first packet received
            // but this will override it
        case T_SENDADDR:
            tok.getnextstring(udpSendAddr);
            UDPClient::getInstance()->setAddress(udpSendAddr);
            break;
        case T_VALIDTIME:
            DataManager::dataValidInterval = tok.getnextfloat();
            break;
        case T_SENDINTERVAL:
            sendInterval = tok.getnextfloat();
            break;
        case T_UPDATEINTERVAL:
            graphicalUpdateInterval = tok.getnextfloat()*1000;
            break;
        case T_AUDIO:
            parseAudio();
            break;
        case T_WAYPOINT:
            parseWaypoint();
            break;
        default:
            throw UnexpException(&tok,"'var', 'frame', config data or end of file");
        }
    }
    
}

QColor ConfigManager::parseColour(QColor deflt){
    QColor c;
    switch(tok.getnext()){
    case T_IDENT:
    case T_STRING:
        c = QColor(tok.getstring());
        return c;
        break;
    case T_DEFAULT:
        return deflt;
    default:
        throw UnexpException(&tok,"colour name or \"#rgb\"");
    }
}

NudgeType ConfigManager::parseNudgeType(){
    switch(tok.getnext()){
    case T_UP:
        return UP;
    case T_DOWN:
        return DOWN;
    case T_MIN:
        return MIN;
    case T_MAX:
        return MAX;
    case T_CENTRE:
        return CENTRE;
    default:
        throw UnexpException(&tok,"nudge type (up, down, centre)");
    }
}

void ConfigManager::setStyle(QWidget *w){
    if(inverse)
        w->setStyleSheet("background-color: white; color:black;");
    else
        w->setStyleSheet("background-color: black; color:white;");
}    

/// set of 'nudgeable' widgets
static QHash<QString,Nudgeable *> nudgeables;

Nudgeable *ConfigManager::getNudgeable(const char *name){
    QString key(name);
    if(nudgeables.contains(key))
        return nudgeables.value(key);
    else
        throw Exception().set("undefined widget output variable '%s'",name);
}

void ConfigManager::registerNudgeable(const char *name,Nudgeable *n){
    QString key(name);
    if(nudgeables.contains(key))
        throw Exception().set("widget output variable '%s' already defined",name);
    nudgeables.insert(key,n);
}
