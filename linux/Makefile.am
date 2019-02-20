bin_PROGRAMS = tuner
tuner_SOURCES = tuner.cpp
AM_CFLAGS = $(GTK_CFLAGS) $(ALSA_CFLAGS) -O -fPIC -Wl,-z,relro -Wall
LIBS = $(GTK_LIBS) $(ALSA_LIBS)  -lm
