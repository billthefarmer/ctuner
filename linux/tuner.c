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

Options options;

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
    GtkWidget *options;
    GtkWidget *separator;

    // Initialise threads

    gdk_threads_init();
    gdk_threads_enter();

    // Restore optiond

    restoreOptions();

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
    gtk_widget_set_size_request(scope.widget, 300, 32);
    gtk_box_pack_start(GTK_BOX(vbox), scope.widget, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(scope.widget), "expose_event",
		     G_CALLBACK(scope_draw_callback), NULL);

    // Spectrum

    spectrum.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(spectrum.widget, 300, 32);
    gtk_box_pack_start(GTK_BOX(vbox), spectrum.widget, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(spectrum.widget), "expose_event",
		     G_CALLBACK(spectrum_draw_callback), NULL);

    // Display

    display.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(display.widget, 300, 100);
    gtk_box_pack_start(GTK_BOX(vbox), display.widget, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(display.widget), "expose_event",
		     G_CALLBACK(display_draw_callback), NULL);

    // Strobe

    strobe.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(strobe.widget, 300, 44);
    gtk_box_pack_start(GTK_BOX(vbox), strobe.widget, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(strobe.widget), "expose_event",
		     G_CALLBACK(strobe_draw_callback), NULL);

    // Meter

    meter.widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(meter.widget, 300, 52);
    gtk_box_pack_start(GTK_BOX(vbox), meter.widget, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(meter.widget), "expose_event",
		     G_CALLBACK(meter_draw_callback), NULL);

    // H box

    hbox = gtk_hbox_new(FALSE, MARGIN);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // Options button

    options = gtk_button_new_with_label(" Options... ");
    gtk_box_pack_start(GTK_BOX(hbox), options, FALSE, FALSE, 0);

    // Options clicked

    g_signal_connect(G_OBJECT(options), "clicked",
		     G_CALLBACK(options_clicked), window);

    // Quit button

    quit = gtk_button_new_with_label("  Quit  ");
    gtk_box_pack_end(GTK_BOX(hbox), quit, FALSE, FALSE, 0);

    // Quit clicked

    g_signal_connect(G_OBJECT(quit), "clicked",
		     G_CALLBACK(gtk_main_quit), NULL);

    // Key pressed callback

    g_signal_connect(G_OBJECT(window), "key-press-event",
		     G_CALLBACK(key_press), NULL);

    // Button pressed callback

    g_signal_connect(G_OBJECT(window), "button-press-event",
		     G_CALLBACK(button_press), NULL);

    gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);

    // Destroy window callback

    g_signal_connect(G_OBJECT(window), "destroy",
		     G_CALLBACK(gtk_main_quit), NULL);

    // Show the window

    gtk_widget_show_all(window);

    // Start audio

    initAudio();

    // Interact with user

    gtk_main();

    // Stop audio

    audio.done = TRUE;

    snd_pcm_close(audio.handle);

    gdk_threads_leave();

    // Exit

    return 0;
}

// Restore options

void restoreOptions()
{
    // Initial values

    spectrum.zoom = TRUE;
    spectrum.expand = 1;

    audio.reference = A5_REFERENCE;
    audio.correction = 1.0;
}

// Init audio

