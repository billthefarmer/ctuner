################################################################################
#
#  Makefile for Tuner
#
################################################################################

GCC	= gcc
G++	= g++
WINDRES = windres

all:	Tuner.exe

Tuner.exe:	Tuner.cpp Tuner.o Tuner.h Makefile

clean:
	rm *.exe *.o

%.exe:	%.cpp
	$(G++) -o $@ $(wordlist 1, 2, $^) -Os -std=gnu++11 \
	-mwindows -lgdiplus -lcomctl32 -lwinmm -lmsimg32

%.o:	%.rc
	$(WINDRES) -o $@ $<
