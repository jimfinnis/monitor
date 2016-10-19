/**
 * @file diamond.h
 * @brief  Brief description of file.
 *
 */

#ifndef __DIAMOND_H
#define __DIAMOND_H

#include <QMap>
#include <QSet>


#if DIAMOND
#include <diamondapparatus/diamondapparatus.h>

struct DiamondTopicKey {
    bool operator < (const DiamondTopicKey& a) const {
        if(topic == a.topic)
            return idx<a.idx;
        else
            return topic < a.topic;
    }
    
    DiamondTopicKey(QString t,int i){
        topic = t;
        idx = i;
    }
    
    
    QString topic;
    int idx;
};

// used by diamond apparatus messages - maps topic+index onto var name
extern QMap<DiamondTopicKey,QString> diamondMap;
extern QSet<QString> diamondSet; // set of topics subscribed

#endif


#endif /* __DIAMOND_H */
