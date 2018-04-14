################################################################################
#
#  Makefile for Tuner
#
################################################################################

GCC	  = gcc
GPLUSPLUS = g++
WINDRES   = windres

all:	Tuner.exe

Tuner.exe:	Tuner.cpp Tuner.o Tuner.h Makefile

clean:
	rm *.exe *.o

%.exe:	%.cpp
	$(GPLUSPLUS) -o $@ $(wordlist 1, 2, $^) -Os -std=gnu++11 \
	-mwindows -lgdiplus -lcomctl32 -lwinmm -lmsimg32

%.o:	%.rc
	$(WINDRES) -o $@ $<
