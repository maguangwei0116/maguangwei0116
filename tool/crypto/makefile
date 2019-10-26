
TARGET = crypto
CC = gcc
CFLAGS = -g -Wall

HEADER_PATH = -I ./algo/inc/ \
              -I ./inc/pub/ \
              -I ./inc/tommath/

LIB_PATH = -L ./lib/x86/
LIBS = -ltommath

.PHONY: default all clean

default: $(TARGET)
all: default

SOURCE = $(wildcard *.c ./algo/src/*.c)
OBJECTS = $(patsubst %.c, %.o, $(SOURCE))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ $(HEADER_PATH)

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS) $(LIB_PATH)

.PHONY : clean
clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)