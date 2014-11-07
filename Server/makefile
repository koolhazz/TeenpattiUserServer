
BIN = CCReactor_UserServer

INC = -I. -I../include	-I/home/AustinChen/lib/include/libmemcached

FILES = $(wildcard *.cpp) 
C_FILES = 
SRC = $(FILES) $(C_FILES)
OBJ = $(patsubst %.c, %.o, $(C_FILES)) $(patsubst %.cpp, %.o, $(FILES))

LIBS = -ldl -lz -L../lib -lreactor -lmemcached -D__KEY_BASE=0x30111
CFLAGS = -fpermissive -g
BUILD = $(VBIN)

CFLAGS += -fPIC -D_MP_MODE

TARGET = $(BIN)



$(TARGET):$(OBJ)
	$(CXX) -o $@ $^ $(INC) $(CFLAGS) $(LIBS)
	install $(TARGET) ../bin/	
%.o:%.cpp
	$(CXX) $(CFLAGS) -c -o $@ $< $(INC) $(CFLAGS)
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $< $(INC) $(CFLAGS)
clean:
	rm -f $(OBJ) $(TARGET)

	