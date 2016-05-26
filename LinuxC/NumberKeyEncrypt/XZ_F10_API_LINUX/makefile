CP=/bin/cp -f
INSDIR=/usr/local/lib

CPP=g++
CXX_CFLAGS=-g -w -m64 -O2 -Wsign-compare -D_DEBUG -D_TTY_USB
SHARED = -shared
FPIC = -fPIC
LIBNAME=libXZ_F10_API_LINUX.so

CXX_INC = -I.
CXX_LDFLAGS = /usr/lib/

SRCS:=$(wildcard *.cpp)
OBJS:=$(SRCS:.cpp=.o)

LIBDIR = ./

all:$(LIBNAME)
$(LIBNAME):$(OBJS)
	@echo Build...$@
	@rm -f $(LIBDIR)/$(LIBNAME)
	$(CPP)  $(SHARED) $(FPIC) -o $(LIBDIR)$(LIBNAME) $(OBJS)

%.o: %.cpp
	@echo Compile...$@
	$(CPP) $(CXX_CFLAGS) $(FPIC) -c $< -o $@

.PHONY:clean install
clean:
	$(RM) $(OBJS)
	$(RM) $(INSDIR)$(LIBNAME)
	$(RM) $(LIBDIR)$(LIBNAME)
	@echo Clean net...done!

install:
	$(CP) $(LIBDIR)$(LIBNAME) $(INSDIR)
