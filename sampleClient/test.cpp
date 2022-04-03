/**
 * \file
 * Brief description. Longer description.
 *
 * 
 * \author $Author$
 * \date $Date$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "udpclient.h"
#include "udpserver.h"

int outport=33333; // the port we're sending telemetry on
int inport=33334; // the port we're listening on for control

/// this is a telemetry variable - it's called "a" and it's 
/// a kind of temperature
float a = 2;

/// this is another telemetry variable, who's value is controlled
/// by the slider setting- but not directly. This value gets pulled
/// towards whatever the slider was last set to. Since this is being
/// used as the slider feedback variable, the slider will show badack
/// until sliderout is very close to the setting.
float sliderout = 5;

/// these are control variables, set by the monitor. Note that these
/// are pointers to floats, created by createVar().

float *heat; //!< a toggle value 0 or 1 - is the heater on?
float *cool; //!< a momentary value 0 or 1 - should we do a cooling burst?
float *sliderset; //!< a slider value
float *bang; //!< a momentary value to reset the sliderout - no feedback.
float *sliderset2; //!< another slider value

/// a multi-state control - either 0.0, 1.0 or 2.0.
float *multi;

UDPClient *client;

/// run a tick of the simulator
void runsim(){
    
    /// output the telemetry packet - this will prepend a timestamp
    client->write("a=%f heat=%f sliderout=%f sliderset2=%f multi=%f batt_pwr=3",a,*heat,sliderout,*sliderset2,*multi);
    
    a += 0.15; //temperature coming in
    a += *heat*0.2; // extra heat if heater is on (note pointer)
    a *= 0.99; // temperature going out
    
    // gradually pull sliderout towards slider
    sliderout = 0.6f*sliderout + 0.4f* (*sliderset);
    
    /// if *cool is high, the user clicked the momentary cooling button.
    /// we perform an action, then acknowledge the action by
    /// sending "cool=1" straight back --- this is the telemetry variable
    /// being monitored for feedback by that button. Finally we
    /// turn the control variable off -- otherwise it's going to stay high.
    
    if(*cool){
        printf("COOL SENT\n");
        a *= 0.5f; // do cooling burst
        client->write("cool=1"); // acknowledge
        *cool=0; // cooling off.
    }
    
    if(*bang){
        sliderout = 0;
        *bang=0;
    }
}



int main(int argc,char *argv[]){
    
    // initialise the server, to which the monitor sends control packets
    UDPServer s(inport);
    // and the client, which sends messages to the server
    client = new UDPClient("127.0.0.1",outport,false);
    
    // create a few variables for the server to control
    
    heat=s.createVar("heat"); // a toggle boolean
    cool=s.createVar("cool"); // a momentary boolean
    bang=s.createVar("bang"); // momentary to reset slider
    multi=s.createVar("multi"); // multistate
    
    // a test value for the slider, which we'll try to pull 'sliderout' towards
    sliderset=s.createVar("sliderset");
    /// another slider
    sliderset2=s.createVar("sliderset2");
    
    printf("outport is %d, inport is %d\n",outport,inport);
    
    for(;;){
        usleep(900000); // wait a bit
        s.update(); // poll the server, reading pending packets
        runsim(); // and run the simulator for a bit
    }
        
}
