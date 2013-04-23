/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#include <QPainter>
#include <QPaintEvent>

#include "compass.h"
#include "../config.h"
#include "../exception.h"
#include "../datamgr.h"
#include "../tokens.h"

#include "compassPanel.h"



Compass::Compass(QWidget *parent,Tokeniser *t) :
QWidget(NULL){
    invalid = true;
    value = 0;
    renderer = NULL;
    
    char varName[64];
    varName[0]=0;
    char title[64];
    title[0]=0;
    
    ConfigRect pos;
    pos.x = -1;
    
    bool done = false;
    inverse = ConfigManager::inverse;
    
    pos = ConfigManager::parseRect();
    t->getnextcheck(T_OCURLY);
    DataBuffer<float> *b=NULL;
    
    while(!done){
        switch(t->getnext()){
        case T_VAR:
        case T_EXPR:
            t->rewind();
            b = ConfigManager::parseFloatSource();
            break;
        case T_TITLE:
            t->getnextstring(title);
            break;
        case T_CCURLY:
            done=true;
            break;
        default:
            throw Exception().set("Unexpected '%s'",t->getstring());
        }
    }
    if(title[0]==0)
        strcpy(title,b->name);
    
    layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    main = new CompassPanel(this);
    label = new QLabel(title);
    layout->setAlignment(Qt::AlignCenter);
    label->setAlignment(Qt::AlignCenter);
    label->setMaximumSize(10000,30);
    
    float minsize = pos.minsizex<pos.minsizey ? pos.minsizex : pos.minsizey;
    main->setMinimumSize(minsize,minsize);
    main->setMaximumSize(minsize,minsize);
    layout->addWidget(main);
    layout->addWidget(label);
    setLayout(layout);
    
    if(!b)
        throw Exception("no data source given");
    
    renderer = new DataRenderer(main,b);
    
    QGridLayout *l = (QGridLayout*)parent->layout();
    l->addWidget(this,pos.y,pos.x,pos.h,pos.w);
}

void Compass::paintEvent(QPaintEvent *event){
    if(renderer){
        DataBuffer<float> *b = renderer->getBuffer()->getFloatBuffer();
        // get most recent datum
        Datum<float> *d = b->read(0);
        if(d && d->isRecent()){
            value = d->d; // get data if any recent
            invalid = false;
        }else invalid=true;
    }
}
