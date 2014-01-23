#ifndef __STATUSBLOCK_H
#define __STATUSBLOCK_H

#include <QWidget>
#include <QHash>
#include "exception.h"

typedef int ColourCode;

/// each colour code has an actual RGB colour
class StatusColour{
    /// map from a name to a colour code
    static QHash<QString,ColourCode> colNameMap;
    /// map of colours from codes
    static QHash<ColourCode,StatusColour *> colCodeMap;
    
    static int ctr;
    
    StatusColour(ColourCode cd, QColor col,bool wt){
        color = col;
        code = cd;
        whiteText = wt;
    }
    

public:    
    int code;
    QColor color;
    bool whiteText;
    
    /// set up default colours
    static void initColours(){
        ctr=0;
        BLACK = addColour("black",Qt::black,true);
        RED = addColour("red",Qt::red,true);
        GREEN = addColour("green",Qt::green,false);
        BLUE = addColour("blue",Qt::blue,true);
        YELLOW = addColour("yellow",Qt::yellow,false);
        
        GREY = addColour("grey",QColor(64,64,64),true);
        
        addColour("darkgreen",Qt::darkGreen,true);
        addColour("darkred",Qt::darkRed,true);
    }
              
    static ColourCode BLACK,GREEN,RED,GREY,YELLOW,BLUE;
    
    /// find a colour by name
    static ColourCode getColourByName(QString s){
        if(colNameMap.contains(s))
            return colNameMap.value(s);
        else
            throw Exception().set("cannot find colour by name"); // damn qstring..
    }
    
    /// find a colour by code
    static StatusColour *getColour(ColourCode code){
        if(colCodeMap.contains(code))
            return colCodeMap.value(code);
        else
            throw Exception().set("cannot find colour code %d",code);
    }
    
    /// add or change colour
    static ColourCode addColour(const char *s,QColor col,bool wt){
        if(colNameMap.contains(s)){
            ColourCode cd = getColourByName(s);
            setColour(cd,col,wt);
            return cd;
        }else{
            StatusColour *c = new StatusColour(ctr,col,wt);
            colNameMap.insert(s,ctr);
            colCodeMap.insert(ctr,c);
            return ctr++;
        }
    }
    
    /// change a colour
    static void setColour(ColourCode code,QColor c,bool wt){
        if(colCodeMap.contains(code)){
            StatusColour *sc = colCodeMap.value(code);
            sc->color = c;
            sc->whiteText = wt;
        }
    }
};

class StatusBlock : public QWidget
{
    Q_OBJECT
public:
    
    explicit StatusBlock(QWidget *parent = 0);
    virtual ~StatusBlock(){}
    
    
        
    
    /// set the rows and columns in the grid
    void setGridSize(int w,int h);
    
    /// add a status element (or cell) to the grid.
    /// returns cell ID, to be passed to set..() methods
    int addItem(int x, int y, const char *s);
    /// add an alternate string to be used for an item when it's a given colour
    void addAltText(int id,ColourCode code,const char *s);
    
protected:
    void paintEvent(QPaintEvent *event);

public:
    int blockwidth,blockheight;

    /// set a cell to a given colour
    void set(int id, ColourCode com);
private:
    struct GridEntry *grid;
    int getID(int x, int y){
        return x+y*blockwidth;
    }
    
};
    


#endif /* __STATUSBLOCK_H */
