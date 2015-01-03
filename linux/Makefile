################################################################################
#
#  Makefile for Tuner
#
################################################################################

GCC	= gcc
CFLAGS  = -O2 -fPIC -std=gnu99 -Wl,-z,relro
LIBS    = -lX11 -lasound -lm

all:	tuner

clean:
	rm tuner

%:	%.c
	$(GCC) $< -o $@ $(CFLAGS) $(LIBS) \
		`pkg-config --cflags --libs gtk+-2.0`
