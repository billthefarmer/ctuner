################################################################################
#
#  Makefile for Tuner
#
################################################################################

GCC	= gcc
GXX     = g++
CFLAGS  = -O -fPIC -Wl,-z,relro -Wall
LIBS    = -lX11 -lasound -lm

all:	tuner

clean:
	rm tuner

tuner:	tuner.cpp tuner.h temperaments.h Makefile

%:	%.cpp
	$(GXX) $< -o $@ $(CFLAGS) $(LIBS) \
		`pkg-config --cflags --libs gtk+-3.0`
