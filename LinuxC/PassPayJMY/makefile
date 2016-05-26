CP=/bin/cp -f
CPP=g++
RLSDIR=.

PROGRAM=PassPayTest
#-D_NORMAL_ENV 正式环境用的是串口，测试环境用的是USB转串口
#-D_OUTFILE 输出日志到文件 -D_OUTSCR 输出日志到屏幕 -D_BOTHLOG同时输出到文件和屏幕
CXX_CFLAGS= -g -m64 -gstabs -Wno-deprecated -D_OUTFILE
CXX_INC=-I./
CXX_LDFLAGS=-L/usr/lib64
LIBS= -lpthread -lrt -ldl

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
