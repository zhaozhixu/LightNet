.PHONY: all clean

CFLAGS = `pkg-config --cflags libjpeg lightnet`
LDFLAGS = `pkg-config --libs libjpeg lightnet`

all: object-detect

object-detect: object-detect.c jpeg_read.c
	gcc -Wall -g $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f object-detect
