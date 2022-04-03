/**
 * \file
 * Brief description. Longer description.
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __EXCEPTIONN_H
#define __EXCEPTIONN_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <exception>

class Exception : public std::exception {
private:
    void consError(const char *e=NULL){
        e = e?e:"????";
        strncpy(error,e,1024);
        rebuild();
    }
protected:
    void rebuild(){
        if(line>=0)
            snprintf(out,1024,"%s (line %d)",error,line);
        else
            strcpy(out,error);
    }        
        
public:
    
    
    Exception(const char *e){
        line = -1;
        consError(e);
    }
    
    Exception(const char *e,int l) {
        line = l;
        consError(e);
    }
    
    /// a variadic fluent modifier to set a better string with sprintf
    Exception& set(const char *s,...){
        va_list args;
        va_start(args,s);
        
        vsnprintf(error,1024,s,args);
        va_end(args);
        rebuild(); // have to do this again
        return *this;
    }
    
    /// default ctor
    Exception(){
        line = -1;
        consError();
    }
    
    Exception(int l) {
        line = l;
        consError();
    }
    
    /// allows us to rethrow an exception, adding a line
    Exception(Exception& e,int l){
        strcpy(error,e.error);
        line = l;
        rebuild();
    }
    
    /// return the error string
    virtual const char *what () const throw (){
        return out;
    }
    
    /// a copy of the error string
    char error[1024];
    char out[1024];
    int line; //!< line number if occurred in parse
    
};

#endif /* __EXCEPTION_H */
