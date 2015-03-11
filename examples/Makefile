include ../Makefile.in



OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
$(info $(OBJECTS))


CXXFLAGS += -g


ALL: timer ssl stdio mqtt

timer:  $(OBJECTS) timer.o
	g++ $^ -o $@ -lssl -lcrypto

ssl:  $(OBJECTS) ssl.o
	g++ $^ -o $@ -lssl -lcrypto

stdio:  $(OBJECTS) stdio.o
	g++ $^ -o $@ -lssl -lcrypto

mqtt: $(OBJECTS) mqtt.o
	g++ $^ -o $@ -lssl -lcrypto

clean:
	rm -f  ../*.o *.o example

