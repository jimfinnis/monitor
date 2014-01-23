/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#include "number.h"
#include <QPainter>
#include <QPaintEvent>

#include "../config.h"
#include "../exception.h"
#include "../datamgr.h"
#include "../tokens.h"


Number::Number(QWidget *parent,Tokeniser *t) :
QWidget(NULL){
    invalid = true;
    value = 0;
    renderer = NULL;
    
    char varName[64];
    varName[0]=0;
    char title[64];
    title[0]=0;
    
    ConfigRect pos = ConfigManager::parseRect();
    
    bool done = false;

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
            throw Exception(t->getline()).set("Unexpected '%s'",t->getstring());
        }
    }
    
    if(title[0]==0)
        strcpy(title,b->name);
    
    layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    main = new QLabel("");
    label = new QLabel(title);
    label->setAlignment(Qt::AlignCenter);
    label->setMaximumSize(10000,30);

    main->setMinimumSize(pos.minsizex,pos.minsizey);
    layout->addWidget(main);
    layout->addWidget(label);
    setLayout(layout);
    
    main->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
    
    if(!b)
        throw Exception("no data source given",t->getline());
    
    renderer = new DataRenderer(this,b);
    
    QGridLayout *l = (QGridLayout*)parent->layout();
    l->addWidget(this,pos.y,pos.x,pos.h,pos.w);
    ConfigManager::setStyle(this);
}

void Number::dataChanged(){
    if(renderer){
        DataBuffer<float> *b = renderer->getBuffer()->getFloatBuffer();
        // get most recent datum
        Datum<float> *d = b->read(0);
        if(d && d->isRecent()){
            value = d->d; // get data if any recent
            invalid = false;
        }
    }
    if(invalid){
        main->setStyleSheet("color:gray;");
        main->setText("no data");
    }else{
        ConfigManager::setStyle(main);
        main->setText(QString::number(value));
    }
}

