
CC = gcc
TARGET   = ac240server
OBJS     = ac240server.o ac240controller.o ac240socket.o
INCLUDES = -I../lib
LOADLIBS = -lpthread -lAqDrv4
MACROS   = -D_LINUX -D_ACQIRIS

all:		$(TARGET)

$(TARGET):	$(OBJS)
		$(CC) -o $(TARGET) $(OBJS) $(LOADLIBS)

ac240server.o: ac240server.c
		$(CC) -c ac240server.c $(MACROS) $(INCLUDES)

ac240controller.o: ac240controller.c
		$(CC) -c ac240controller.c $(MACROS) $(INCLUDES)

ac240socket.o: ac240socket.c
		$(CC) -c ac240socket.c $(MACROS) $(INCLUDES)

install:
		cp ac240server ../bin/ac240server

clean:
		rm -f $(TARGET) $(OBJS) *~