void initAudio(void)
{
    int err, dir;
    unsigned rate_min, rate_max;
    snd_pcm_hw_params_t *hwparams;

    if ((err = snd_pcm_open(&audio.handle, "default",
			    SND_PCM_STREAM_CAPTURE, 0)) < 0)
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
    enum
    {TIMER_COUNT = 16};

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

	// Copy the input data

	memmove(buffer, buffer + STEP, (SAMPLES - STEP) * sizeof(double));

	// Butterworth filter, 3dB/octave

	for (int i = 0; i < STEP; i++)
	{
	    static double G = 3.023332184e+01;
	    static double K = 0.9338478249;

	    static double xv[2];
	    static double yv[2];

	    xv[0] = xv[1];
	    xv[1] = (double)data[i] / G;

	    yv[0] = yv[1];
	    yv[1] = (xv[0] + xv[1]) + (K * yv[0]);

	    // Choose filtered/unfiltered data

	    buffer[(SAMPLES - STEP) + i] =
		audio.filter? yv[1]: (double)data[i];
	}

	// Maximum data value

	static double dmax;

	if (dmax < 4096.0)
	    dmax = 4096.0;

	// Calculate normalising value

	double norm = dmax;
	dmax = 0.0;

	// Copy data to FFT input arrays for tuner

	for (int i = 0; i < SAMPLES; i++)
	{
	    // Find the magnitude

	    if (dmax < fabs(buffer[i]))
		dmax = fabs(buffer[i]);

	    // Calculate the window

	    double window =
		0.5 - 0.5 * cos(2.0 * M_PI *
				i / SAMPLES);

	    // Normalise and window the input data

	    x[i].r = (double)buffer[i] / norm * window;
	}

	// do FFT for tuner

	fftr(x, SAMPLES);

	// Process FFT output for tuner

	for (int i = 1; i < RANGE; i++)
	{
	    double real = x[i].r;
	    double imag = x[i].i;

	    xa[i] = hypot(real, imag);

	    // Do frequency calculation

	    double p = atan2(imag, real);
	    double dp = xp[i] - p;
	    xp[i] = p;

	    // Calculate phase difference

	    dp -= (double)i * expect;

	    int qpd = dp / M_PI;

	    if (qpd >= 0)
		qpd += qpd & 1;

	    else
		qpd -= qpd & 1;

	    dp -=  M_PI * (double)qpd;

	    // Calculate frequency difference

	    double df = OVERSAMPLE * dp / (2.0 * M_PI);

	    // Calculate actual frequency from slot frequency plus
	    // frequency difference and correction value

	    xf[i] = (i * fps + df * fps) / audio.correction;

	    // Calculate differences for finding maxima

	    dx[i] = xa[i] - xa[i - 1];
	}

	// Downsample

	if (audio.downsample)
	{
	    // x2 = xa << 2

	    for (int i = 0; i < Length(x2); i++)
	    {
		x2[i] = 0.0;

		for (int j = 0; j < 2; j++)
		    x2[i] += xa[(i * 2) + j] / 2.0;
	    }

	    // x3 = xa << 3

	    for (int i = 0; i < Length(x3); i++)
	    {
		x3[i] = 0.0;

		for (int j = 0; j < 3; j++)
		    x3[i] += xa[(i * 3) + j] / 3.0;
	    }

	    // x4 = xa << 4

	    for (int i = 0; i < Length(x4); i++)
	    {
		x4[i] = 0.0;

		for (int j = 0; j < 4; j++)
		    x2[i] += xa[(i * 4) + j] / 4.0;
	    }

	    // x5 = xa << 5

	    for (int i = 0; i < Length(x5); i++)
	    {
		x5[i] = 0.0;

		for (int j = 0; j < 5; j++)
		    x5[i] += xa[(i * 5) + j] / 5.0;
	    }

	    // Add downsamples

	    for (int i = 1; i < Length(xa); i++)
	    {
		if (i < Length(x2))
		    xa[i] += x2[i];

		if (i < Length(x3))
		    xa[i] += x3[i];

		if (i < Length(x4))
		    xa[i] += x4[i];

		if (i < Length(x5))
		    xa[i] += x5[i];

		// Recalculate differences

		dx[i] = xa[i] - xa[i - 1];
	    }
	}

	// Maximum FFT output

	double max = 0.0;
	double f = 0.0;

	int count = 0;
	int limit = RANGE - 1;

	// Find maximum value, and list of maxima

	for (int i = 1; i < limit; i++)
	{
	    if (xa[i] > max)
	    {
		max = xa[i];
		f = xf[i];
	    }

	    // If display not locked, find maxima and add to list

	    if (!display.lock && count < Length(maxima) &&
		xa[i] > MINIMUM && xa[i] > (max / 4.0) &&
		dx[i] > 0.0 && dx[i + 1] < 0.0)
	    {
		maxima[count].f = xf[i];

		// Cents relative to reference

		double cf =
		    -12.0 * log2(audio.reference / xf[i]);

		// Reference note

		maxima[count].fr = audio.reference * pow(2.0, round(cf) / 12.0);

		// Note number

		maxima[count].n = round(cf) + C5_OFFSET;

		// Set limit to octave above

		if (!audio.downsample && (limit > i * 2))
		    limit = i * 2 - 1;

		count++;
	    }
	}

	// Reference note frequency and lower and upper limits

	double fr = 0.0;
	double fl = 0.0;
	double fh = 0.0;

	// Note number

	int n = 0;

	// Found flag and cents value

	gboolean found = FALSE;
	double c = 0.0;

	// Do the note and cents calculations

	if (max > MINIMUM)
	{
	    found = TRUE;

	    // Frequency

	    if (!audio.downsample)
		f = maxima[0].f;

	    // Cents relative to reference

	    double cf =
		-12.0 * log2(audio.reference / f);

	    // Reference note

	    fr = audio.reference * pow(2.0, round(cf) / 12.0);

	    // Lower and upper freq

	    fl = audio.reference * pow(2.0, (round(cf) - 0.55) / 12.0);
	    fh = audio.reference * pow(2.0, (round(cf) + 0.55) / 12.0);

	    // Note number

	    n = round(cf) + C5_OFFSET;

	    if (n < 0)
		found = FALSE;

	    // Find nearest maximum to reference note

	    double df = 1000.0;

	    for (int i = 0; i < count; i++)
	    {
		if (fabs(maxima[i].f - fr) < df)
		{
		    df = fabs(maxima[i].f - fr);
		    f = maxima[i].f;
		}
	    }

	    // Cents relative to reference note

	    c = -12.0 * log2(fr / f);

	    // Ignore silly values

	    if (!isfinite(c))
		c = 0.0;

	    // Ignore if not within 50 cents of reference note

	    if (fabs(c) > 0.5)
		found = FALSE;
	}

	// If display not locked

	gdk_threads_enter();

	if (!display.lock)
	{
	    // Update scope window

	    gtk_widget_queue_draw(scope.widget);

	    // Update spectrum window

	    for (int i = 0; i < count; i++)
		values[i].f = maxima[i].f / fps * audio.correction;

	    spectrum.count = count;

	    if (found)
	    {
		spectrum.f = f  / fps * audio.correction;
		spectrum.r = fr / fps * audio.correction;
		spectrum.l = fl / fps * audio.correction;
		spectrum.h = fh / fps * audio.correction;
	    }

	    gtk_widget_queue_draw(spectrum.widget);
	}

	static long timer;

	if (found)
	{
	    // If display not locked

	    if (!display.lock)
	    {
		// Update the display struct

		display.f = f;
		display.fr = fr;
		display.c = c;
		display.n = n;
		display.count = count;

		// Update meter

		meter.c = c;

		// Update strobe

		strobe.c = c;
	    }

	    // Update display

	    gtk_widget_queue_draw(display.widget);

	    // Reset count;

	    timer = 0;
	}

	else
	{
	    // If display not locked

	    if (!display.lock)
	    {

		if (timer > TIMER_COUNT)
		{
		    display.f = 0.0;
		    display.fr = 0.0;
		    display.c = 0.0;
		    display.n = 0;
		    display.count = 0;

		    // Update meter

		    meter.c = 0.0;

		    // Update strobe

		    strobe.c = 0.0;

		    // Update spectrum

		    spectrum.f = 0.0;
		    spectrum.r = 0.0;
		    spectrum.l = 0.0;
		    spectrum.h = 0.0;
		}

		// Update display

		gtk_widget_queue_draw(display.widget);
	    }
	}

	gdk_threads_leave();

	timer++;
    }

    if (frames < 0)
	printf("Read error: %s\n", snd_strerror(frames));

    if ((err = snd_pcm_close(audio.handle)) < 0)
    {
	printf("Capture close error: %s\n", snd_strerror(err));
	return NULL;
    }
}

