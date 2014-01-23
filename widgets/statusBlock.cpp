#include "statusBlock.h"

#include <QPainter>
#include <QTimer>
#include <QHash>

struct GridEntry {
    QHash<int,QString> altNames;
    QString name;
    ColourCode curColour;
    QColor color;
    bool whiteText;
};

ColourCode StatusColour::BLACK,StatusColour::RED,StatusColour::GREEN,
StatusColour::YELLOW,StatusColour::BLUE,StatusColour::GREY;

QHash<QString,ColourCode> StatusColour::colNameMap;
QHash<ColourCode,StatusColour *> StatusColour::colCodeMap;
int StatusColour::ctr=0;

StatusBlock::StatusBlock(QWidget *parent) : QWidget(parent) {
    grid = NULL;
}

void StatusBlock::setGridSize(int w,int h){
    blockwidth = w;
    blockheight = h;
    
    if(grid)
        delete grid;
    
    grid = new GridEntry[blockwidth*blockheight];
    
    for(int i=0;i<blockwidth*blockheight;i++){
        grid[i].name.fromAscii("");
        set(i,StatusColour::BLACK);
    }
}

int StatusBlock::addItem(int x, int y, const char *s){
    int id = getID(x,y);
    set(id,StatusColour::BLACK);
    grid[id].name = QString::fromAscii(s);
    return id;
}

void StatusBlock::addAltText(int id,ColourCode c,const char *s){
    grid[id].altNames[c]=QString::fromAscii(s);
}

void StatusBlock::paintEvent(QPaintEvent *event){
    float w = width();
    float h = height();
    
    QPainter painter(this);
    
    float boxWidth = w/(float)blockwidth;
    float boxHeight = h/(float)blockheight;
    
    painter.setRenderHint(QPainter::Antialiasing);
    QFontMetrics metrics = painter.fontMetrics();
    
    float textBoxWidth = boxWidth;//*0.75f;
    
    for(int x=0;x<blockwidth;x++){
        for(int y=0;y<blockheight;y++){
            GridEntry *g = grid + getID(x,y);
            
            int x1 = boxWidth*(float)x;
            int y1 = boxHeight*(float)y;
            
            painter.fillRect(x1,y1,boxWidth,boxHeight,
                             g->color);
            
            painter.setPen(g->whiteText ? Qt::white : Qt::black);
            QString name;
            if(g->altNames.contains(g->curColour))
                name = g->altNames[g->curColour];
            else
                name = g->name;
            
            QSize size = metrics.size(0,name);
            
            int tx = x1+boxWidth/2 - size.width()/2;
            int ty = y1+boxHeight/2 + size.height()/2;
            
            painter.drawText(tx,
                             ty,
                             name);
        }
    }
    QWidget::paintEvent(event);
}

void StatusBlock::set(int id, ColourCode col){
    grid[id].curColour=col;
    StatusColour *c = StatusColour::getColour(col);
    grid[id].color = c->color;
    grid[id].whiteText = c->whiteText;
}

