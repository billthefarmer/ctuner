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

    // Start audio thread

    pthread_create(&audio.thread, NULL, initAudio, NULL);

    // Interact with user

    gtk_main();
    gdk_threads_leave();

    // Exit

    return 0;
}

// init audio

void *initAudio(void *data)
{
    // gdk_threads_enter();
    // gdk_threads_leave();
}

// Scope draw callback

gboolean scope_draw_callback(GtkWidget *widget, GdkEventExpose *event,
			void *data)
{
  cairo_t *cr = gdk_cairo_create(event->window);

  cairo_set_source_rgb(cr, 1, 0, 0);
  cairo_paint(cr);
  cairo_destroy(cr);

  return TRUE;
}

// Spectrum draw callback

gboolean spectrum_draw_callback(GtkWidget *widget, GdkEventExpose *event,
			   void *data)
{
  cairo_t *cr = gdk_cairo_create(event->window);

  cairo_set_source_rgb(cr, 0, 1, 0);
  cairo_paint(cr);
  cairo_destroy(cr);

  return TRUE;
}

// Display draw callback

gboolean display_draw_callback(GtkWidget *widget, GdkEventExpose *event,
			   void *data)
{
  cairo_t *cr = gdk_cairo_create(event->window);

  cairo_set_source_rgb(cr, 1, 1, 0);
  cairo_paint(cr);
  cairo_destroy(cr);

  return TRUE;
}

// Strobe draw callback

gboolean strobe_draw_callback(GtkWidget *widget, GdkEventExpose *event,
			 void *data)
{
  cairo_t *cr = gdk_cairo_create(event->window);

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
