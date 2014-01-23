/**
 * \file
 * Brief description. Longer description.
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include <stdlib.h>
#include <unistd.h>
#include "audio.h"

static time_t lastt=0;


AudioWarning::AudioWarning(const char *w,DataBuffer<float> *b,bool sp){
    speech = sp;
    buf = b;
    
    str = strdup(w);
    
}

static void say(const char *s){
    time_t msgt;
    time(&msgt);
    if(msgt - lastt > 2){
        lastt=msgt;
        if(!fork()){
            execl("/usr/bin/espeak","espeak",s,NULL);
            exit(0);
        }
    }
}

static void play(const char *s){
    time_t msgt;
    time(&msgt);
    if(msgt - lastt > 2){
        lastt=msgt;
        if(!fork()){
            execl("/usr/bin/aplay","aplay",s,NULL);
            exit(0);
        }
    }
}

void AudioWarning::check(){
    Datum<float> *d = buf->read(0);
    if(d && d->d>0.5f){
        if(speech)
            say(str);
        else
            play(str);
    }
}
