test: test.o udpclient.o udpserver.o
	g++ -g -o $@ $^ -lrt

udpclient.o:	udpclient.cpp udpclient.h
udpserver.o:	udpserver.cpp udpserver.h
test.o:	test.cpp udpclient.h udpserver.h

.cpp.o:	g++ -g -o $@ $<


clean:
	rm *.o test
	rm *~
