/**
 * @file
 * Status block wrapper implementation - wrapping a status block
 * in databuffer/expression/parser stuff.
 * 
 */


#include "../config.h"
#include "../exception.h"
#include "../tokens.h"
#include <QGridLayout>

#include "status.h"

class StatusCellRenderer: public DataRenderer {
    StatusCell *cell;
public:
    StatusCellRenderer(QWidget *w,RawDataBuffer *b,StatusCell *c):
    DataRenderer(w,b){
        cell = c;
    }
    virtual void onNewData(RawDataBuffer *b){
        cell->set(); // tell the cell to recalculate its colour
        // and then redraw the widget
        DataRenderer::onNewData(b);
    }
};


StatusBlockWrapper::StatusBlockWrapper(QWidget *parent,Tokeniser *t):
StatusBlock(NULL){
    bool done=false;
    int w=-1,h=-1;
    
    ConfigRect pos = ConfigManager::parseRect();
    t->getnextcheck(T_OCURLY);
    
    if(t->getnext()!=T_SIZE)
        throw UnexpException(t,"size of grid");
    w = t->getnextint();
    t->getnextcheck(T_COMMA);
    h = t->getnextint();
    setGridSize(w,h);
    
    while(!done){
        switch(t->getnext()){
        case T_CCURLY:
            done = true;
            break;
        case T_COLOUR:
        case T_COL:
        case T_COLOR:
            {
                char colname[128];
                t->getnextident(colname);
                QColor col = ConfigManager::parseColour(Qt::green);
                int qq = t->getnext();
                if(qq!=T_TRUE && qq!=T_FALSE)
                    throw UnexpException(t,"true or false for white text in colour");
                
                StatusColour::addColour(colname,col,qq==T_TRUE);
            }
            
            break;
        case T_FLOATRANGE:
            cells.append(new StatusFloatRangeCell(t,this));
            break;
        case T_BOOL:
            cells.append(new StatusBooleanCell(t,this));
            break;
        default:
            throw UnexpException(t);
        }
    }
    
    setMinimumSize(pos.minsizex,pos.minsizey);
    QGridLayout *l = (QGridLayout*)parent->layout();
    l->addWidget(this,pos.y,pos.x,pos.h,pos.w);
}

void StatusFloatRangeCell::set(){
    // get the latest datum out of the buffer
    DataBuffer<float> *b = renderer->getBuffer()->getFloatBuffer();
    Datum<float> *d = b->read(0);
    if(d && d->isRecent()){
        ColourCode col=elseCol;
        float v = d->d;
        for(int i=0;i<ct;i++){
            if(v<rangeVal[i]){
                col=cols[i];break;
            }
        }
        widget->set(id,col);
    } else
        widget->set(id,StatusColour::GREY);
}

static ColourCode parseCol(Tokeniser *t){
    if(t->getnext()!=T_IDENT)
        throw UnexpException(t,"colour name");
    
    return StatusColour::getColourByName(t->getstring());
}

void StatusFloatRangeCell::parseBands(Tokeniser *t){
    static StatusFloatRangeCell *prev = NULL;
    for(;;){
        switch(t->getnext()){
        case T_LT:
            if(ct==16)
                throw ParseException(t,"too many bands in floatrange");
            rangeVal[ct]=t->getnextfloat();
            cols[ct++]=parseCol(t);
            break;
        case T_ELSE:
            elseCol = parseCol(t);
            prev=this;
            return;
        case T_PREVIOUS: // must be last
            {
                if(!prev)
                    throw ParseException(t,"no previous bands to copy!");
                for(int i=0;i<prev->ct;i++){
                    cols[i] = prev->cols[i];
                    rangeVal[i]=prev->rangeVal[i];
                }
                elseCol = prev->elseCol;
                ct = prev->ct;
            }
            prev=this;
            return; // must be last
        default:
            throw UnexpException(t,"<, else or previous");
        }
    }
}

StatusFloatRangeCell::StatusFloatRangeCell(Tokeniser *t,StatusBlockWrapper *w):
StatusCell(w){
    bool done=false;
    t->getnextcheck(T_OCURLY);
    int x=-1,y=-1;
    DataBuffer<float> *buf;
    ct=0;
    char title[256];
    char str[256];
    ColourCode whenCol;
    
    // these have to be in some kind of order
    
    if(t->getnext() != T_POS)
        throw UnexpException(t,"x,y position in grid");
    x = t->getnextint();
    t->getnextcheck(T_COMMA);
    y = t->getnextint();
    
    if(x>=widget->blockwidth)
        throw ParseException(t,"pos x out of range");
    if(y>=widget->blockheight)
        throw ParseException(t,"pos x out of range");
    
    if(t->getnext() != T_TITLE)
        throw UnexpException(t,"title string");
    t->getnextstring(title);
    
    
    switch(t->getnext()){
    case T_EXPR:
    case T_VAR:
        t->rewind();
        buf = ConfigManager::parseFloatSource();
        break;
    default:
        throw UnexpException(t,"expression or variable");
    }
    
    id = widget->addItem(x,y,title);
    
    // now we've created the item, we can be a little more
    // lax on ordering
    
    while(!done){
        switch(t->getnext()){
        case T_BANDS:
            parseBands(t);
            break;
        case T_WHEN:
            whenCol = parseCol(t);
            t->getnextstring(str);
            widget->addAltText(id,whenCol,str);
            break;
        case T_CCURLY:
            done = true;
            break;
        }
    }
    
    renderer = new StatusCellRenderer(widget,buf,this);
}


StatusBooleanCell::StatusBooleanCell(Tokeniser *t,StatusBlockWrapper *w):
StatusCell(w){
    bool done=false;
    t->getnextcheck(T_OCURLY);
    int x=-1,y=-1;
    DataBuffer<float> *buf;
    char title[256];
    
    trueCol = StatusColour::GREEN;
    falseCol = StatusColour::BLACK;
    
    // these have to be in some kind of order
    
    if(t->getnext() != T_POS)
        throw UnexpException(t,"x,y position in grid");
    x = t->getnextint();
    t->getnextcheck(T_COMMA);
    y = t->getnextint();
    
    if(t->getnext() != T_TITLE)
        throw UnexpException(t,"title string");
    t->getnextstring(title);
    
    
    switch(t->getnext()){
    case T_EXPR:
    case T_VAR:
        t->rewind();
        buf = ConfigManager::parseFloatSource();
        break;
    default:
        throw UnexpException(t,"expression or variable");
    }
    
    id = widget->addItem(x,y,title);
    
    // now we've created the item, we can be a little more
    // lax on ordering
    
    while(!done){
        switch(t->getnext()){
        case T_TRUE:
            trueCol = parseCol(t);
            break;
        case T_FALSE:
            falseCol = parseCol(t);
            break;
        case T_CCURLY:
            done = true;
            break;
        }
    }
    renderer = new StatusCellRenderer(widget,buf,this);
}
    

void StatusBooleanCell::set(){
    // get the latest datum out of the buffer
    DataBuffer<float> *b = renderer->getBuffer()->getFloatBuffer();
    Datum<float> *d = b->read(0);
    if(d && d->isRecent()){
        widget->set(id,d->d<0 ? falseCol:trueCol);
    } else
        widget->set(id,StatusColour::GREY);
}
