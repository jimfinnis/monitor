/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __AUDIO_H
#define __AUDIO_H

#include <string.h>
#include "datamgr.h"

class AudioWarning {
public:
    AudioWarning(const char *w,DataBuffer<float> *b,bool speech);
    void check();
private:
    DataBuffer<float> *buf;
    bool speech; //!< is it speech or sample?
    char *str; //!< if speech is true
    
};


#endif /* __AUDIO_H */
