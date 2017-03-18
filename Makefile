TARGET   = tinyFsDemo
CC       = gcc
CCFLAGS  = -std=c99 -pedantic -Wall -Werror
LDFLAGS  = -lm
SOURCES = libDisk.c libTinyFS.c tinyFsDemo.c
INCLUDES = $(wildcard *.h)
OBJECTS  = $(SOURCES:.c=.o)
DISKS = $(wildcard *.dsk)

all:$(TARGET)

$(TARGET):$(OBJECTS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJECTS)

$(OBJECTS):$(SOURCES) $(INCLUDES)
	$(CC) -c $(CCFLAGS) $(SOURCES)

clean:
	rm -f $(TARGET) $(OBJECTS) $(DISKS)
