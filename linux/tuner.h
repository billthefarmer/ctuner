////////////////////////////////////////////////////////////////////////////////
//
//  Tuner - A tuner written in C++.
//
//  Copyright (C) 2019  Bill Farmer
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

#ifndef TUNER_H
#define TUNER_H

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

#define APP_ID "org.billthefarmer.tuner"
#define MINIMUM 0.5

// Layout values
enum
    {MARGIN          = 20,
     SPACING         = 8,
     DISPLAY_WIDTH   = 340,
     SCOPE_HEIGHT    = 48,
     SPECTRUM_HEIGHT = 48,
     DISPLAY_HEIGHT  = 150,
     STROBE_HEIGHT   = 66,
     STAFF_HEIGHT    = 66,
     METER_HEIGHT    = 78};

// PCM values
enum
    {SAMPLE_RATE = 11025,
     LATENCY     = 50000,
     CHANNELS    = 1};

// Audio processing values
enum
    {MAXIMA     = 8,
     OVERSAMPLE = 16,
     SAMPLES    = 16384,
     RANGE      = SAMPLES * 7 / 16,
     STEP       = SAMPLES / OVERSAMPLE};

// Tuner reference values
enum
    {A5_REFERENCE = 440,
     C5_OFFSET    = 57,
     A_OFFSET     = 9,
     OCTAVE       = 12,
     EQUAL        = 8};

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
    gint n;
} maximum;

typedef struct
{
    GtkWidget *widget;
    short *data;
    gint length;
} Scope;

typedef struct
{
    GtkWidget *widget;
    gint length;
    gint expand;
    gint count;
    gboolean zoom;
    double f;
    double r;
    double l;
    double h;
    double *data;
    double *values;
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
    gint transpose;
    gint count;
    gint n;
} Display;

typedef struct
{
    GtkWidget *widget;
    gboolean changed;
    gboolean enable;
    gint colours;
    double c;
} Strobe;

typedef struct
{
    GtkWidget *widget;
    gboolean enable;
    gint transpose;
    gint n;
} Staff;

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
    GtkWidget *window;
    GtkWidget *key;
    GtkWidget *note;
    GtkWidget *zoom;
    GtkWidget *lock;
    GtkWidget *strobe;
    GtkWidget *filter;
    GtkWidget *expand;
    GtkWidget *colours;
    GtkWidget *multiple;
    GtkWidget *transpose;
    GtkWidget *downsample;
    GtkWidget *fundamental;
    GtkWidget *reference;
    GtkWidget *correction;
    GtkWidget *temperament;
} Options;

typedef struct
{
    GtkWidget *window;
    gboolean notes[12];
    gboolean octaves[9];
} Filters;

typedef struct
{
    pthread_t thread;
    snd_pcm_t *handle;
    gboolean done;
    gboolean note;
    gboolean filter;
    gboolean downsample;
    gboolean fundamental;
    gint temperament;
    gint key;
    double reference;
} Audio;

// Data
Scope scope;
Spectrum spectrum;
Display display;
Strobe strobe;
Staff staff;
Meter meter;
Status status;

Options options;
Filters filters;

Audio audio;

// Temperaments
double temperaments[32][12] =
#include "temperaments.h"

// Functions
void initAudio(void);
void *readAudio(void *);

gboolean scope_draw_callback(GtkWidget *, cairo_t *, gpointer);
gboolean spectrum_draw_callback(GtkWidget *, cairo_t *, gpointer);
gboolean display_draw_callback(GtkWidget *, cairo_t *, gpointer);
gboolean strobe_draw_callback(GtkWidget *, cairo_t *, gpointer);
gboolean staff_draw_callback(GtkWidget *, cairo_t *, gpointer);
gboolean meter_draw_callback(GtkWidget *, cairo_t *, gpointer);

gboolean key_press(GtkWidget*, GdkEventKey*, gpointer);
gboolean button_press(GtkWidget*, GdkEventButton*, gpointer);

void widget_queue_draw(gpointer);
void activate(GtkApplication *, gpointer);
void options_clicked(GtkWidget *, GtkWindow *);
void save_clicked(GtkWidget *, GtkWindow *);

void saveOptions(void);
void restoreOptions(void);

void fftr(complex[], int);

#endif
