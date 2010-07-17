////////////////////////////////////////////////////////////////////////////////
//
//  Tuner - A Tuner written in C.
//
//  Copyright (C) 2010  Bill Farmer
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
//  Bill Farmer  william j farmer [at] tiscali [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

#include <Carbon/Carbon.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

// Macros

#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

#define OCTAVE 12
#define MIN   0.5

#define kCustomViewClassID CFSTR("com.wjf.tuner.view")

// Wave in values

enum
    {SAMPLE_RATE = 11025L,
     BITS_PER_SAMPLE = 16,
     BLOCK_ALIGN = 2,
     CHANNELS = 1};

// Audio processing values

enum
    {OVERSAMPLE = 8,
     SAMPLES = 8192,
     RANGE = SAMPLES / 4,
     STEP = SAMPLES / OVERSAMPLE};

// Tuner reference values

enum
    {A5_REFNCE = 440,
     A5_OFFSET = 60,
     E7_OFFSET = 91};

// Slider values

enum
    {MAX_VOL  = 100,
     MIN_VOL  = 0,
     STEP_VOL = 10,

     MAX_REF  = 4500,
     REF_REF  = 4400,
     MIN_REF  = 4300,
     STEP_REF = 10,

     MAX_METER = 200,
     REF_METER = 100,
     MIN_METER = 0};

// Timer values

enum
    {STROBE_DELAY = 100};

// Window size

enum
    {WIDTH  = 320,
     HEIGHT = 420};

// Command IDs

enum
    {kCommandVolume    = 'volm',
     kCommandMinus     = 'mnus',
     kCommandPlus      = 'plus',
     kCommandReference = 'rfnc',
     kCommandStrobe    = 'strb',
     kCommandOptions   = 'optn',
     kCommandZoom      = 'zoom',
     kCommandFilter    = 'fltr'};

// Global data

typedef struct
{
    double r;
    double i;
} complex;

// Function prototypes.

OSStatus DrawEventHandler(EventHandlerCallRef, EventRef, void *);

void fftr(complex[], int);

// Function main

