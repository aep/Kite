include ../Makefile.in

OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

CXXFLAGS += -g
LDFLAGS += -g

EXES=\
	 http.exe \
	 util.exe \
	 process.exe \
	 unixbus.exe \
	 eventloop.exe \
	 #timer.exe \


ALL: $(EXES) stdio.exe

%.exe:  $(OBJECTS) %.o
	$(CXX) $^ -o $@ $(LDFLAGS)

test: $(EXES) stdio.exe
	$(foreach e,$(EXES),  timeout 10 ./$e &&) true

clean:
	rm -f  ../*.o *.o *.exe

