////////////////////////////////////////////////////////////////////////////////
//
//  Tuner - A tuner written in C.
//
//  Copyright (C) 2014  Bill Farmer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with this program; if not, write to the Free Software Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//  Bill Farmer  william j farmer [at] yahoo [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <gtk/gtk.h>
#include <pthread.h>

#include <ft2build.h>
#include FT_FREETYPE_H 

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include <alsa/asoundlib.h>

// Macros

#define Length(a) (sizeof(a) / sizeof(a[0]))

#define OCTAVE    12
#define MINIMUM   0.5

enum
    {MARGIN = 10};

// PCM values

enum
    {SAMPLE_RATE = 11025,
     LATENCY = 50000,
     CHANNELS = 1};

// Audio processing values

enum
    {MAXIMA = 8,
     OVERSAMPLE = 16,
     SAMPLES = 16384,
     RANGE = SAMPLES * 3 / 8,
     STEP = SAMPLES / OVERSAMPLE};

// Tuner reference values

enum
    {A5_REFNCE = 440,
     C5_OFFSET = 57};

// Structs

typedef struct
{
    double r;
    double i;
} complex;

typedef struct
{
    double f;
    double fr;
    int n;
} maximum;

typedef struct
{
    double f;
    double r;
    double l;
    double h;
} value;

typedef struct
{
    GtkWidget *widget;
    short *data;
    int length;
} Scope;

typedef struct
{
    GtkWidget *widget;
    int length;
    int expand;
    int count;
    gboolean zoom;
    double f;
    double r;
    double l;
    double h;
    double *data;
    value *values;
} Spectrum;

typedef struct
{
    GtkWidget *widget;
    maximum *maxima;
    double f;
    double fr;
    double c;
    gboolean lock;
    gboolean zoom;
    gboolean multiple;
    int count;
    int n;
} Display;

typedef struct
{
    GtkWidget *widget;
    gboolean changed;
    gboolean enable;
    int colours;
    double c;
} Strobe;

typedef struct
{
    GtkWidget *widget;
    GtkWidget *slider; 
    double c;
} Meter;

typedef struct
{
    struct
    {
	GtkWidget *sample;
	GtkWidget *actual;
    } status;

    struct
    {
	GtkWidget *reference;
    } preferences;
} Legend;

typedef struct
{
    GtkWidget *zoom;
    GtkWidget *lock;
    GtkWidget *strobe;
    GtkWidget *filter;
    GtkWidget *multiple;
    GtkWidget *downsample;
} Check;

typedef struct
{
    pthread_t thread;
    snd_pcm_t *handle;
    gboolean done;
    gboolean filter;
    gboolean downsample;
    double correction;
    double reference;
} Audio;

void initAudio(void);
void *readAudio(void *);

gboolean scope_draw_callback(GtkWidget *, GdkEventExpose *, void *);
gboolean spectrum_draw_callback(GtkWidget *, GdkEventExpose *, void *);
gboolean display_draw_callback(GtkWidget *, GdkEventExpose *, void *);
gboolean strobe_draw_callback(GtkWidget *, GdkEventExpose *, void *);
gboolean meter_draw_callback(GtkWidget *, GdkEventExpose *, void *);

void options_clicked(GtkWidget *, GtkWindow *);
void quit_clicked(GtkWidget *, GtkWindow *);

void fftr(complex[], int);