// Real to complex FFT, ignores imaginary values in input array

void fftr(complex a[], int n)
{
    double norm = sqrt(1.0 / n);

    for (int i = 0, j = 0; i < n; i++)
    {
	if (j >= i)
	{
	    double tr = a[j].r * norm;

	    a[j].r = a[i].r * norm;
	    a[j].i = 0.0;

	    a[i].r = tr;
	    a[i].i = 0.0;
	}

	int m = n / 2;
	while (m >= 1 && j >= m)
	{
	    j -= m;
	    m /= 2;
	}
	j += m;
    }
    
    for (int mmax = 1, istep = 2 * mmax; mmax < n;
	 mmax = istep, istep = 2 * mmax)
    {
	double delta = (M_PI / mmax);
	for (int m = 0; m < mmax; m++)
	{
	    double w = m * delta;
	    double wr = cos(w);
	    double wi = sin(w);

	    for (int i = m; i < n; i += istep)
	    {
		int j = i + mmax;
		double tr = wr * a[j].r - wi * a[j].i;
		double ti = wr * a[j].i + wi * a[j].r;
		a[j].r = a[i].r - tr;
		a[j].i = a[i].i - ti;
		a[i].r += tr;
		a[i].i += ti;
	    }
	}
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

// Right justify text

void cairo_right_justify_text(cairo_t *cr, char *t)
{
    double x, y;
    cairo_text_extents_t extents;

    cairo_get_current_point(cr, &x, &y);
    cairo_text_extents(cr, t, &extents);

    cairo_move_to(cr, x - extents.x_advance, y);
    cairo_show_text(cr, t);
}

// Centre text

void cairo_centre_text(cairo_t *cr, char *t)
{
    double x, y;
    cairo_text_extents_t extents;

    cairo_get_current_point(cr, &x, &y);
    cairo_text_extents(cr, t, &extents);

    cairo_move_to(cr, x - extents.width / 2, y);
    cairo_show_text(cr, t);
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
    cairo_set_source_rgb(cr, 0, 0.5, 0);
    cairo_set_line_width(cr, 1);

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

    // Show F for filter

    if (audio.filter)
    {
	cairo_set_source_rgb(cr, 1, 1, 0);
	cairo_move_to(cr, 2, -(height / 2) + 10);
	cairo_show_text(cr, "F");
    }

    cairo_destroy(cr);

    return TRUE;
}

// Spectrum draw callback

gboolean spectrum_draw_callback(GtkWidget *widget, GdkEventExpose *event,
				void *data)
{
    static char s[16];

    enum
    {FONT_HEIGHT   = 10};

    cairo_t *cr = gdk_cairo_create(event->window);

    cairo_edge(cr,  widget->allocation.width, widget->allocation.height);

    int width = widget->allocation.width - 4;
    int height = widget->allocation.height - 4;

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    cairo_translate(cr, 0, height - 1);
    cairo_set_source_rgb(cr, 0, 0.5, 0);
    cairo_set_line_width(cr, 1);

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

    // Don't attempt the trace until there's a buffer

    if (spectrum.data == NULL)
	return TRUE;

    // Select font

    // cairo_select_font_face(cr, "sans-serif",
    // 			   CAIRO_FONT_SLANT_NORMAL,
    // 			   CAIRO_FONT_WEIGHT_NORMAL);
    // cairo_set_font_size(cr, FONT_HEIGHT);

    // Green pen for spectrum trace

    cairo_set_source_rgb(cr, 0, 1, 0);

    static float max;

    if (max < 1.0)
	max = 1.0;

    // Calculate the scaling

    float yscale = (float)height / max;

    max = 0.0;

    // Draw the spectrum

    cairo_move_to(cr, 0, 0);

    if (spectrum.zoom)
    {
	// Calculate scale

	float xscale = ((float)width / (spectrum.r - spectrum.l)) / 2.0;

	for (int i = floor(spectrum.l); i <= ceil(spectrum.h); i++)
	{
	    if (i > 0 && i < spectrum.length)
	    {
		float value = spectrum.data[i];

		if (max < value)
		    max = value;

		int y = -round(value * yscale);
		int x = round(((float)i - spectrum.l) * xscale); 

		cairo_line_to(cr, x, y);
	    }
	}

	cairo_move_to(cr, width / 2, 0);
	cairo_line_to(cr, width / 2, -height);

	cairo_stroke(cr);

	// Yellow pen for frequency trace

	cairo_set_source_rgb(cr, 1, 1, 0);

	// Draw lines for each frequency

	for (int i = 0; i < spectrum.count; i++)
	{
	    // Draw line for each that are in range

	    if (spectrum.values[i].f > spectrum.l &&
		spectrum.values[i].f < spectrum.h)
	    {
		int x = round((spectrum.values[i].f - spectrum.l) * xscale);
		cairo_move_to(cr, x, 0);
		cairo_line_to(cr, x, -height);

		double f = display.maxima[i].f;

		// Reference freq

		double fr = display.maxima[i].fr;
		double c = -12.0 * log2(fr / f);

		// Ignore silly values

		if (!isfinite(c))
		    continue;

		sprintf(s, "%+0.0f", c * 100.0);
		cairo_move_to(cr, x, -2);
		cairo_centre_text(cr, s);
	    }
	}
    }

    else
    {
	float xscale = ((float)spectrum.length /
			(float)spectrum.expand) / (float)width;

	for (int x = 0; x < width; x++)
	{
	    float value = 0.0;

	    // Don't show DC component

	    if (x > 0)
	    {
		for (int j = 0; j < xscale; j++)
		{
		    int n = x * xscale + j;

		    if (value < spectrum.data[n])
			value = spectrum.data[n];
		}
	    }

	    if (max < value)
		max = value;

	    int y = -round(value * yscale);

	    cairo_line_to(cr, x, y);
	}

	cairo_stroke(cr);

	// Yellow pen for frequency trace

	cairo_set_source_rgb(cr, 1, 1, 0);

	// Draw lines for each frequency

	for (int i = 0; i < spectrum.count; i++)
	{
	    // Draw line for each

	    int x = round(spectrum.values[i].f / xscale);
	    cairo_move_to(cr, x, 0);
	    cairo_line_to(cr, x, -height);

	    double f = display.maxima[i].f;

	    // Reference freq

	    double fr = display.maxima[i].fr;
	    double c = -12.0 * log2(fr / f);

	    // Ignore silly values

	    if (!isfinite(c))
		continue;

	    sprintf(s, "%+0.0f", c * 100.0);
	    cairo_move_to(cr, x, -2);
	    cairo_centre_text(cr, s);
	}

	if (spectrum.expand > 1)
	{
	    sprintf(s, "x%d", spectrum.expand);
	    cairo_move_to(cr, 0, -2);
	    cairo_show_text(cr, s);
	}
    }

    cairo_stroke(cr);

    // Show D for downsample

    if (audio.downsample)
    {
	cairo_move_to(cr, 0, 10 - height);
	cairo_show_text(cr, "D");
    }

    cairo_destroy(cr);

    return TRUE;
}

// Display draw callback

gboolean display_draw_callback(GtkWidget *widget, GdkEventExpose *event,
			       void *data)
{
    static FT_Library library;
    static FT_Face face;
    static cairo_font_face_t *musica;
    static char s[16];

    enum
    {FONT_HEIGHT   = 12,
     MUSIC_HEIGHT  = 26,
     LARGE_HEIGHT  = 36,
     LARGER_HEIGHT = 48,
     MEDIUM_HEIGHT = 24};

    static char *notes[] =
	{"C", "C", "D", "E", "E", "F",
	 "F", "G", "A", "A", "B", "B"};

    static char *sharps[] =
	{"", "#", "", "b", "", "",
	 "#", "", "b", "", "b", ""};

    if (library == NULL)
    {
	int err;

	err = FT_Init_FreeType(&library);

	// err = FT_New_Face(library, "/usr/share/fonts/truetype/musica.ttf",
	// 		  0, &face);
	err = FT_New_Face(library, "Musica.ttf", 0, &face);

	musica = cairo_ft_font_face_create_for_ft_face(face, 0);
    }

    cairo_t *cr = gdk_cairo_create(event->window);

    cairo_edge(cr,  widget->allocation.width, widget->allocation.height);

    int width = widget->allocation.width - 4;
    int height = widget->allocation.height - 4;

    cairo_set_source_rgb(cr, 0, 0, 0);

    if (display.multiple)
    {
	// Select font

	cairo_select_font_face(cr, "sans-serif",
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, FONT_HEIGHT);

	// Set text align

	if (display.count == 0)
	{
	    // Display note

	    sprintf(s, "%s%s%d", notes[display.n % Length(notes)],
		    sharps[display.n % Length(notes)], display.n / 12);
	    cairo_move_to(cr, 8, FONT_HEIGHT);
	    cairo_show_text(cr, s);
	    // TextOut(hbdc, 8, 0, s, strlen(s));

	    // Display cents

	    sprintf(s, "%+4.2lf¢", display.c * 100.0);
	    cairo_move_to(cr, 36, FONT_HEIGHT);
	    cairo_show_text(cr, s);
	    // TextOut(hbdc, 36, 0, s, strlen(s));

	    // Display reference

	    sprintf(s, "%4.2lfHz", display.fr);
	    cairo_move_to(cr, 90, FONT_HEIGHT);
	    cairo_show_text(cr, s);
	    // TextOut(hbdc, 90, 0, s, strlen(s));

	    // Display frequency

	    sprintf(s, "%4.2lfHz", display.f);
	    cairo_move_to(cr, 162, FONT_HEIGHT);
	    cairo_show_text(cr, s);
	    // TextOut(hbdc, 162, 0, s, strlen(s));

	    // Display difference

	    sprintf(s, "%+4.2lfHz", display.f - display.fr);
	    cairo_move_to(cr, 234, FONT_HEIGHT);
	    cairo_show_text(cr, s);
	    // TextOut(hbdc, 234, 0, s, strlen(s));
	}

	for (int i = 0; i < display.count; i++)
	{
	    double f = display.maxima[i].f;

	    // Reference freq

	    double fr = display.maxima[i].fr;

	    int n = display.maxima[i].n;

	    if (n < 0)
		n = 0;

	    double c = -12.0 * log2(fr / f);

	    // Ignore silly values

	    if (!isfinite(c))
		continue;

	    // Display note

	    sprintf(s, "%s%s%d", notes[n % Length(notes)],
		    sharps[n % Length(notes)], n / 12);
	    cairo_move_to(cr, 8, FONT_HEIGHT + (FONT_HEIGHT * i));
	    cairo_show_text(cr, s);
	    // TextOut(hbdc, 8, i * FONT_HEIGHT, s, strlen(s));

	    // Display cents

	    sprintf(s, "%+4.2lf¢", c * 100.0);
	    cairo_move_to(cr, 36, FONT_HEIGHT + (FONT_HEIGHT * i));
	    cairo_show_text(cr, s);
	    // TextOut(hbdc, 36, i * FONT_HEIGHT, s, strlen(s));

	    // Display reference

	    sprintf(s, "%4.2lfHz", fr);
	    cairo_move_to(cr, 90, FONT_HEIGHT + (FONT_HEIGHT * i));
	    cairo_show_text(cr, s);
	    // TextOut(hbdc, 90, i * FONT_HEIGHT, s, strlen(s));

	    // Display frequency

	    sprintf(s, "%4.2lfHz", f);
	    cairo_move_to(cr, 162, FONT_HEIGHT + (FONT_HEIGHT * i));
	    cairo_show_text(cr, s);
	    // TextOut(hbdc, 162, i * FONT_HEIGHT, s, strlen(s));

	    // Display difference

	    sprintf(s, "%+4.2lfHz", f - fr);
	    cairo_move_to(cr, 234, FONT_HEIGHT + (FONT_HEIGHT * i));
	    cairo_show_text(cr, s);
	    // TextOut(hbdc, 234, i * FONT_HEIGHT, s, strlen(s));

	    if (i == 5)
		break;
	}
    }

    else
    {
	double x, y;
	cairo_matrix_t matrix;

	// Select larger font

	cairo_select_font_face(cr, "sans-serif",
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_BOLD);
	cairo_matrix_init_scale(&matrix, 0.8, 1.0);
	cairo_set_font_matrix(cr, &matrix);
	cairo_set_font_size(cr, LARGER_HEIGHT);

	// Display note

	sprintf(s, "%s", notes[display.n % Length(notes)]);
	cairo_move_to(cr, 8, LARGE_HEIGHT);
	cairo_show_text(cr, s);
	// TextOut(hbdc, 8, y, s, strlen(s));

	cairo_get_current_point(cr, &x, &y);

	// Select medium font

	cairo_set_font_size(cr, MEDIUM_HEIGHT);
	// SelectObject(hbdc, medium);

	sprintf(s, "%d", display.n / 12);
	cairo_show_text(cr, s);
	// TextOut(hbdc, x, y, s, strlen(s));

	// Select musica font

	cairo_save(cr);
	cairo_set_font_face(cr, musica);
	cairo_set_font_size(cr, MUSIC_HEIGHT);
	// SelectObject(hbdc, musica);

	sprintf(s, "%s", sharps[display.n % Length(sharps)]);
	cairo_move_to(cr, x, MEDIUM_HEIGHT);
	cairo_show_text(cr, s);
	// TextOut(hbdc, x + 8, y - 20, s, strlen(s));

	// Select large font

	cairo_restore(cr);
	cairo_set_font_size(cr, LARGE_HEIGHT);
	// SelectObject(hbdc, large);
	// SetTextAlign(hbdc, TA_BASELINE|TA_RIGHT);

	// Display cents

	sprintf(s, "%+4.2f¢", display.c * 100.0);
	cairo_move_to(cr, width - 8, y);
	cairo_right_justify_text(cr, s);
	// TextOut(hbdc, width - 8, y, s, strlen(s));

	y += MEDIUM_HEIGHT;

	// Select medium font

	cairo_select_font_face(cr, "sans-serif",
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, MEDIUM_HEIGHT);
	// SelectObject(hbdc, medium);
	// SetTextAlign(hbdc, TA_BASELINE|TA_LEFT);

	// Display reference frequency

	sprintf(s, "%4.2fHz", display.fr);
	cairo_move_to(cr, 8, y);
	cairo_show_text(cr, s);
	// TextOut(hbdc, 8, y, s, strlen(s));

	// Display actual frequency

	// SetTextAlign(hbdc, TA_BASELINE|TA_RIGHT);
	sprintf(s, "%4.2fHz", display.f);
	cairo_move_to(cr, width - 8, y);
	cairo_right_justify_text(cr, s);
	// TextOut(hbdc, width - 8, y, s, strlen(s));

	y += 24;

	// Display reference

	// SetTextAlign(hbdc, TA_BASELINE|TA_LEFT);
	sprintf(s, "%4.2fHz", audio.reference);
	cairo_move_to(cr, 8, y);
	cairo_show_text(cr, s);
	// TextOut(hbdc, 8, y, s, strlen(s));

	// Display frequency difference

	// SetTextAlign(hbdc, TA_BASELINE|TA_RIGHT);
	sprintf(s, "%+4.2lfHz", display.f - display.fr);
	cairo_move_to(cr, width - 8, y);
	cairo_right_justify_text(cr, s);
	// TextOut(hbdc, width - 8, y, s, strlen(s));
    }

    // Show lock

    if (display.lock)
	// DrawLock(hbdc, -1, height + 1);

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

// Key press

gboolean key_press(GtkWidget *window, GdkEventKey *event, void *data)
{
    switch (event->keyval)
    {
    case GDK_KEY_c:
    case GDK_KEY_C:
	break;

    case GDK_KEY_d:
    case GDK_KEY_D:
	audio.downsample = !audio.downsample;
	break;

    case GDK_KEY_f:
    case GDK_KEY_F:
	audio.filter = !audio.filter;
	break;

    case GDK_KEY_l:
    case GDK_KEY_L:
	display.lock = !display.lock;
	break;

    case GDK_KEY_m:
    case GDK_KEY_M:
	display.multiple = !display.multiple;
	break;

    case GDK_KEY_s:
    case GDK_KEY_S:
	strobe.enable = !strobe.enable;
	break;

    case GDK_KEY_z:
    case GDK_KEY_Z:
	spectrum.zoom = !spectrum.zoom;
	break;

    case GDK_KEY_plus:
    case GDK_KEY_equal:
    case GDK_KEY_KP_Add:
	if (spectrum.expand < 16)
	    spectrum.expand *= 2;
	break;

    case GDK_KEY_minus:
    case GDK_KEY_underscore:
    case GDK_KEY_KP_Subtract:
	if (spectrum.expand > 1)
	    spectrum.expand /= 2;
	break;

    default:
	printf("Key code = %x\n", event->keyval);
	return TRUE;
	return FALSE;
    }

    return TRUE;
}

// Button press

gboolean button_press(GtkWidget *window, GdkEventButton *event, void *data)
{
    GtkWidget *menu;
    GtkWidget *item;

    if (event->button != 3)
	return FALSE;

    // Menu

    menu = gtk_menu_new();

    // Downsample

    item = gtk_check_menu_item_new_with_label("Downsample audio");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), audio.downsample);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Filter

    item = gtk_check_menu_item_new_with_label("Filter audio");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), audio.filter);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Lock

    item = gtk_check_menu_item_new_with_label("Display lock");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), display.lock);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Multiple

    item = gtk_check_menu_item_new_with_label("Multiple notes");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), display.multiple);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Strobe

    item = gtk_check_menu_item_new_with_label("Show strobe");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), strobe.enable);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Zoom

    item = gtk_check_menu_item_new_with_label("Zoom spectrum");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), spectrum.zoom);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Separator

    item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Options

    item = gtk_menu_item_new_with_label("Options...");
    g_signal_connect(G_OBJECT(item), "activate",
		     G_CALLBACK(options_clicked), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Separator

    item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    // Quit

    item = gtk_menu_item_new_with_label("Quit");
    g_signal_connect(G_OBJECT(item), "activate",
		     G_CALLBACK(gtk_main_quit), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
		   event->button, event->time);

    return TRUE;
}

