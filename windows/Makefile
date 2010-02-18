################################################################################
#
#  Makefile for Tuner
#
################################################################################

GCC	= gcc
WINDRES = windres

all:	Tuner.exe

Tuner.exe:	Tuner.c Tuner.o

clean:
	rm *.exe

%.exe:	%.c
	$(GCC) -o $* $< $*.o -march=opteron -std=gnu99 -mwindows -lcomctl32 -lwinmm

%.o:	%.rc
	$(WINDRES) -o $@ $<
