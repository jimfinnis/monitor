/**
 * \file
 * Handles the base DataRenderer class, which are the parts of the custom
 * widgets which receive data from DataBuffers and either render it or
 * tell the widget to do so.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __DATARENDERER_H
#define __DATARENDERER_H

#include "QWidget"
#include "datamgr.h"

/// a DataRenderer is the interface between a widget and one or more data variables
/// (in their form DataBuffers). Each DataRenderer is part of one of
/// the custom widgets. On receiving new data from the buffer, it typically
/// informs the widget that it should render itself. The widget will
/// then do so, possibly telling the DataRenderer to do the rendering
/// or in simple cases doing it itself.
/// Renderers are able to listen to multiple variables.

class DataRenderer : public QObject,DataBufferListener {
    Q_OBJECT
protected:
    /// the variable to which I am linked - note that I can't actually get any
    /// data out of this interface without knowing what the type is and casting
    /// to the appropriate template. Fraught with peril.
    RawDataBuffer *buffer;
    QWidget *widget; //!< the widget of which I am a part

signals:
    /// we emit this signal when our buffer changes
    void changed();
    
public:
    /// create the renderer, linking to both the parent widget and the buffer
    /// I'm supposed to watch
    DataRenderer(QWidget *w, RawDataBuffer *buf) {
        widget = w;
        connect(this,SIGNAL(changed()),w,SLOT(dataChanged()));
        buffer = buf;
        buffer->addListener(this);
    }
    
    virtual ~DataRenderer(){
        buffer->removeListener(this);
    }
    
    /// get the buffer - the variable - I am watching.
    RawDataBuffer *getBuffer(){
        return buffer;
    }
    
    /// this will be called whenever the data buffer has a new
    /// item
    virtual void onNewData(UNUSED RawDataBuffer *b){
        emit changed();
    }
};






#endif /* __DATARENDERER_H */
