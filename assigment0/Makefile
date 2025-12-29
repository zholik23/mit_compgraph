# Windows/MSYS2 Makefile
CC = g++

# 1. Compiler Flags
#    -Ivecmath  <-- This is the critical fix. It finds vecmath.h
#    -Iinclude  <-- This finds standard GL headers if you have them there
CFLAGS = -g -O2 -Wall -I. -Ivecmath -Iinclude

# 2. Libraries
LIBS = -Llib -lfreeglut -lglu32 -lopengl32 -static-libgcc -static-libstdc++

# 3. Source Files
SRCS = main.cpp $(wildcard vecmath/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = a0.exe

# 4. Build Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del $(TARGET) *.o vecmath\*.o