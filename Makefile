# Minimal Makefile for process-track

CFLAGS = -O

process-track: process-track.c
test: test.c

clean:
	$(RM) process-track
