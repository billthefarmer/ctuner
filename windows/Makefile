################################################################################
#
#  Makefile for Tuner
#
################################################################################

GCC	= gcc
GXX     = g++
WINDRES = windres

all:	Tuner.exe

Tuner.exe:	Tuner.cpp Tuner.o

Tuner.o:	Tuner.rc Tuner.h Tuner.manifest Tuner.ico Toolbar.bmp Makefile

clean:
	rm *.exe *.o

%.exe:	%.cpp
	$(GXX) -o $@ $^ -Os -std=gnu++11 -static \
	-mwindows -lgdiplus -lcomctl32 -lwinmm -lmsimg32

%.o:	%.rc
	$(WINDRES) -o $@ $<
