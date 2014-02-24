A highly configurable Qt program for monitoring data being sent by key/value UDP packets; can also send data.

#Building

##Dependencies
* You'll need Qt4 and qmake and all that jazz.
* To build the mapping widgets, you'll also need MarbleWidget (on Ubuntu, that's the *libmarble-dev* package).

## Building
* Run **qmake-qt4 monitor.pro** to build the makefile if you need to.
* Then run **make** to build the code.

## Running
* Once it's built, run

        ./monitor [--port N] [--file configfilename]

* The defaults are *--port 13231 --file config*.
* The current *config* is for the rover.
* *exampleconfig* contains lots of comments, and runs in association with a test file found in the directory "test" which produces packets of the form
    
        time=1363456055 lat=52.841473 long=-3.459698 a=0.099833 b=-0.666276

*You don't need to send every variable in every packet, but there must always be "time" which must be a UNIX timestamp.
        

## More documentation
* The syntax of config files is fully documented in *doc/main.pdf*
* The waypoint transmission protocol is documented in *waypoint/WAYPOINTPROTO*

