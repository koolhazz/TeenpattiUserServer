
#memory check flag
#MEMCHECK=1

#gcc version
CC  = gcc
CXX = g++
GCCVER := $(shell $(CC) -dumpversion | awk -F. '{ print $$1"."$$2}' )

CORE_INC=../include
CORE_SRC=../src/core
COMPILE_LIB_HOME=../lib

INC     = -I. -I $(CORE_INC) 
OPT     = -pipe -fno-ident -fPIC -shared -z defs -e version
LINK    = -lpthread

#ARCH = $(shell $(CC) $(DEBUG) -E -DARCHTEST $(BUILD_DIR)/conftest.c|grep 86)
#ifeq ($(ARCH),i386)
#CFLAGS += $(shell $(BUILD_DIR)/conftest.sh "$(CC)" -march=pentium3 "$(BUILD_DIR)")
#else
#ifeq ($(ARCH),x86_64)
#else
#echo $(error Unsupported Compiler, CC=$(CC) ARCH=$(ARCH)))
#endif
#endif

CFLAGS += $(OPT) -g -pg -Wall -D_GNU_SOURCE -funroll-loops -MMD -D_REENTRANT
#CFLAGS += $(shell $(BUILD_DIR)/conftest.sh "$(CC)" -ftracer "$(BUILD_DIR)")
#CFLAGS += $(shell $(BUILD_DIR)/conftest.sh "$(CC)" -ftree-vectorize "$(BUILD_DIR)")
ifeq ($(MEMCHECK),1)
CFLAGS += -DMEMCHECK
endif
CXXFLAGS := $(CFLAGS)

SRCS = 	core/MarkupSTL.cpp \
		core/cache.cpp\
		core/mempool.cpp\
        core/net.cpp \
        core/log.cpp \
        core/config.cpp \
        core/memcheck.cpp \
        core/poller.cpp \
        core/timerlist.cpp \
		core/tm_task.cpp \
		CCReactor.cpp \
		CCSocketServer.cpp\
		CCSocketHandler.cpp \

#DYNAMIC_NAME= libcore-gcc$(GCCVER)-ver$(COMM_LIB_VER)
DYNAMIC_NAME = libreactor.so
STATIC_NAME = libreactor.a
DYNAMIC_LIB	= $(COMPILE_LIB_HOME)/$(DYNAMIC_NAME)
STATIC_LIB = $(COMPILE_LIB_HOME)/$(STATIC_NAME)

all: $(DYNAMIC_LIB) $(STATIC_LIB)

$(DYNAMIC_LIB): $(SRCS:.cpp=.o) Version.o 
	$(CXX) -pg -o $@ $^ $(CXXFLAGS) $(LINK)
	cp $(DYNAMIC_LIB) .
#	@-ln -sf $(DYNAMIC_NAME) $(COMPILE_LIB_HOME)/libcore.so

$(STATIC_LIB): $(SRCS:.cpp=.o) Version.o
	@ar cr $@ $^
	cp $(STATIC_LIB) .	

%.o: %.c Makefile
	$(CC) $(CFLAGS) $(INC) -c -pg -o $@ $<
#	@-mv -f $*.d .dep.$@
%.o: %.cc Makefile
	$(CXX) $(CXXFLAGS) $(INC) -c -pg -o $@ $<
#	@-mv -f $*.d .dep.$@
%.o: %.cpp Makefile
	$(CXX) $(CXXFLAGS) $(INC) -c -pg -o $@ $<
#	@-mv -f $*.d .dep.$@
clean:
	rm -f *.o .po *.so *.d .dep.*  $(SRCS:.cpp=.o) $(SRCS:.cpp=.d) $(DYNAMIC_LIB) $(STATIC_LIB) $(DYNAMIC_NAME) $(STATIC_NAME)
-include /dev/null $(wildcard .dep.*) 
