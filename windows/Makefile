################################################################################
#
#  Makefile for Tuner
#
################################################################################

GCC	= gcc
WINDRES = windres

all:	Tuner.exe SigGen.exe

Tuner.exe:	Tuner.c Tuner.o

SigGen.exe:	SigGen.c SigGen.o

clean:
	rm *.exe

%.exe:	%.c
	$(GCC) -o $@ $^ -O2 -march=opteron -ffast-math -std=gnu99 \
		-mwindows -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