int main(int argc, char *argv[])
{
    WindowRef window;
    HIViewRef content;
    HIViewRef slider;
    HIViewRef button;
    HIViewRef scope;
    HIViewRef spectrum;
    HIViewRef display;
    HIViewRef strobe;
    HIViewRef group;
    HIViewRef meter;
    HIViewRef text;

    MenuRef menu;
    int i;

    // GetStatus();

    // Window bounds

    Rect bounds = {0, 0, 420, 420};

    // Create window

    OSStatus status =
	CreateNewWindow(kDocumentWindowClass,
			kWindowStandardFloatingAttributes |
			kWindowStandardHandlerAttribute |
			kWindowInWindowMenuAttribute |
			kWindowCompositingAttribute,
			&bounds, &window);

    if (status != noErr)
    {
	StandardAlert(kAlertCautionAlert, "\pCreateNewWindow",
		      "\perror", NULL, NULL);
    }

    // Set the title

    SetWindowTitleWithCFString(window, CFSTR("Tuner"));

    // Create an application menu

    CreateNewMenu(0, 0, &menu);

    // Set menu title

    SetMenuTitleWithCFString(menu,
	CFStringCreateWithPascalString(kCFAllocatorDefault,
                                       "\p\024",
                                       kCFStringEncodingMacRoman));
    // Create an about item

    InsertMenuItemTextWithCFString(menu, CFSTR("About Tuner"),
                                   0, 0, kHICommandAbout);

    // Insert the menu

    InsertMenu(menu, 0);

    // Create a standard window menu

    CreateStandardWindowMenu(0, &menu);

    // Insert the menu

    InsertMenu(menu, 0);

    // Show and position the window

    ShowWindow(window);
    RepositionWindow(window, NULL, kWindowCascadeOnMainScreen);

    // Find the window content

    HIViewFindByID(HIViewGetRoot(window),
                   kHIViewWindowContentID,
                   &content);

    // Bounds of slider

    bounds.bottom = 72;
    bounds.right  = 18;

    // Create slider

    CreateSliderControl(window, &bounds, MAX_VOL, MIN_VOL, MAX_VOL,
                        kControlSliderPointsDownOrRight, 0, false,
			NULL, &slider);

    // Set command ID

    HIViewSetCommandID(slider, kCommandVolume);

    // Place in the window

    HIViewAddSubview(content, slider);
    HIViewPlaceInSuperviewAt(slider, 20, 20);

    // Bounds of scope

    bounds.bottom = 32;
    bounds.right  = 254;

    // Create scope view

    CreateUserPaneControl(window, &bounds, 0, &scope);

    // Place in the window

    HIViewAddSubview(content, scope);
    HIViewPlaceInSuperviewAt(scope, 46, 20);

    // Bounds of spectrum

    bounds.bottom = 32;
    bounds.right  = 254;

    // Create spectrum view

    CreateUserPaneControl(window, &bounds, 0, &spectrum);

    // Set command ID

    HIViewSetCommandID(spectrum, kCommandZoom);

    // Place in the window

    HIViewAddSubview(content, spectrum);
    HIViewPlaceInSuperviewAt(spectrum, 46, 60);

    // Bounds of display

    bounds.bottom = 102;
    bounds.right  = 280;

    // Create display view

    CreateUserPaneControl(window, &bounds, 0, &display);

    // Place in the window

    HIViewAddSubview(content, display);
    HIViewPlaceInSuperviewAt(display, 20, 100);

    // Bounds of button

    bounds.bottom = 16;
    bounds.right  = 16;

    // Create button

    CreateBevelButtonControl(window, &bounds, CFSTR("-"),
			     kControlBevelButtonNormalBevel,
			     kControlBehaviorPushbutton,
			     NULL, 0, 0, 0, &button);

    // Set command ID

    HIViewSetCommandID(slider, kCommandMinus);

    // Place in the window

    HIViewAddSubview(content, button);
    HIViewPlaceInSuperviewAt(button, 20, 212);

    // Bounds of slider

    bounds.bottom = 19;
    bounds.right  = 232;

    // Create reference slider

    CreateSliderControl(window, &bounds, REF_REF, MIN_REF, MAX_REF,
                        kControlSliderPointsUpOrLeft, 0, false,
			NULL, &slider);

    // Set command ID

    HIViewSetCommandID(slider, kCommandReference);

    // Place in the window

    HIViewAddSubview(content, slider);
    HIViewPlaceInSuperviewAt(slider, 44, 210);

    // Bounds of button

    bounds.bottom = 16;
    bounds.right  = 16;

    // Create button

    CreateBevelButtonControl(window, &bounds, CFSTR("+"),
			     kControlBevelButtonNormalBevel,
			     kControlBehaviorPushbutton,
			     NULL, 0, 0, 0, &button);

    // Set command ID

    HIViewSetCommandID(button, kCommandPlus);

    // Place in the window

    HIViewAddSubview(content, button);
    HIViewPlaceInSuperviewAt(button, 284, 212);

    // Bounds of strobe

    bounds.bottom = 44;
    bounds.right  = 280;

    // Create display view

    CreateUserPaneControl(window, &bounds, 0, &strobe);

    // Set command ID

    HIViewSetCommandID(strobe, kCommandZoom);

    // Place in the window

    HIViewAddSubview(content, strobe);
    HIViewPlaceInSuperviewAt(display, 20, 237);

    // Bounds of meter

    bounds.bottom = 52;
    bounds.right  = 280;

    // Create meter view

    CreateUserPaneControl(window, &bounds, 0, &meter);

    // Place in the window

    HIViewAddSubview(content, meter);
    HIViewPlaceInSuperviewAt(display, 20, 289);

    // Bounds of options button

    bounds.bottom = 20;
    bounds.right  = 72;

    // Create push button

    CreatePushButtonControl(window, &bounds, CFSTR("Options"), &button);

    // Set command ID

    HIViewSetCommandID(button, kHICommandPreferences); 

    // Place in the window

    HIViewAddSubview(content, button);
    HIViewPlaceInSuperviewAt(button, 20, 361);

    // Bounds of quit button

    bounds.bottom = 20;
    bounds.right  = 72;

    // Create push button

    CreatePushButtonControl(window, &bounds, CFSTR("Quit"), &button);

    // Set command ID

    HIViewSetCommandID(button, kHICommandQuit); 

    // Place in the window

    HIViewAddSubview(content, button);
    HIViewPlaceInSuperviewAt(button, 228, 361);

    // Group box bounds, wider than the window to hide rounded corners

    bounds.bottom = 20;
    bounds.right = 328;

    // Create group box for fake status bar

    CreateGroupBoxControl(window, &bounds, NULL, false, &group);

    // Place in window at negative offset to hide rounded corners

    HIViewAddSubview(content, group);
    HIViewPlaceInSuperviewAt(group, -4, 400);

    // Text bounds

    bounds.bottom = 16;
    bounds.right  = 140;

    // Font style

    ControlFontStyleRec style;
    style.flags = kControlUseFontMask | kControlUseJustMask;
    style.font = kControlFontSmallSystemFont;
    style.just = teFlushLeft;

    // Create static text

    CreateStaticTextControl(window, &bounds,
        CFSTR("Sample rate: 11025.0"),
                            &style, &text);

    // Place in group box

    HIViewAddSubview(group, text);
    HIViewPlaceInSuperviewAt(text, 24, 2);

    // Text bounds

    bounds.bottom = 16;
    bounds.right  = 140;

    // Font style

    style.flags = kControlUseFontMask | kControlUseJustMask;
    style.font = kControlFontSmallSystemFont;
    style.just = teFlushRight;

    // Create static text

    CreateStaticTextControl(window, &bounds,
        CFSTR("Correction: 1.00000"),
                            &style, &text);

    // Place in group box

    HIViewAddSubview(group, text);
    HIViewPlaceInSuperviewAt(text, 144, 2);

    EventTypeSpec drawEvents[] =
	{kEventClassControl, kEventControlDraw};

    InstallControlEventHandler(scope, NewEventHandlerUPP(DrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       scope, NULL);

    InstallControlEventHandler(spectrum, NewEventHandlerUPP(DrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       spectrum, NULL);

    // Run the application event loop

    RunApplicationEventLoop();

    // Stop the graph

    // AUGraphStop(graph);

    // Dispose of the graph

    // DisposeAUGraph(graph);

    // Exit

    return 0;
}

OSStatus DrawEventHandler(EventHandlerCallRef handler,
			  EventRef event, void *data)
{
    OSStatus status = noErr;
    CGContextRef context;
    HIRect bounds, inset;

    status = GetEventParameter(event, kEventParamCGContextRef,
			       typeCGContextRef, NULL,
			       sizeof(CGContextRef), NULL,
			       &context);
    require_noerr(status, CantGetGraphicsContext);

    HIViewGetBounds((HIViewRef)data, &bounds);
    require_noerr(status, CantGetBoundingRectangle);

    // ********** Your drawing code here **********

    CGContextSetRGBStrokeColor(context, 0, 0, 1, 1);
    CGContextSetLineWidth(context, 3);
    CGContextStrokeRect(context, bounds);

    inset = CGRectInset(bounds, 3, 3);

    CGContextSetRGBStrokeColor(context, 0, 0, 0, 1);
    CGContextSetLineWidth(context, 1);
    CGContextStrokeRect(context, inset);

CantGetGraphicsContext:
CantGetBoundingRectangle:

    return status;
}

OSStatus scopeDrawEventHandler(EventHandlerCallRef handler,
			       EventRef event, void *data)
{
    OSStatus status = noErr;
    CGContextRef context;
    HIRect bounds, inset;

    status = GetEventParameter(event, kEventParamCGContextRef,
			       typeCGContextRef, NULL,
			       sizeof(CGContextRef), NULL,
			       &context);
    require_noerr(status, CantGetGraphicsContext);

    HIViewGetBounds((HIViewRef)data, &bounds);
    require_noerr(status, CantGetBoundingRectangle);

    // ********** Your drawing code here **********

CantGetGraphicsContext:
CantGetBoundingRectangle:

    return status;
}

OSStatus spectrumDrawEventHandler(EventHandlerCallRef handler,
				  EventRef event, void *data)
{
    OSStatus status = noErr;
    CGContextRef context;
    HIRect bounds, inset;

    status = GetEventParameter(event, kEventParamCGContextRef,
			       typeCGContextRef, NULL,
			       sizeof(CGContextRef), NULL,
			       &context);
    require_noerr(status, CantGetGraphicsContext);

    HIViewGetBounds((HIViewRef)data, &bounds);
    require_noerr(status, CantGetBoundingRectangle);

    // ********** Your drawing code here **********

CantGetGraphicsContext:
CantGetBoundingRectangle:

    return status;
}

OSStatus displayDrawEventHandler(EventHandlerCallRef handler,
				 EventRef event, void *data)
{
    OSStatus status = noErr;
    CGContextRef context;
    HIRect bounds, inset;

    status = GetEventParameter(event, kEventParamCGContextRef,
			       typeCGContextRef, NULL,
			       sizeof(CGContextRef), NULL,
			       &context);
    require_noerr(status, CantGetGraphicsContext);

    HIViewGetBounds((HIViewRef)data, &bounds);
    require_noerr(status, CantGetBoundingRectangle);

    // ********** Your drawing code here **********

CantGetGraphicsContext:
CantGetBoundingRectangle:

    return status;
}

OSStatus strobeDrawEventHandler(EventHandlerCallRef handler,
				EventRef event, void *data)
{
    OSStatus status = noErr;
    CGContextRef context;
    HIRect bounds, inset;

    status = GetEventParameter(event, kEventParamCGContextRef,
			       typeCGContextRef, NULL,
			       sizeof(CGContextRef), NULL,
			       &context);
    require_noerr(status, CantGetGraphicsContext);

    HIViewGetBounds((HIViewRef)data, &bounds);
    require_noerr(status, CantGetBoundingRectangle);

    // ********** Your drawing code here **********

CantGetGraphicsContext:
CantGetBoundingRectangle:

    return status;
}

OSStatus meterDrawEventHandler(EventHandlerCallRef handler,
			       EventRef event, void *data)
{
    OSStatus status = noErr;
    CGContextRef context;
    HIRect bounds, inset;

    status = GetEventParameter(event, kEventParamCGContextRef,
			       typeCGContextRef, NULL,
			       sizeof(CGContextRef), NULL,
			       &context);
    require_noerr(status, CantGetGraphicsContext);

    HIViewGetBounds((HIViewRef)data, &bounds);
    require_noerr(status, CantGetBoundingRectangle);

    // ********** Your drawing code here **********

CantGetGraphicsContext:
CantGetBoundingRectangle:

    return status;
}

// FFT

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
