/**
 * \file
 * Brief description. Longer description.
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include "widgetmgr.h"
#include <QWidget>
#include <QHash>
#include <QList>
#include <QGridLayout>

/// this structure holds information about a frame - its widget and its set of 
/// contained widgets (as a QHash)
struct FrameData {
    QWidget *frame;
    QGridLayout *layout;
    QList<QWidget *> widgets;
    
    FrameData(QWidget *f, bool vert){
        frame = f;
        layout = new QGridLayout(f);
        layout->setSpacing(5);
    }

    void updateAll(){
        for(int i=0;i<widgets.size();i++)
            widgets[i]->update();
    }
};

static QHash<QString,FrameData *> frames;

void WidgetManager::addFrame(const char *name,QWidget *frame){
    FrameData *f = new FrameData(frame,true);
    frames.insert(name,f);
}

void WidgetManager::addWidget(const char *frameName, QWidget *wid,int x,int y,int w,int h){
    if(FrameData *f = frames.value(frameName)){
        f->widgets.append(wid);
        f->layout->addWidget(wid,y,x,h,w); // row,col,rowspan,colspan
    }
}

void WidgetManager::updateAll(){
    QHashIterator<QString,FrameData *> iter(frames);
    while(iter.hasNext()){
        iter.next();
        FrameData *f = iter.value();
        f->updateAll();
    }
}
