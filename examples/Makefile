include ../Makefile.in

OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

CXXFLAGS += -g


ALL: timer.exe ssl.exe stdio.exe mqtt.exe udpechoserver.exe http.exe
#mqttsnss.exe

%.exe:  $(OBJECTS) %.o
	$(CXX) $^ -o $@ -lssl -lcrypto

clean:
	rm -f  ../*.o *.o *.exe

