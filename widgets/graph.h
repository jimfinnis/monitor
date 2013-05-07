/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */

#ifndef __GRAPH_H
#define __GRAPH_H

#include <QWidget>
#include <QString>
#include <QList>
#include <QColor>
#include <QPainter>

#include "datarenderer.h"
#include "tokeniser.h"

class Graph;

/// an interface for anything which renders on a graph

class GraphDataRenderer {
public:
    virtual void render(int idx,Graph *g,QPainter &painter)=0;
    virtual ~GraphDataRenderer(){}
};

/// a renderer for float data in a graph, adding style data -
/// actual rendering is done by the widget, though, it's simpler
/// that way.

class GraphFloatRenderer : public DataRenderer, public GraphDataRenderer {
public:
    QColor color; //!< pen colour
    qreal width; //!< pen width
          
    double minVal; //!< start of range
    double maxVal; //!< end of range
    
    /// parse the configuration details for a variable and return
    /// a new graph float renderer. This assumes that "var x {" has
    /// just been parsed.
    GraphFloatRenderer(Graph *g,DataBuffer<float> *b,Tokeniser *t);
    virtual ~GraphFloatRenderer(){}
    /// render the item and its key
    virtual void render(int idx,Graph *g,QPainter &painter);
};


/// This widget is a graph, which can show historical data for
/// several databuffers. Each is added to system as a 
/// GraphFloatRenderer.

class Graph : public QWidget {
    Q_OBJECT
          
public:
    explicit Graph(QWidget *parent=0);
    virtual ~Graph(){}

    virtual void paintEvent(QPaintEvent *event);
    
    
    /// parse the configuration details for a gauge 
    Graph(QWidget *parent,Tokeniser *t);
    
    
    double tNow; //!< current time, used in rendering
    double pixPerSec; //!< pixels per second on x axis, used in rendering, calculated from widthInSeconds
    double widthInSeconds; //!< set by user
    
    bool inverse;
    /// how many items
    int getItemCount(){
        return renderers.count();
    }
public slots:
    void dataChanged(){
        update();
    }
    
private:
    /// renderers by buffer name (just floats for now)
    QList<GraphDataRenderer *> renderers;
};


#endif /* __GRAPH_H */
