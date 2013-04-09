/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __NUMBER_H
#define __NUMBER_H


#include "datarenderer.h"
#include "tokeniser.h"
#include <QLabel>
#include <QVBoxLayout>

class Number : public QWidget {
    Q_OBJECT
          
public:
    virtual ~Number(){
        delete renderer;
    }
    
    
    /// parse the configuration details 
    Number(QWidget *parent,Tokeniser *t);
    
    virtual void paintEvent(QPaintEvent *e);

private:
    QVBoxLayout *layout; //!< the layout
    QLabel *main; //!< the main widget
    QLabel *label; //!< the label

    float value; //!< current value
    
    /// our link to the variable          
    DataRenderer *renderer;

    /// if this is false, the value has not been set - it's invalid.
    bool invalid;
};
    
    
#endif /* __NUMBER_H */
