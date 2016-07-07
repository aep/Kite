include ../Makefile.in

OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

CXXFLAGS += -g


ALL: tftpd.exe timer.exe ssl.exe stdio.exe mqtt.exe udpechoserver.exe http.exe process.exe shell.exe wget.exe miniterm.exe
#mqttsnss.exe

%.exe:  $(OBJECTS) %.o
	$(CXX) $^ -o $@ $(LDFLAGS)

clean:
	rm -f  ../*.o *.o *.exe

