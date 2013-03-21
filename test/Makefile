test: test.o udpclient.o 
	g++ -o $@ $^

udpclient.o:	udpclient.cpp udpclient.h
test.o:	test.cpp udpclient.h

.cpp.o:	g++ -o $@ $<


clean:
	rm *.o test
	rm *~
