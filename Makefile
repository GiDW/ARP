# Makefile for arp
CFLAGS=-Wall
EXE=arp
EXE_TEST=test

clean:
	rm -rf *.o $(EXE) $(EXE_TEST)