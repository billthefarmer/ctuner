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

#include "tuner.h"

// Data

Scope scope;
Spectrum spectrum;
Display display;
Strobe strobe;
Meter meter;

Audio audio;

// Main function

int main(int argc, char *argv[])
{
    // Widgets

    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *wbox;
    GtkWidget *hbox;
    GtkWidget *quit;
    GtkWidget *label;
    GtkWidget *separator;

    // Initialise threads

    gdk_threads_init();
    gdk_threads_enter();

    // Initialise GTK

    gtk_init(&argc, &argv);

    // Create main window

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tuner");
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    // V box, this contains the fake status bar and the rest of the
    // display

    vbox = gtk_vbox_new(FALSE, MARGIN);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Label, this label and separator are a fake status bar that can
    // have small text

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label),
			 "Test.");
    gtk_box_pack_end(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    // Separator

    separator = gtk_hseparator_new();
    gtk_box_pack_end(GTK_BOX(vbox), separator, FALSE, FALSE, 0);

    // H box

    hbox = gtk_hbox_new(FALSE, MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, MARGIN);

    // V box

    vbox = gtk_vbox_new(FALSE, MARGIN);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, MARGIN);

    // Scope

    scope.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(scope.widget, 280, 32);
    gtk_box_pack_start(GTK_BOX(vbox), scope.widget, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(scope.widget), "expose_event",
		     G_CALLBACK(scope_draw_callback), NULL);

    // Spectrum

    spectrum.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(spectrum.widget, 280, 32);
    gtk_box_pack_start(GTK_BOX(vbox), spectrum.widget, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(spectrum.widget), "expose_event",
		     G_CALLBACK(spectrum_draw_callback), NULL);

    // Display

    display.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(display.widget, 280, 100);
    gtk_box_pack_start(GTK_BOX(vbox), display.widget, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(display.widget), "expose_event",
		     G_CALLBACK(display_draw_callback), NULL);

    // Strobe

    strobe.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(strobe.widget, 280, 44);
    gtk_box_pack_start(GTK_BOX(vbox), strobe.widget, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(strobe.widget), "expose_event",
		     G_CALLBACK(strobe_draw_callback), NULL);

    // Meter

    meter.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(meter.widget, 280, 52);
    gtk_box_pack_start(GTK_BOX(vbox), meter.widget, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(meter.widget), "expose_event",
		     G_CALLBACK(meter_draw_callback), NULL);

    // H box

    hbox = gtk_hbox_new(FALSE, MARGIN);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // Quit button

    quit = gtk_button_new_with_label("  Quit  ");
    gtk_box_pack_end(GTK_BOX(hbox), quit, FALSE, FALSE, 0);

    // Quit clicked

    g_signal_connect(G_OBJECT(quit), "clicked",
		     G_CALLBACK(quit_clicked), window);

    // Destroy window callback

    g_signal_connect(window, "destroy",
		     G_CALLBACK(gtk_main_quit), NULL);

    // Show the window

    gtk_widget_show_all(window);

    // Start audio

    initAudio();

    // Interact with user

    gtk_main();

    // Stop audio

    audio.done = TRUE;
    pthread_join(audio.thread, NULL);

    gdk_threads_leave();

    // Exit

    return 0;
}

// Init audio

