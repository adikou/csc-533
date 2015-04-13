#
#	Makefile for OpenGL example from Chapter 1 in Red Book
#

CC = g++ 
CFLAGS = -D_DEBUG -I./glm -g -std=c++0x

.cpp.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@  $<

LIBS = -lGL -lGLU -lGLEW -lglut

SRCS = viewer.cpp LoadShaders.cpp
OBJS = viewer.o LoadShaders.o

all: $(OBJS)
	g++ -g -o viewer $(OBJS) $(LIBS)

clean:
	rm -f viewer *.o
