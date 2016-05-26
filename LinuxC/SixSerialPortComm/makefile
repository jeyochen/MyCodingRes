CP=/bin/cp -f
CPP=g++
RLSDIR=.

PROGRAM=SerialPortTest
CXX_CFLAGS= -g -m64 -gstabs -Wno-deprecated
CXX_INC=-I./
CXX_LDFLAGS=-L/usr/lib64
LIBS= -lrt -ldl

DIRS=.
SRCS=$(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
OBJS=$(patsubst %.cpp, %.o, $(SRCS))

PROGDIR=.
all:$(PROGRAM)
$(PROGRAM):$(OBJS)
	@echo Build $(PROGDIR)/$@
	$(CPP) $(CXX_CFLAGS) $(CXX_INC) $(CXX_LDFLAGS) -o $(RLSDIR)/$(PROGRAM) $(OBJS)  $(LIBS)

%.o:%.cpp
	$(CPP) $(CXX_CFLAGS) $(CXX_INC) -c $< -o $@

.PHONY:clean install
clean:
	$(RM) $(PROGDIR)/$(PROGRAM)
	$(RM) $(OBJS)
install:
	$(RM) $(RLSDIR)/$(PROGRAM)
	$(cp) $(PROGRAM) $(RLSDIR) 