void initAudio(void)
{
    int err, dir;
    unsigned rate_min, rate_max;
    snd_pcm_hw_params_t *hwparams;

    if ((err = snd_pcm_open(&audio.handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
	printf("Capture open error: %s\n", snd_strerror(err));
	return;
    }

    if ((err = snd_pcm_hw_params_malloc(&hwparams)) < 0)
    {
	printf("Capture allocate error: %s\n", snd_strerror(err));
	return;
    }

    if ((err = snd_pcm_hw_params_any(audio.handle, hwparams)) < 0)
    {
	printf("Setting of hwparams failed: %s\n", snd_strerror(err));
	return;
    }

    if ((err = snd_pcm_hw_params_get_rate_min(hwparams, &rate_min, &dir)) < 0)
    {
	printf("Getting of min rate failed: %s\n", snd_strerror(err));
	return;
    }

    if ((err = snd_pcm_hw_params_get_rate_max(hwparams, &rate_max, &dir)) < 0)
    {
	printf("Getting of max rate failed: %s\n", snd_strerror(err));
	return;
    }

    if ((SAMPLE_RATE < rate_min) || (SAMPLE_RATE > rate_max))
    {
	printf("Required ample rate %d outside available range %d - %d\n",
	       SAMPLE_RATE, rate_min, rate_max);
	return;
    }

    if ((err = snd_pcm_set_params(audio.handle, SND_PCM_FORMAT_S16,
				  SND_PCM_ACCESS_RW_INTERLEAVED,
				  CHANNELS, SAMPLE_RATE,
				  1, LATENCY)) < 0)
    {
	printf("Setting parameters failed: %s\n", snd_strerror(err));
	return;
    }

    // Create the audio thread

    pthread_create(&audio.thread, NULL, readAudio, NULL);

}

// Read audio

void *readAudio(void *dummy)
{
    int err;
    snd_pcm_sframes_t frames;

    // Create buffers for processing the audio data

    static double buffer[SAMPLES];
    static complex x[SAMPLES];

    static double xa[RANGE];
    static double xp[RANGE];
    static double xf[RANGE];

    static double x2[RANGE / 2];
    static double x3[RANGE / 3];
    static double x4[RANGE / 4];
    static double x5[RANGE / 5];

    static double dx[RANGE];

    static maximum maxima[MAXIMA];
    static value   values[MAXIMA];

    static double fps = (double)SAMPLE_RATE / (double)SAMPLES;
    static double expect = 2.0 * M_PI * (double)STEP / (double)SAMPLES;

    static short data[STEP];

    // initialise data structs

    if (scope.data == NULL)
    {
	scope.data = data;
	scope.length = STEP;

	spectrum.data = xa;
	spectrum.length = RANGE;

	spectrum.values = values;
	display.maxima = maxima;
    }

    while (!audio.done)
    {
	if ((frames = snd_pcm_readi(audio.handle, data, STEP)) < 0)
	    break;


	// gdk_threads_enter();
	gtk_widget_queue_draw(scope.widget);
	// gdk_threads_leave();

    }

    if (frames < 0)
	printf("Read error: %s\n", snd_strerror(frames));

    if ((err = snd_pcm_close(audio.handle)) < 0)
    {
	printf("Capture close error: %s\n", snd_strerror(err));
	return NULL;
    }
}

// Round rect

void cairo_round_rect(cairo_t *cr, double x, double y,
		      double w, double h, double r)
{
    cairo_move_to(cr, x + r, y);
    cairo_line_to(cr, x + w - r, y);
    cairo_arc(cr, x + w - r, y + r, r, -M_PI / 2, 0);
    cairo_line_to(cr, x + w, y + h - r);
    cairo_arc(cr, x + w - r, y + h - r, r, 0, M_PI / 2);
    cairo_line_to(cr, x + r, y + h);
    cairo_arc(cr, x + r, y + h - r, r, M_PI / 2, M_PI);
    cairo_line_to(cr, x, y + r);
    cairo_arc(cr, x + r, y + r, r, M_PI, M_PI * 3 / 2);
}

// Draw edge

void cairo_edge(cairo_t *cr, int w, int h)
{
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_round_rect(cr, 0, 0, w, h, 5);
    cairo_stroke(cr);

    cairo_rectangle(cr, 2, 2, w - 4, h - 4);
    cairo_clip(cr);

    cairo_translate(cr, 2, 2);
}

// Scope draw callback

gboolean scope_draw_callback(GtkWidget *widget, GdkEventExpose *event,
			     void *data)
{
    cairo_t *cr = gdk_cairo_create(event->window);

    cairo_edge(cr,  widget->allocation.width, widget->allocation.height);

    int width = widget->allocation.width - 4;
    int height = widget->allocation.height - 4;

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    cairo_translate(cr, 0, height / 2);
    cairo_set_source_rgb(cr, 0, 0.2, 0);

    for (int x = 0; x < width; x += 5)
    {
	cairo_move_to(cr, x, -height / 2);
	cairo_line_to(cr, x, height / 2);
    }

    for (int y = 0; y < height / 2; y += 5)
    {
	cairo_move_to(cr, 0, y);
	cairo_line_to(cr, width, y);
	cairo_move_to(cr, 0, -y);
	cairo_line_to(cr, width, -y);
    }

    cairo_stroke(cr);

    // Don't attempt the trace until there's a buffer

    if (scope.data == NULL)
	return TRUE;

    // Initialise sync

    int maxdx = 0;
    int dx = 0;
    int n = 0;

    for (int i = 1; i < width; i++)
    {
	dx = scope.data[i] - scope.data[i - 1];
	if (maxdx < dx)
	{
	    maxdx = dx;
	    n = i;
	}

	if (maxdx > 0 && dx < 0)
	    break;
    }

    static int max;

    if (max < 4096)
	max = 4096;

    int yscale = max / (height / 2);

    max = 0;

    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);

    cairo_move_to(cr, 0, 0);
    for (int i = 0; i < width; i++)
    {
	if (max < abs(scope.data[n + i]))
	    max = abs(scope.data[n + i]);

	int y = -scope.data[n + i] / yscale;
	cairo_line_to(cr, i, y);
    }

    cairo_stroke(cr);
    cairo_destroy(cr);

    return TRUE;
}

// Spectrum draw callback

gboolean spectrum_draw_callback(GtkWidget *widget, GdkEventExpose *event,
				void *data)
{
    cairo_t *cr = gdk_cairo_create(event->window);

    cairo_edge(cr,  widget->allocation.width, widget->allocation.height);

    int width = widget->allocation.width - 4;
    int height = widget->allocation.height - 4;

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    cairo_translate(cr, 0, height);
    cairo_set_source_rgb(cr, 0, 0.5, 0);

    for (int x = 0; x < width; x += 5)
    {
	cairo_move_to(cr, x, -height);
	cairo_line_to(cr, x, 0);
    }

    for (int y = 0; y < height; y += 5)
    {
	cairo_move_to(cr, 0, -y);
	cairo_line_to(cr, width, -y);
    }

    cairo_stroke(cr);
    cairo_destroy(cr);

    return TRUE;
}

// Display draw callback

gboolean display_draw_callback(GtkWidget *widget, GdkEventExpose *event,
			       void *data)
{
    cairo_t *cr = gdk_cairo_create(event->window);

    cairo_edge(cr,  widget->allocation.width, widget->allocation.height);

    cairo_destroy(cr);

    return TRUE;
}

// Strobe draw callback

gboolean strobe_draw_callback(GtkWidget *widget, GdkEventExpose *event,
			      void *data)
{
    cairo_t *cr = gdk_cairo_create(event->window);

    cairo_edge(cr,  widget->allocation.width, widget->allocation.height);

    cairo_set_source_rgb(cr, 0, 0, 1);
    cairo_paint(cr);
    cairo_destroy(cr);

    return TRUE;
}

// Meter draw callback

gboolean meter_draw_callback(GtkWidget *widget, GdkEventExpose *event,
			     void *data)
{
    cairo_t *cr = gdk_cairo_create(event->window);

    cairo_edge(cr,  widget->allocation.width, widget->allocation.height);

    cairo_set_source_rgb(cr, 0, 1, 1);
    cairo_paint(cr);
    cairo_destroy(cr);

    return TRUE;
}

// Quit callback

void quit_clicked(GtkWidget *widget, GtkWindow *window)
{
    // Create a message dialog
#ifdef QUERY_QUIT
    GtkWidget *dialog =
	gtk_message_dialog_new(GTK_WINDOW(window),
			       GTK_DIALOG_MODAL,
			       GTK_MESSAGE_QUESTION,
			       GTK_BUTTONS_OK_CANCEL,
			       "Quit");
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
					     "Really?");
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (result == GTK_RESPONSE_OK)
#endif
	gtk_main_quit();
}
