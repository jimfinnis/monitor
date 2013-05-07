/**
 * @file
 * Worker for home-made timers, since QTimer doesn't work consistently
 * on different platforms.
 * 
 */

#ifndef __WORKER_H
#define __WORKER_H

#include <QObject>

#include <stdio.h>
#include <unistd.h>

/// timed worker to send tick() signals. See how createTimer() in app.cpp
/// works.
class Worker : public QObject {
    Q_OBJECT
private:          
    long iterationTime;
    const char *name;
public:
    Worker(const char *n, long t){
        iterationTime = t;
        name=n;
    }
    
    ~Worker(){
    }
    
public slots:
    void process(){
        for(;;){
            usleep(iterationTime*1000);
            printf("Timer %s fires\n",name);
            emit tick();
        }
    }
signals:
    void finished();
    void tick();
};

#endif /* __WORKER_H */
