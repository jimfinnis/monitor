/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#ifndef __COMPASS_H
#define __COMPASS_H


#include "datarenderer.h"
#include "tokeniser.h"
#include <QLabel>
#include <QVBoxLayout>

class Compass : public QWidget {
    Q_OBJECT
          
public:
    virtual ~Compass(){
        delete renderer;
    }
    
    
    /// parse the configuration details 
    Compass(QWidget *parent,Tokeniser *t);
    
    virtual void paintEvent(QPaintEvent *e);

    /// if this is false, the value has not been set - it's invalid.
    bool invalid;
    float value; //!< current value
private:
    QVBoxLayout *layout; //!< the layout
    QWidget *main; //!< the main widget
    QLabel *label; //!< the label

    
    /// our link to the variable          
    DataRenderer *renderer;

};
    
    
#endif /* __COMPASS_H */
