/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __EXCEPTION_H
#define __EXCEPTION_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <exception>

class Exception : public std::exception {
public:
    Exception(const char *e){
        if(e)
            strncpy(error,e,1024);
        else
            strcpy(error,"???");
    }
    
    /// a variadic fluent modifier to set a better string with sprintf
    Exception& set(const char *s,...){
        va_list args;
        va_start(args,s);
        
        vsnprintf(error,1024,s,args);
        va_end(args);
        return *this;
    }
    
    /// default ctor
    Exception(){
        strcpy(error,"???");
    }
    
    /// construct the exception. Note that
    /// space is allocated here, which is deleted
    /// by the destructor. This version considers e to be
    /// a format string, and arg a string argument.
    Exception(const char *e,const char *arg){
        snprintf(error,1024,e,arg);
    }
    
    /// return the error string
    virtual const char *what () const throw (){
        return error;
    }
    
    /// a copy of the error string
    char error[1024];
};

/// this exception is thrown when a generic runtime error occurs

class RuntimeException : public Exception {
public:
    RuntimeException(const char *e,const char *fn,int l){
        char tmp[512];
        if(e){
            strcpy(tmp,e);e=tmp;  // to stop bloody exp-ptrcheck complaining.
        }
    
        if(fn)
            strncpy(fileName,fn,1024);
        else
            strcpy(fileName,"???");
        line = l;
        
        snprintf(error,512,"%s(%d):  %s",fileName,line,
                 e?e:"unknown");
    }
   
    char fileName[1024];
    /// the current Lana line or -1
    int line;
};

/// macro producing a RuntimeException with the file's name and line number
#define RUNT(x) RuntimeException(x,__FILE__,__LINE__)

#endif /* __EXCEPTION_H */
