include ../Makefile.in

OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

CXXFLAGS += -g


ALL: timer.exe ssl.exe stdio.exe mqtt.exe udpechoserver.exe http.exe process.exe shell.exe wget.exe
#mqttsnss.exe

%.exe:  $(OBJECTS) %.o
	$(CXX) $^ -o $@ $(LDFLAGS)

clean:
	rm -f  ../*.o *.o *.exe

