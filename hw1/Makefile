#
#	Makefile for OpenGL example from Chapter 1 in Red Book
#

CC = g++ 
CFLAGS = -D_DEBUG
CPPFLAGS = -g

.cpp.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@  $<

LIBS = -lGL -lGLU -lGLEW -lglut

SRCS = program1.cpp LoadShaders.cpp
OBJS = program1.o LoadShaders.o

program1: $(OBJS)
	g++ -g -o program1 $(OBJS) $(LIBS)

clean:
	rm -f program1 *.o
