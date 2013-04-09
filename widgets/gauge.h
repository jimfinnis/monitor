#ifndef GAUGE_H
#define GAUGE_H

#include "datarenderer.h"
#include "tokeniser.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

/// the internal widget which actually does the gauge.
class GaugeInternal : public QWidget {
    Q_OBJECT

public:
    explicit GaugeInternal(class Gauge *g) : QWidget((QWidget *)g){
        gauge = g;
    }

protected:
    virtual void paintEvent(QPaintEvent *event);
private:
    class Gauge *gauge;

};




/// a simple gauge widget, shadowing data in a DataBuffer - actually consists of a label and a GaugeInternal.
class Gauge : public QWidget
{
    friend class GaugeInternal;

    Q_OBJECT
          
public:
    virtual ~Gauge(){
        if(renderer)
            delete renderer;
    }
    bool inverse;

    /// the painter, actually called by an internal widget (which needs to be passed in,
    /// as it's that which is redrawn)
    virtual void handlePaint(QPaintEvent *p,GaugeInternal *w);
    
    /// parse the configuration details for a gauge and construct it
    Gauge(QWidget *parent,Tokeniser *t);

private:
    QVBoxLayout *layout; //!< the layout
    QWidget *main; //!< the main widget
    QLabel *label; //!< the label containing varname or title
    QLabel *label2; //!< the subtitle

    float value; //!< current value
    
    float redLevel; //!< red start value - 0 to 1
    float yellowLevel; //!< yellow start value - 0 to 1
    
    QColor redCol,greenCol,yellowCol;
    int darkFactor;
    
    /// our link to the variable          
    DataRenderer *renderer;
};


#endif // GAUGE_H