// Options callback

void options_clicked(GtkWidget *widget, GtkWindow *window)
{
    static GtkWidget *dialog;

    GtkWidget *hbox;
    GtkWidget *h1box;
    GtkWidget *vbox;
    GtkWidget *close;
    GtkWidget *label;
    GtkWidget *save;
    GtkWidget *separator;

    if (dialog != NULL)
    {
	gtk_widget_show_all(dialog);
	return;
    }
	
    // Create options dialog

    dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(dialog), "Tuner Options");
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), window);

    // V box

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(dialog), vbox);

    // H box

    hbox = gtk_hbox_new(FALSE, MARGIN);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, MARGIN);

    // Label

    label = gtk_label_new("Sample rate: 11025.0");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, MARGIN);

    // Close button

    close = gtk_button_new_with_label("  Close  ");
    gtk_box_pack_end(GTK_BOX(hbox), close, FALSE, FALSE, MARGIN);

    // Close clicked

    g_signal_connect_swapped(G_OBJECT(close), "clicked",
			     G_CALLBACK(gtk_widget_hide), dialog);
    // H box

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // Label

    label = gtk_label_new("Correction:");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, MARGIN);

    // Correction

    options.correction = gtk_spin_button_new_with_range(0.9999, 1.0001,
							0.000001);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(options.correction),
			      audio.correction);
    gtk_box_pack_start(GTK_BOX(hbox), options.correction, FALSE, FALSE, 0);

    // Save button

    save = gtk_button_new_with_label("  Save  ");
    gtk_box_pack_end(GTK_BOX(hbox), save, FALSE, FALSE, MARGIN);

    // Save clicked

    g_signal_connect(G_OBJECT(save), "clicked",
		     G_CALLBACK(save_clicked), window);
    // H box

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, MARGIN);

    // Label

    label = gtk_label_new("Use correction if your sound card clock is\n"
			  "significantly inaccurate");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, MARGIN);

    // Separator

    separator = gtk_hseparator_new();
    gtk_box_pack_end(GTK_BOX(vbox), separator, FALSE, FALSE, 0);

    // H box

    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // V box

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, MARGIN);

    // Filter

    options.filter = gtk_check_button_new_with_label("Filter audio");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.filter),
				 audio.filter);
    gtk_box_pack_start(GTK_BOX(vbox), options.filter, FALSE, FALSE, 0);

    // Lock

    options.lock = gtk_check_button_new_with_label("Lock display");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.lock),
				 display.lock);
    gtk_box_pack_start(GTK_BOX(vbox), options.lock, FALSE, FALSE, 0);

    // Strobe

    options.strobe = gtk_check_button_new_with_label("Show strobe");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.strobe),
				 strobe.enable);
    gtk_box_pack_start(GTK_BOX(vbox), options.strobe, FALSE, FALSE, 0);

    // H1 box

    h1box = gtk_hbox_new(FALSE, MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), h1box, FALSE, FALSE, 0);

    // Label

    label = gtk_label_new("Reference:");
    gtk_box_pack_start(GTK_BOX(h1box), label, FALSE, FALSE, 0);

    // Reference

    options.reference = gtk_spin_button_new_with_range(430.0, 450.0, 0.1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(options.reference),
			      audio.reference);
    gtk_box_pack_start(GTK_BOX(h1box), options.reference, FALSE, FALSE, 0);

    // V box

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, MARGIN);

    // Downsample

    options.downsample = gtk_check_button_new_with_label("Downsample audio");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.downsample),
				 audio.downsample);
    gtk_box_pack_start(GTK_BOX(vbox), options.downsample, FALSE, FALSE, 0);

    // Multiple

    options.multiple = gtk_check_button_new_with_label("Multiple notes");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.multiple),
				 display.multiple);
    gtk_box_pack_start(GTK_BOX(vbox), options.multiple, FALSE, FALSE, 0);

    // Zoom

    options.zoom = gtk_check_button_new_with_label("Zoom spectrum");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(options.zoom),
				 spectrum.zoom);
    gtk_box_pack_start(GTK_BOX(vbox), options.zoom, FALSE, FALSE, 0);

    // Destroy dialog callback

    g_signal_connect(G_OBJECT(dialog), "destroy",
		     G_CALLBACK(gtk_widget_destroyed), &dialog);

    gtk_widget_show_all(dialog);
}

// Save callback

void save_clicked(GtkWidget *widget, GtkWindow *window)
{
}
