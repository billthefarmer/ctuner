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

#include <gtk/gtk.h>
#include <pthread.h>

#include <cairo/cairo.h>
#include <alsa/asoundlib.h>

enum
  {MARGIN = 10};

// Structs

typedef struct
{
    float f;
    float fr;
    int n;
} maximum;

typedef struct
{
    GtkWidget *widget;
    float *data;
    int length;
} Scope;

typedef struct
{
    GtkWidget *widget;
    int length;
    int expand;
    int count;
    gboolean zoom;
    float f;
    float r;
    float l;
    float h;
    float *data;
    float *values;
} Spectrum;

typedef struct
{
    GtkWidget *widget;
    maximum *maxima;
    float f;
    float fr;
    float c;
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
    float c;
} Strobe;

typedef struct
{
    GtkWidget *widget;
    GtkWidget *slider; 
    float c;
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
} Audio;

void *initAudio(void *);

gboolean scope_draw_callback(GtkWidget *, GdkEventExpose *, void *);
gboolean spectrum_draw_callback(GtkWidget *, GdkEventExpose *, void *);
gboolean display_draw_callback(GtkWidget *, GdkEventExpose *, void *);
gboolean strobe_draw_callback(GtkWidget *, GdkEventExpose *, void *);
gboolean meter_draw_callback(GtkWidget *, GdkEventExpose *, void *);

void quit_clicked(GtkWidget *, GtkWindow *);
