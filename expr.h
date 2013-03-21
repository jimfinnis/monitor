/**
 * @file
 * Expression parser and executor for variable expressions.
 *
 */

#ifndef __EXPR_H
#define __EXPR_H

#include <QList>
#include <QWidget>

#include "datamgr.h"

/// a parsed expression ready to run
class Expression {
public:
    /// compile an expression from a string
    Expression(const char *s);
    
    /// the databuffer holding my calculated values
    DataBuffer<float> *buffer;
    
    /// recalculate me and add a new value to the buffer if required
    void recalc();
    
    /// change the min and max values for the buffer
    void setMinMax(float mn,float mx){
        buffer->minVal = mn;
        buffer->maxVal = mx;
    }
    
private:
    /// instructions, actually a list of opcodes but the implementation
    /// is hidden
    void *instructions;
    /// the original string
    const char *str;
};

    


#endif /* __EXPR_H */
