/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __STATUS_H
#define __STATUS_H

/// Wrapper for the status block widget, giving us the extra
/// functionality we need

#include <QList>
#include "statusBlock.h"

#include "../datarenderer.h"

class StatusBlockWrapper;
class Tokeniser;

/// represents a cell inside the block - it's an abstract class

class StatusCell {
protected:
    int id; //!< ID within block
    StatusBlockWrapper *widget;
    DataRenderer *renderer; //!< our data renderer
public:
    
    /// constructor, taking the status block of which we are a part -
    /// will add the cell to that block
    StatusCell(StatusBlockWrapper *w){
        widget = w;
        renderer=NULL;
    };
    
    virtual ~StatusCell(){
        if(renderer)delete renderer;
    }
    
    /// set this status cell's colour
    virtual void set()=0;
};

/// a status cell which shows different colours for different
/// ranges of a float value. There is a series of (float,color)
/// pairs which are iterated through in order - the colour shown is the first
/// colour for which (input<float) is true.

class StatusFloatRangeCell : public StatusCell {
    float rangeVal[16]; //!< values we test against
    StatusBlock::Colour cols[16]; //!< colours corresponding with the values
    StatusBlock::Colour elseCol; //!< colour to show if no conditions are true
    int ct; //!< how many entries
    
    void parseBands(Tokeniser *t);
    
public:
    virtual void set();
    StatusFloatRangeCell(Tokeniser *t,StatusBlockWrapper *w);
};

/// a status cell which shows two colours for negative and non-negative
/// ranges of a float value.
class StatusBooleanCell : public StatusCell {
    StatusBlock::Colour trueCol; //!< for non-negative values
    StatusBlock::Colour falseCol; //!< for negative values
    
public:
    virtual void set();
    StatusBooleanCell(Tokeniser *t,StatusBlockWrapper *w);
};

    
/// our widget, which is a wrapper around a StatusBlock widget.
class StatusBlockWrapper : public StatusBlock {
    Q_OBJECT

public:          
    
    explicit StatusBlockWrapper(QWidget *parent,Tokeniser *tok);
    virtual ~StatusBlockWrapper(){}
    
private:          
    QList<StatusCell *>cells;
    
};

#endif /* __STATUS_H */
