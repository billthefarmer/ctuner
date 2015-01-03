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
#include <gdk/gdkkeysyms.h>

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

// Layout values

enum
    {MARGIN = 8,
     DISPLAY_WIDTH = 300,
     SCOPE_HEIGHT = 32,
     SPECTRUM_HEIGHT = 32,
     DISPLAY_HEIGHT = 100,
     STROBE_HEIGHT = 44,
     METER_HEIGHT = 52};

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
    {A5_REFERENCE = 440,
     C5_OFFSET = 57};

// Strobe colours

enum
    {BLUE,
     OLIVE,
     MAGENTA};

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
    GtkWidget *widget;
} Status;

typedef struct
{
    GtkWidget *dialog;
    GtkWidget *zoom;
    GtkWidget *lock;
    GtkWidget *strobe;
    GtkWidget *filter;
    GtkWidget *multiple;
    GtkWidget *downsample;
    GtkWidget *reference;
    GtkWidget *correction;
} Options;

typedef struct
{
    pthread_t thread;
    snd_pcm_t *handle;
    gboolean done;
    gboolean filter;
    gboolean downsample;
    double correction;
    double reference;
    gboolean save;
} Audio;

void restoreOptions(void);
void initAudio(void);
void *readAudio(void *);

gboolean scope_draw_callback(GtkWidget *, GdkEventExpose *, void *);
gboolean spectrum_draw_callback(GtkWidget *, GdkEventExpose *, void *);
gboolean display_draw_callback(GtkWidget *, GdkEventExpose *, void *);
gboolean strobe_draw_callback(GtkWidget *, GdkEventExpose *, void *);
gboolean meter_draw_callback(GtkWidget *, GdkEventExpose *, void *);

gboolean key_press(GtkWidget*, GdkEventKey*, void *);
gboolean button_press(GtkWidget*, GdkEventButton*, void *);

void options_clicked(GtkWidget *, GtkWindow *);
void save_clicked(GtkWidget *, GtkWindow *);

void saveOptions();
void restoreOptions();

void fftr(complex[], int);
