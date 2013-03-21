#ifndef __STATUSBLOCK_H
#define __STATUSBLOCK_H

#include <QWidget>

class StatusBlock : public QWidget
{
    Q_OBJECT
public:
    
    explicit StatusBlock(QWidget *parent = 0);
    virtual ~StatusBlock(){}
    
    /// the colours the status block cells can be

    enum Colour {
        RED=0,YELLOW,BLUE,GREEN,BLACK
    };

    /// set the rows and columns in the grid
    void setGridSize(int w,int h);
    
    /// add a status element (or cell) to the grid.
    /// returns cell ID, to be passed to set..() methods
    int addItem(int x, int y, const char *s);
    /// add an alternate string to be used for an item when it's a given colour
    void addAltText(int id,Colour c,const char *s);
    
protected:
    void paintEvent(QPaintEvent *event);

public:

    /// set a cell to a given colour
    void set(int id, Colour col);

    /// set a cell to an entirely arbitrary colour
    void setCol(int id, float r,float g,float b, bool whiteText);
private:
    int blockwidth,blockheight;
    struct GridEntry *grid;
    int getID(int x, int y){
        return x+y*blockwidth;
    }
    
};
    


#endif /* __STATUSBLOCK_H */
