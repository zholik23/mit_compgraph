# Windows/MSYS2 Makefile for 6.837

# 1. Include paths: Look in current dir and vecmath/include
INCFLAGS  = -I. -Ivecmath/include

# 2. Link flags: Standard Windows OpenGL libraries
LINKFLAGS = -lfreeglut -lglu32 -lopengl32

CFLAGS    = -O2 -Wall
CC        = g++

# 3. Sources: Your main files AND the vecmath files (wildcard finds them all)
# We assume vecmath sources are in a 'src' subdirectory. 
# If they are just in 'vecmath', change to vecmath/*.cpp
SRCS      = main.cpp parse.cpp curve.cpp surf.cpp camera.cpp $(wildcard vecmath/src/*.cpp)
OBJS      = $(SRCS:.cpp=.o)
PROG      = a1.exe

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LINKFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -c -o $@ $(INCFLAGS)

clean:
	rm -f $(OBJS) $(PROG)