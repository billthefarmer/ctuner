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

// Wave in values

enum
    {SAMPLE_RATE = 11025,
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
    {kA5Reference = 440,
     kA5Offset    = 60};

// Slider values

enum
    {kVolumeMax  = 100,
     kVolumeMin  = 0,
     kVolumeStep = 10,

     kMeterMax   = 200,
     kMeterValue = 100,
     kMeterMin   = 0};

// Arrows values

enum
    {kReferenceMax   = 4500,
     kReferenceValue = 4400,
     kReferenceMin   = 4300,
     kReferenceStep  = 10,

     kCorrectionMax    = 110000,
     kCorrectionValue  = 100000,
     kCorrectionMin    = 99000,
     kCorrectionStep   = 1};

// Timer values

enum
    {STROBE_DELAY = 100};

// Command IDs

enum
    {kCommandVolume     = 'volm',
     kCommandCorrection = 'crtn',
     kCommandReference  = 'rfnc',
     kCommandStrobe     = 'strb',
     kCommandResize     = 'resz',
     kCommandMultiple   = 'mult',
     kCommandZoom       = 'zoom',
     kCommandLock       = 'lock',
     kCommandFilter     = 'fltr'};

// HIView IDs

HIViewID kHIViewIDText =
    {'text', 101};

// Global data

typedef struct
{
    double r;
    double i;
} complex;

typedef struct
{
    HIViewRef view;
    float *data;
    int length;
} Scope;

Scope scope;

typedef struct
{
    HIViewRef view;
    int length;
    bool zoom;
    float r;
    float x;
    double *data;
} Spectrum;

Spectrum spectrum;

typedef struct
{
    HIViewRef view;
    double f;
    double fr;
    double c;
    bool lock;
    bool resize;
    bool multiple;
    int n;
} Display;

Display display;

typedef struct
{
    HIViewRef view;
    float c;
    bool enable;
} Strobe;

Strobe strobe;

typedef struct
{
    struct
    {
	HIViewRef sample;
	HIViewRef correction;
    } status;

    struct
    {
	HIViewRef sample;
	HIViewRef reference;
	HIViewRef correction;
    } preferences;
} Legend;

Legend legend;

typedef struct
{
    HIViewRef zoom;
    HIViewRef lock;
    HIViewRef strobe;
    HIViewRef filter;
    HIViewRef resize;
    HIViewRef multiple;
} Check;

Check check;

typedef struct
{
    HIViewRef reference;
    HIViewRef correction;
} Arrow;

Arrow arrow;

typedef struct
{
    bool filter;
    MPTaskID task;
    MPQueueID queue;
    double correction;
    double reference;
} Audio;

Audio audio;

// Function prototypes.

OSStatus scopeDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus spectrumDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus displayDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus strobeDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus meterDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus WindowEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus CommandEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus FocusEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus TextEventHandler(EventHandlerCallRef, EventRef, void *);

OSStatus AudioProc(void *);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);

OSStatus DisplayPreferences(EventRef, void *);
OSStatus ChangeVolume(EventRef, HICommandExtended, UInt32);
OSStatus ChangeReference(EventRef, HICommandExtended, UInt32);
OSStatus ChangeCorrection(EventRef, HICommandExtended, UInt32);

void ReferenceActionProc(HIViewRef, ControlPartCode);
void CorrectionActionProc(HIViewRef, ControlPartCode);

void GetPreferences(void);
void fftr(complex[], int);

// Function main

int main(int argc, char *argv[])
{
    WindowRef window;
    HIViewRef content;
    HIViewRef slider;
    HIViewRef button;
    HIViewRef group;
    HIViewRef meter;

    MenuRef menu;

    GetPreferences();

    // Window bounds

    Rect bounds = {0, 0, 394, 320};

    // Create window

    OSStatus status =
	CreateNewWindow(kDocumentWindowClass,
			kWindowStandardFloatingAttributes |
			kWindowStandardHandlerAttribute |
			kWindowCompositingAttribute,
			&bounds, &window);

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

    AppendMenuItemTextWithCFString(menu, CFSTR("About Tuner"),
                                   0, kHICommandAbout, NULL);
								   
    // Create a separator

    AppendMenuItemTextWithCFString(menu, NULL,
				   kMenuItemAttrSeparator, 0, NULL);

    // Create an preferences item

    AppendMenuItemTextWithCFString(menu, CFSTR("Preferences..."),
                                   0, kHICommandPreferences, NULL);
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
    bounds.right  = 16;

    // Create slider

    CreateSliderControl(window, &bounds, kVolumeMax, kVolumeMin, kVolumeMax,
                        kControlSliderPointsDownOrRight, 0, false,
			NULL, &slider);

    // Control size

    ControlSize small = kControlSizeSmall;

    // Set control size

    SetControlData(slider, kControlEntireControl, kControlSizeTag,
		   sizeof(ControlSize), &small);

    // Set command ID

    HIViewSetCommandID(slider, kCommandVolume);

    // Place in the window

    HIViewAddSubview(content, slider);
    HIViewPlaceInSuperviewAt(slider, 20, 20);

    // Bounds of scope

    bounds.bottom = 32;
    bounds.right  = 256;

    // Create scope pane

    CreateUserPaneControl(window, &bounds, 0, &scope.view);

    // Set command ID

    HIViewSetCommandID(scope.view, kCommandFilter);

    // Place in the window

    HIViewAddSubview(content, scope.view);
    HIViewPlaceInSuperviewAt(scope.view, 44, 20);

    // Bounds of spectrum

    bounds.bottom = 32;
    bounds.right  = 256;

    // Create spectrum pane

    CreateUserPaneControl(window, &bounds, 0, &spectrum.view);

    // Set command ID

    HIViewSetCommandID(spectrum.view, kCommandZoom);

    // Place in the window

    HIViewAddSubview(content, spectrum.view);
    HIViewPlaceInSuperviewAt(spectrum.view, 44, 60);

    // Bounds of display

    bounds.bottom = 102;
    bounds.right  = 280;

    // Create display pane

    CreateUserPaneControl(window, &bounds, 0, &display.view);

    // Place in the window

    HIViewAddSubview(content, display.view);
    HIViewPlaceInSuperviewAt(display.view, 20, 100);

    // Bounds of strobe

    bounds.bottom = 44;
    bounds.right  = 280;

    // Create strobe pane

    CreateUserPaneControl(window, &bounds, 0, &strobe.view);

    // Set command ID

    HIViewSetCommandID(strobe.view, kCommandStrobe);

    // Place in the window

    HIViewAddSubview(content, strobe.view);
    HIViewPlaceInSuperviewAt(strobe.view, 20, 210);
    strobe.enable = true;

    // Bounds of meter

    bounds.bottom = 52;
    bounds.right  = 280;

    // Create meter pane

    CreateUserPaneControl(window, &bounds, 0, &meter);

    // Place in the window

    HIViewAddSubview(content, meter);
    HIViewPlaceInSuperviewAt(meter, 20, 262);

    // Bounds of slider

    bounds.bottom = 20;
    bounds.right  = 264;

    // Create meter slider

    CreateSliderControl(window, &bounds, kMeterValue, kMeterMin, kMeterMax,
                        kControlSliderPointsUpOrLeft, 0, false,
			NULL, &slider);

    // Set control size

    SetControlData(slider, kControlEntireControl, kControlSizeTag,
		   sizeof(ControlSize), &small);

    // Place in the pane

    HIViewAddSubview(meter, slider);
    HIViewPlaceInSuperviewAt(slider, 28, 284);

    // Bounds of preferences button

    bounds.bottom = 20;
    bounds.right  = 106;

    // Create push button

    CreatePushButtonControl(window, &bounds, CFSTR("Preferences..."), &button);

    // Set command ID

    HIViewSetCommandID(button, kHICommandPreferences); 

    // Place in the window

    HIViewAddSubview(content, button);
    HIViewPlaceInSuperviewAt(button, 20, 334);

    // Bounds of quit button

    bounds.bottom = 20;
    bounds.right  = 106;

    // Create push button

    CreatePushButtonControl(window, &bounds, CFSTR("Quit"), &button);

    // Set command ID

    HIViewSetCommandID(button, kHICommandQuit); 

    // Place in the window

    HIViewAddSubview(content, button);
    HIViewPlaceInSuperviewAt(button, 194, 334);

    // Group box bounds, wider than the window to hide rounded corners

    bounds.bottom = 20;
    bounds.right = 328;

    // Create group box for fake status bar

    CreateGroupBoxControl(window, &bounds, NULL, false, &group);

    // Place in window at negative offset to hide rounded corners

    HIViewAddSubview(content, group);
    HIViewPlaceInSuperviewAt(group, -4, 374);

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
                            &style, &legend.status.sample);

    // Place in group box

    HIViewAddSubview(group, legend.status.sample);
    HIViewPlaceInSuperviewAt(legend.status.sample, 24, 2);

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
                            &style, &legend.status.correction);

    // Place in group box

    HIViewAddSubview(group, legend.status.correction);
    HIViewPlaceInSuperviewAt(legend.status.correction, 164, 2);

    // Window events type spec

    EventTypeSpec windowEvents[] =
        {{kEventClassWindow, kEventWindowClose}};

    // Install event handler

    InstallWindowEventHandler(window, NewEventHandlerUPP(WindowEventHandler),
                              LENGTH(windowEvents), windowEvents,
                              NULL, NULL);

    // Command events type spec

    EventTypeSpec commandEvents[] =
        {{kEventClassCommand, kEventCommandProcess}};

    // Install event handler

    InstallApplicationEventHandler(NewEventHandlerUPP(CommandEventHandler),
                                   LENGTH(commandEvents), commandEvents,
                                   window, NULL);
    // Text events type spec

    EventTypeSpec textEvents[] =
        {{kEventClassTextInput, kEventTextInputUnicodeForKeyEvent}};

    // Install event handler

    InstallApplicationEventHandler(NewEventHandlerUPP(TextEventHandler),
                                   LENGTH(textEvents), textEvents,
                                   window, NULL);

    // Draw events type spec

    EventTypeSpec drawEvents[] =
	{kEventClassControl, kEventControlDraw};

    // Install event handlers

    InstallControlEventHandler(scope.view,
			       NewEventHandlerUPP(scopeDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       scope.view, NULL);

    InstallControlEventHandler(spectrum.view,
			       NewEventHandlerUPP(spectrumDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       spectrum.view, NULL);

    InstallControlEventHandler(display.view,
			       NewEventHandlerUPP(displayDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       display.view, NULL);

    InstallControlEventHandler(strobe.view,
			       NewEventHandlerUPP(strobeDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       strobe.view, NULL);

    InstallControlEventHandler(meter,
			       NewEventHandlerUPP(meterDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       meter, NULL);
    // Set up audio
#ifdef AUDIO
    AudioProc(NULL);
#endif
    // Run the application event loop

    RunApplicationEventLoop();

    return 0;
}

// Get preferences

void GetPreferences()
{
    Boolean found;
    Boolean flag;

    flag = CFPreferencesGetAppBooleanValue(CFSTR("Zoom"),
					   kCFPreferencesCurrentApplication,
					   &found);
    if (found)
	spectrum.zoom = flag;

    flag = CFPreferencesGetAppBooleanValue(CFSTR("Strobe"),
					   kCFPreferencesCurrentApplication,
					   &found);
    if (found)
	strobe.enable = flag;

    flag = CFPreferencesGetAppBooleanValue(CFSTR("Filter"),
					   kCFPreferencesCurrentApplication,
					   &found);
    if (found)
	audio.filter = flag;

    CFIndex value;

    value = CFPreferencesGetAppIntegerValue(CFSTR("Correction"),
					   kCFPreferencesCurrentApplication,
					   &found);
    if (found)
	audio.correction = (double)value / 100000.0;

    value = CFPreferencesGetAppIntegerValue(CFSTR("Reference"),
					   kCFPreferencesCurrentApplication,
					   &found);
    if (found)
	audio.reference = (double)value / 10.0;
;

}
// Audio proc

OSStatus AudioProc(void *data)
{
    OSStatus status;

    Component cp;
    ComponentDescription dc;
    ComponentInstance output;
    ComponentInstance convert;

    dc.componentManufacturer = kAudioUnitManufacturer_Apple;

    dc.componentType = kAudioUnitType_Output;
    dc.componentSubType = kAudioUnitSubType_HALOutput;

    dc.componentFlags = 0;
    dc.componentFlagsMask = 0;

    cp = FindNextComponent(NULL, &dc);

    if (cp == NULL)
    {
	StandardAlert(kAlertCautionAlert, "\pFindNextComponent",
		      "\perror", NULL, NULL);
    }

    OpenAComponent(cp, &output);

    UInt32 enable;
    UInt32 size;

    enable = true;
    AudioUnitSetProperty(output, kAudioOutputUnitProperty_EnableIO,
			 kAudioUnitScope_Input,
			 1, &enable, sizeof(enable));

    enable = false;
    AudioUnitSetProperty(output, kAudioOutputUnitProperty_EnableIO,
			 kAudioUnitScope_Output,
			 0, &enable, sizeof(enable));

    AudioDeviceID id;
    size = sizeof(id);

    AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,
			     &size, &id);

    AudioUnitSetProperty(output, kAudioOutputUnitProperty_CurrentDevice, 
                         kAudioUnitScope_Global, 0, &id, sizeof(id));

    dc.componentType = kAudioUnitType_FormatConverter;
    dc.componentSubType = kAudioUnitSubType_AUConverter;

    cp = FindNextComponent(NULL, &dc);

    if (cp == NULL)
    {
	StandardAlert(kAlertCautionAlert, "\pFindNextComponent",
		      "\perror", NULL, NULL);
    }

    OpenAComponent(cp, &convert);

    AudioUnitConnection ac =
	{output, 1, 0};

    status = AudioUnitSetProperty(convert, kAudioUnitProperty_MakeConnection, 
                         kAudioUnitScope_Input, 0, &ac, sizeof(ac));

    if (status != noErr)
    {
	StandardAlert(kAlertCautionAlert, "\pAudioUnitSetProperty",
		      "\perror", NULL, NULL);
    }

    Float64 rate;
    size = sizeof(rate);

    status = AudioDeviceGetProperty(id, 0, true,
				    kAudioDevicePropertyNominalSampleRate,
				    &size, &rate);
    if (status != noErr)
    {
	StandardAlert(kAlertCautionAlert, "\pAudioDeviceGetProperty",
		      "\perror", NULL, NULL);
    }

    rate = 11025.0;

    status = AudioDeviceSetProperty(id, NULL, 0, true,
				    kAudioDevicePropertyNominalSampleRate,
				    sizeof(rate), &rate);
    if (status != noErr)
    {
	StandardAlert(kAlertCautionAlert, "\pAudioDeviceSetProperty",
		      "\perror", NULL, NULL);
    }

    AudioStreamBasicDescription DeviceFormat;
    size = sizeof(AudioStreamBasicDescription);

    status = AudioUnitGetProperty(convert, kAudioUnitProperty_StreamFormat,
				  kAudioUnitScope_Input, 0,
				  &DeviceFormat, &size);
    if (status != noErr)
    {
	StandardAlert(kAlertCautionAlert, "\pAudioUnitGetProperty",
		      "\perror", NULL, NULL);
    }

    DeviceFormat.mSampleRate = 11025.0;
    DeviceFormat.mBytesPerPacket = 4;
    DeviceFormat.mBytesPerFrame = 4;
    DeviceFormat.mChannelsPerFrame = 1;

    status = AudioUnitSetProperty(convert, kAudioUnitProperty_StreamFormat,
				  kAudioUnitScope_Output, 0, &DeviceFormat,
				  sizeof(AudioStreamBasicDescription));
    if (status != noErr)
    {
	StandardAlert(kAlertCautionAlert, "\pAudioUnitSetProperty",
		      "\perror", NULL, NULL);
    }

    AURenderCallbackStruct input =
		{InputProc, &convert};

    status = AudioUnitSetProperty(output,
				  kAudioOutputUnitProperty_SetInputCallback,
				  kAudioUnitScope_Global, 0, &input,
				  sizeof(AURenderCallbackStruct));

    if (status != noErr)
    {
	StandardAlert(kAlertCautionAlert, "\pAudioUnitSetProperty",
		      "\perror", NULL, NULL);
    }

    status = AudioUnitInitialize(convert);

    if (status != noErr)
    {
	StandardAlert(kAlertCautionAlert, "\pAudioUnitInitialize",
		      "\perror", NULL, NULL);
    }

    status = AudioUnitInitialize(output);

    if (status != noErr)
    {
	StandardAlert(kAlertCautionAlert, "\pAudioUnitInitialize",
		      "\perror", NULL, NULL);
    }

    status = AudioOutputUnitStart(convert);

    if (status != noErr)
    {
	StandardAlert(kAlertCautionAlert, "\pAudioOutputUnitStart",
		      "\perror", NULL, NULL);
    }

    status = AudioOutputUnitStart(output);

    if (status != noErr)
    {
	StandardAlert(kAlertCautionAlert, "\pAudioOutputUnitStart",
		      "\perror", NULL, NULL);
    }
}

// Input proc

OSStatus InputProc(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
		   const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
		   UInt32 inNumberFrames, AudioBufferList *ioData)
{
    static long n;
    static float buffer[1024];
    static AudioBufferList abl =
	{1, {1, sizeof(buffer), &buffer}};

    OSStatus status = noErr;

    status = AudioUnitRender(*(AudioUnit *)inRefCon, ioActionFlags,
			     inTimeStamp, inBusNumber,
			     inNumberFrames, &abl);
    if (status != noErr)
	return status;

    scope.data = abl.mBuffers[0].mData;
    scope.length = abl.mBuffers[0].mDataByteSize / sizeof(float);

    if ((n++ % 8) == 0)
    {
	static char s[32];

	HIViewSetNeedsDisplay(scope.view, true);

	sprintf(s, "Input: %d", n);
	HIViewSetText(legend.status.correction,
		      CFStringCreateWithCString(kCFAllocatorDefault,
						s, kCFStringEncodingMacRoman));
    }
}

HIRect DrawEdge(CGContextRef context, HIRect bounds)
{
    int width = bounds.size.width;
    int height = bounds.size.height;

    CGContextSetShouldAntialias(context, false);
    CGContextSetLineWidth(context, 1);

    CGContextBeginPath(context);

    CGContextMoveToPoint(context, 1, height);
    CGContextAddLineToPoint(context, 1, 2);
    CGContextAddLineToPoint(context, width, 2);

    CGContextSetGrayStrokeColor(context, 0.4, 1);
    CGContextStrokePath(context);

    CGContextBeginPath(context);

    CGContextMoveToPoint(context, 1, height - 1);
    CGContextAddLineToPoint(context, width - 1, height - 1);
    CGContextAddLineToPoint(context, width - 1, 2);

    CGContextSetGrayStrokeColor(context, 0.9, 1);
    CGContextStrokePath(context);

    CGContextBeginPath(context);

    CGContextMoveToPoint(context, 0, height);
    CGContextAddLineToPoint(context, 0, 1);
    CGContextAddLineToPoint(context, width, 1);

    CGContextSetGrayStrokeColor(context, 0.6, 1);
    CGContextStrokePath(context);

    CGContextBeginPath(context);

    CGContextMoveToPoint(context, 0, height);
    CGContextAddLineToPoint(context, width, height);
    CGContextAddLineToPoint(context, width, 1);

    CGContextSetGrayStrokeColor(context, 1, 1);
    CGContextStrokePath(context);

    return CGRectInset(bounds, 2, 2);
}

OSStatus scopeDrawEventHandler(EventHandlerCallRef next,
			       EventRef event, void *data)
{
    CGContextRef context;
    HIRect bounds, inset;

    GetEventParameter(event, kEventParamCGContextRef,
		      typeCGContextRef, NULL,
		      sizeof(CGContextRef), NULL,
		      &context);

    HIViewGetBounds((HIViewRef)data, &bounds);

    inset = DrawEdge(context, bounds);

    float width = inset.size.width;
    float height = inset.size.height;

    CGContextSetShouldAntialias(context, false);
    CGContextSetLineWidth(context, 1);

    CGContextSetGrayFillColor(context, 0, 1);
    CGContextFillRect(context, inset);

    CGContextTranslateCTM(context, 2, 3);

    CGContextSetRGBStrokeColor(context, 0, 0.6, 0, 1);

    CGContextBeginPath(context);

    for (int i = 0; i <= width; i += 6)
    {
	CGContextMoveToPoint(context, i, 0);
	CGContextAddLineToPoint(context, i, height - 1);
    }

    for (int i = 5; i <= height; i += 6)
    {
	CGContextMoveToPoint(context, 0, i);
	CGContextAddLineToPoint(context, width, i);
    }

    CGContextStrokePath(context);

    if (scope.data == NULL)
	return noErr;

    // Initialise sync

    float maxdx = 0;
    float dx = 0;
    int n = 0;

    for (int i = 1; i < width; i++)
    {
	dx = scope.data[i] - scope.data[i - 1];
	if (maxdx > dx)
	{
	    maxdx = dx;
	    n = i;
	}

	if (maxdx < 0.0 && dx > 0.0)
	    break;
    }

    // Green stroke for scope trace

    CGContextSetRGBStrokeColor(context, 0, 1, 0, 1);

    // Move the origin

    CGContextTranslateCTM(context, 0, height / 2);

    static float max;

    if (max < 4096.0)
	max = 4096.0;

    float yscale = max / (height / 2);

    max = 0.0;

    // Draw the trace

    CGContextBeginPath(context);
    CGContextMoveToPoint(context, 0, 0);

    for (int i = 0; i < width; i++)
    {
	if (max < abs(scope.data[n + i]))
	    max = abs(scope.data[n + i]);

	float y = scope.data[n + i] / yscale;
	CGContextAddLineToPoint(context, i, y);
    }

    CGContextStrokePath(context);
}

OSStatus spectrumDrawEventHandler(EventHandlerCallRef next,
				  EventRef event, void *data)
{
    CGContextRef context;
    HIRect bounds, inset;

    GetEventParameter(event, kEventParamCGContextRef,
		      typeCGContextRef, NULL,
		      sizeof(CGContextRef), NULL,
		      &context);

    HIViewGetBounds((HIViewRef)data, &bounds);

    inset = DrawEdge(context, bounds);

    int width = inset.size.width;
    int height = inset.size.height;

    CGContextSetShouldAntialias(context, false);
    CGContextSetLineWidth(context, 1);

    CGContextSetGrayFillColor(context, 0, 1);
    CGContextFillRect(context, inset);

    CGContextTranslateCTM(context, 2, 3);

    CGContextSetRGBStrokeColor(context, 0, 0.6, 0, 1);

    CGContextBeginPath(context);

    for (int i = 0; i <= width; i += 6)
    {
	CGContextMoveToPoint(context, i, 0);
	CGContextAddLineToPoint(context, i, height - 1);
    }

    for (int i = 5; i <= height; i += 6)
    {
	CGContextMoveToPoint(context, 0, i);
	CGContextAddLineToPoint(context, width, i);
    }

    CGContextStrokePath(context);

    return noErr;
}

OSStatus displayDrawEventHandler(EventHandlerCallRef next,
				 EventRef event, void *data)
{
    enum
    {kTextSizeLarge  = 36,
     kTextSizeMedium = 24};

    static char *notes[] =
	{"A", "Bb", "B", "C", "C#", "D",
	 "Eb", "E", "F", "F#", "G", "Ab"};

    CGContextRef context;
    HIRect bounds, inset;

    GetEventParameter(event, kEventParamCGContextRef,
		      typeCGContextRef, NULL,
		      sizeof(CGContextRef), NULL,
		      &context);

    HIViewGetBounds((HIViewRef)data, &bounds);

    inset = DrawEdge(context, bounds);

    int width = inset.size.width;
    int height = inset.size.height;

    CGContextTranslateCTM(context, 2, 4);
    CGContextSetLineWidth(context, 1);

    CGContextSetGrayStrokeColor(context, 0, 1);
    CGContextSetGrayFillColor(context, 0, 1);

    CGContextSetShouldAntialias(context, true);

    // Select font

    CGContextSelectFont(context, "Arial Bold", kTextSizeLarge,
			kCGEncodingMacRoman);
    CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1, -1));
    CGContextSetTextDrawingMode(context, kCGTextFill);

    static char s[64];
    int y = kTextSizeLarge;

    sprintf(s, "%4s%d  ", notes[display.n % LENGTH(notes)], display.n / 12); 
    CGContextShowTextAtPoint(context, 8, y, s, strlen(s));

    sprintf(s, "%+6.2lf¢  ", display.c * 100.0);
    CGContextShowTextAtPoint(context, width / 2, y, s, strlen(s));

    y += kTextSizeMedium;
    CGContextSetFontSize(context, kTextSizeMedium);

    sprintf(s, "%9.2lfHz  ", display.fr);
    CGContextShowTextAtPoint(context, 8, y, s, strlen(s));

    sprintf(s, "%9.2lfHz  ", display.f);
    CGContextShowTextAtPoint(context, width / 2, y, s, strlen(s));

    y += kTextSizeMedium;

    sprintf(s, "%9.2lfHz  ", (audio.reference == 0)?
	    kA5Reference: audio.reference);
    CGContextShowTextAtPoint(context, 8, y, s, strlen(s));

    sprintf(s, "%+8.2lfHz  ", display.fr - display.f);
    CGContextShowTextAtPoint(context, width / 2, y, s, strlen(s));

    return noErr;
}

OSStatus strobeDrawEventHandler(EventHandlerCallRef next,
				EventRef event, void *data)
{
    CGContextRef context;
    HIRect bounds, inset;

    GetEventParameter(event, kEventParamCGContextRef,
		      typeCGContextRef, NULL,
		      sizeof(CGContextRef), NULL,
		      &context);

    HIViewGetBounds((HIViewRef)data, &bounds);

    inset = DrawEdge(context, bounds);

    static float mc = 0.0;
    static float mx = 0.0;

    int width = inset.size.width;
    int height = inset.size.height;

    CGContextClipToRect(context, inset);
    CGContextTranslateCTM(context, 2, 2);
    CGContextSetGrayFillColor(context, 0, 1);

    if (strobe.enable)
    {
	mc = ((7.0 * mc) + strobe.c) / 8.0;
	mx += mc * 50.0;

	if (mx > 160.0)
	    mx = 0.0;

	if (mx < 0.0)
	    mx = 160.0;

	int rx = round(mx - 160.0);

	for (int x = rx % 20; x < width; x += 20)
	  CGContextFillRect(context, CGRectMake(x, 0, 10, 10));

	for (int x = rx % 40; x < width; x += 40)
	  CGContextFillRect(context, CGRectMake(x, 10, 20, 10));

	for (int x = rx % 80; x < width; x += 80)
	  CGContextFillRect(context, CGRectMake(x, 20, 40, 10));

	for (int x = rx % 160; x < width; x += 160)
	  CGContextFillRect(context, CGRectMake(x, 30, 80, 10));
    }

    return noErr;
}

OSStatus meterDrawEventHandler(EventHandlerCallRef next,
			       EventRef event, void *data)
{
    CGContextRef context;
    HIRect bounds, inset;

    GetEventParameter(event, kEventParamCGContextRef,
		      typeCGContextRef, NULL,
		      sizeof(CGContextRef), NULL,
		      &context);

    HIViewGetBounds((HIViewRef)data, &bounds);

    inset = DrawEdge(context, bounds);

    int width = inset.size.width;
    int height = inset.size.height;

    CGContextTranslateCTM(context, 2, 3);
    CGContextSetLineWidth(context, 1);

    CGContextSetGrayStrokeColor(context, 0, 1);
    CGContextSetGrayFillColor(context, 0, 1);

    CGContextTranslateCTM(context, width / 2, 0);
    CGContextSetShouldAntialias(context, true);

    CGContextSelectFont(context, "Arial", 14, kCGEncodingMacRoman);
    CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1, -1));
    CGContextSetTextDrawingMode(context, kCGTextFill);

    // Draw the meter scale

    for (int i = 0; i < 6; i++)
    {
	if (i == 0)
	    CGContextShowTextAtPoint(context, -3, 14, "0", 1);

	else
	{
	    int x = width / 11 * i;
	    static char s[16];

	    sprintf(s, "%d", i * 10);

	    CGContextShowTextAtPoint(context, x - 6, 14, s, strlen(s));
	    CGContextShowTextAtPoint(context, -x - 6, 14, s, strlen(s));
	}
    }

    CGContextSetShouldAntialias(context, false);
    CGContextBeginPath(context);

    for (int i = 0; i < 6; i++)
    {
	int x = width / 11 * i;

	CGContextMoveToPoint(context, x, 18);
	CGContextAddLineToPoint(context, x, 24);

	CGContextMoveToPoint(context, -x, 18);
	CGContextAddLineToPoint(context, -x, 24);

	for (int j = 1; j < 5; j++)
	{
	    if (i < 5)
	    {
		CGContextMoveToPoint(context, x + j * width / 55, 20);
		CGContextAddLineToPoint(context, x + j * width / 55, 24);
	    }

	    CGContextMoveToPoint(context, -x + j * width / 55, 20);
	    CGContextAddLineToPoint(context, -x + j * width / 55, 24);
	}
    }

    CGContextStrokePath(context);

    return noErr;
}

// Window event handler

OSStatus WindowEventHandler(EventHandlerCallRef next,
                       EventRef event, void *data)
{
    WindowRef window;
    UInt32 kind;

    // Get the event kind

    kind = GetEventKind(event);

    // Get the window

    GetEventParameter(event, kEventParamDirectObject,
                      typeWindowRef, NULL, sizeof(window),
                      NULL, &window);

    // Switch on event kind

    switch (kind)
    {
        // Window close event

    case kEventWindowClose:

	// Flush preferences

	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);

        // Quit the application

        QuitApplicationEventLoop();
        break;

    default:
        return eventNotHandledErr;
    }

    // Return ok

    return noErr;
}

OSStatus CommandEventHandler(EventHandlerCallRef next, EventRef event,
			     void *data)
{
    HICommandExtended command;
    CFIndex number;
    UInt32 value;

    // Get the command

    GetEventParameter(event, kEventParamDirectObject,
                      typeHICommand, NULL, sizeof(command),
                      NULL, &command);

    // Get the value

    value = HIViewGetValue(command.source.control);

    // Switch on the command ID

    switch (command.commandID)
    {
	// Volume

    case kCommandVolume:
	ChangeVolume(event, command, value);
	break;

	// Zoom

    case kCommandZoom:
	spectrum.zoom = value;

	CFPreferencesSetAppValue(CFSTR("Zoom"),
				 value? kCFBooleanTrue: kCFBooleanFalse,
				 kCFPreferencesCurrentApplication);
	break;

	// Strobe

    case kCommandStrobe:
	strobe.enable = value;

	CFPreferencesSetAppValue(CFSTR("Strobe"),
				 value? kCFBooleanTrue: kCFBooleanFalse,
				 kCFPreferencesCurrentApplication);
	break;

	// Filter

    case kCommandFilter:
	audio.filter = value;

	CFPreferencesSetAppValue(CFSTR("Filter"),
				 value? kCFBooleanTrue: kCFBooleanFalse,
				 kCFPreferencesCurrentApplication);
	break;

	// Lock

    case kCommandLock:
	display.lock = value;
	break;

	// Multiple

    case kCommandMultiple:
	display.multiple = value;
	break;

	// Reference

    case kCommandReference:
	ChangeReference(event, command, value);
	break;

	// Correction

    case kCommandCorrection:
	ChangeCorrection(event, command, value);
	break;

        // Preferences button

    case kHICommandPreferences:
	DisplayPreferences(event, data);
	break;

	// Save

    case kHICommandSave:
	number = round(audio.correction * 10000.0);
	CFPreferencesSetAppValue(CFSTR("Correction"),
				 CFNumberCreate(kCFAllocatorDefault,
						kCFNumberCFIndexType,
						&number),
				 kCFPreferencesCurrentApplication);
	break;

	// Close

    case kHICommandClose:

        // Quit

    case kHICommandQuit:

	// Flush preferences

	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);

        // Let the default handler handle it

    default:
        return eventNotHandledErr;
	}
}

// Change volume

OSStatus ChangeVolume(EventRef event,
		      HICommandExtended command,
		      UInt32 value)
{
    return eventNotHandledErr;
}

OSStatus DisplayPreferences(EventRef event, void *data)
{
    static WindowRef window;

    HIViewRef content;
    HIViewRef button;
    HIViewRef slider;
    HIViewRef group;
    HIViewRef text;

    WindowRef parent = (WindowRef)data;

    // Check window

    if (window != NULL)
    {
	if (IsWindowVisible(window))
	{
	    SelectWindow(window);
	    return noErr;
	}
    }

    // Window bounds

    Rect bounds = {0, 0, 304, 320};

    // Create window

    OSStatus status =
	CreateNewWindow(kDocumentWindowClass,
			kWindowStandardFloatingAttributes |
			kWindowStandardHandlerAttribute |
			kWindowCompositingAttribute,
			&bounds, &window);

    // Set the title

    SetWindowTitleWithCFString(window, CFSTR("Tuner Preferences"));

    // Show and position the window

    ShowWindow(window);
    RepositionWindow(window, parent, kWindowAlertPositionOnParentWindow);

    // Find the window content

    HIViewFindByID(HIViewGetRoot(window),
                   kHIViewWindowContentID,
                   &content);

    // Group box bounds

    bounds.bottom = 122;
    bounds.right = 280;

    // Create group box

    CreateGroupBoxControl(window, &bounds, NULL, false, &group);

    // Place in window

    HIViewAddSubview(content, group);
    HIViewPlaceInSuperviewAt(group, 20, 20);

    // Bounds of check box

    bounds.bottom = 18;
    bounds.right  = 124;

    // Create  check box

    CreateCheckBoxControl(window, &bounds, CFSTR("Zoom spectrum"),
			  spectrum.zoom, true, &check.zoom);

    // Set command ID

    HIViewSetCommandID(check.zoom, kCommandZoom);

    // Place in group box

    HIViewAddSubview(group, check.zoom);
    HIViewPlaceInSuperviewAt(check.zoom, 16, 16);

    // Create  check box

    CreateCheckBoxControl(window, &bounds, CFSTR("Display strobe"),
			  strobe.enable, true, &check.strobe);

    // Set command ID

    HIViewSetCommandID(check.strobe, kCommandStrobe);

    // Place in group box

    HIViewAddSubview(group, check.strobe);
    HIViewPlaceInSuperviewAt(check.strobe, 140, 16);

    // Create  check box

    CreateCheckBoxControl(window, &bounds, CFSTR("Audio filter"),
			  audio.filter, true, &check.filter);

    // Set command ID

    HIViewSetCommandID(check.filter, kCommandFilter);

    // Place in group box

    HIViewAddSubview(group, check.filter);
    HIViewPlaceInSuperviewAt(check.filter, 16, 40);

    // Create  check box

    CreateCheckBoxControl(window, &bounds, CFSTR("Lock display"),
			  display.lock, true, &check.lock);

    // Set command ID

    HIViewSetCommandID(check.lock, kCommandLock);

    // Place in group box

    HIViewAddSubview(group, check.lock);
    HIViewPlaceInSuperviewAt(check.lock, 140, 40);

    // Create  check box

    CreateCheckBoxControl(window, &bounds, CFSTR("Resize display"),
			  spectrum.zoom, true, &check.resize);

    // Set command ID

    HIViewSetCommandID(check.resize, kCommandResize);

    // Place in group box

    HIViewAddSubview(group, check.resize);
    HIViewPlaceInSuperviewAt(check.resize, 16, 64);

    // Create  check box

    CreateCheckBoxControl(window, &bounds, CFSTR("Multiple notes"),
			  display.multiple, true, &check.multiple);

    // Set command ID

    HIViewSetCommandID(check.multiple, kCommandMultiple);

    // Place in group box

    HIViewAddSubview(group, check.multiple);
    HIViewPlaceInSuperviewAt(check.multiple, 140, 64);

    // Text bounds

    bounds.bottom = 16;
    bounds.right  = 140;

    // Create static text

    CreateStaticTextControl(window, &bounds,
			    CFSTR("Reference:"),
                            NULL, &text);

    // Place in group box

    HIViewAddSubview(group, text);
    HIViewPlaceInSuperviewAt(text, 16, 90);

    // Edit bounds

    bounds.bottom = 16;
    bounds.right  = 56;

    // Create edit control

    CreateEditUnicodeTextControl(window, &bounds,
				 CFSTR("440.00"),
				 false, NULL, &legend.preferences.reference);
    // Focus event type spec

    EventTypeSpec focusEvents[] =
	{kEventClassControl, kEventControlSetFocusPart};

    // Install event handlers

    InstallControlEventHandler(legend.preferences.reference,
			       NewEventHandlerUPP(FocusEventHandler),
			       LENGTH(focusEvents), focusEvents,
			       scope.view, NULL);
    // Set control data

    Boolean single = true;

    SetControlData(legend.preferences.reference,
		   kControlEditTextPart,
		   kControlEditTextSingleLineTag,
		   sizeof(single), &single);

    // Set command ID

    HIViewSetCommandID(legend.preferences.reference, kCommandReference);

    // Place in group box

    HIViewAddSubview(group, legend.preferences.reference);
    HIViewPlaceInSuperviewAt(legend.preferences.reference, 98, 90);

    // Arrows bounds

    bounds.bottom = 22;
    bounds.right  = 13;

    CreateLittleArrowsControl(window, &bounds,
			      kReferenceValue, kReferenceMin,
			      kReferenceMax, kReferenceStep,
			      &arrow.reference);
    //Set action proc

    SetControlAction(arrow.reference, ReferenceActionProc);

    // Place in group box

    HIViewAddSubview(group, arrow.reference);
    HIViewPlaceInSuperviewAt(arrow.reference, 166, 88);

    // Group box bounds

    bounds.bottom = 116;
    bounds.right = 280;

    // Create group box

    CreateGroupBoxControl(window, &bounds, NULL, false, &group);

    // Place in window

    HIViewAddSubview(content, group);
    HIViewPlaceInSuperviewAt(group, 20, 168);

    // Text bounds

    bounds.bottom = 16;
    bounds.right  = 72;

    // Create static text

    CreateStaticTextControl(window, &bounds,
			    CFSTR("Correction:"),
                            NULL, &text);

    // Place in group box

    HIViewAddSubview(group, text);
    HIViewPlaceInSuperviewAt(text, 16, 16);

    // Edit bounds

    bounds.bottom = 16;
    bounds.right  = 56;

    // Create edit control

    CreateEditUnicodeTextControl(window, &bounds,
				 CFSTR("1.00000"),
				 false, NULL,
				 &legend.preferences.correction);
    // Install event handlers

    InstallControlEventHandler(legend.preferences.correction,
			       NewEventHandlerUPP(FocusEventHandler),
			       LENGTH(focusEvents), focusEvents,
			       scope.view, NULL);
    // Set control data

    SetControlData(legend.preferences.correction,
		   kControlEditTextPart,
		   kControlEditTextSingleLineTag,
		   sizeof(single), &single);

    // Set command ID

    HIViewSetCommandID(legend.preferences.correction, kCommandCorrection);

    // Place in group box

    HIViewAddSubview(group, legend.preferences.correction);
    HIViewPlaceInSuperviewAt(legend.preferences.correction, 98, 16);

    // Arrows bounds

    bounds.bottom = 22;
    bounds.right  = 13;

    CreateLittleArrowsControl(window, &bounds,
			      kCorrectionValue, kCorrectionMin,
			      kCorrectionMax, kCorrectionStep,
			      &arrow.correction);

    //Set action proc

    SetControlAction(arrow.correction, CorrectionActionProc);

    // Place in group box

    HIViewAddSubview(group, arrow.correction);
    HIViewPlaceInSuperviewAt(arrow.correction, 166, 14);

    // Button bounds

    bounds.bottom = 20;
    bounds.right  = 72;

    // Create push button

    CreatePushButtonControl(window, &bounds, CFSTR("Save"), &button);

    // Set command ID

    HIViewSetCommandID(button, kHICommandSave); 

    // Place in group box

    HIViewAddSubview(group, button);
    HIViewPlaceInSuperviewAt(button, 192, 15);

    // Text bounds

    bounds.bottom = 32;
    bounds.right  = 248;

    // Create static text

    CreateStaticTextControl(window, &bounds,
			    CFSTR("Use correction if your sound card "
				  "is significantly inaccurate."),
                            NULL, &text);

    // Place in group box

    HIViewAddSubview(group, text);
    HIViewPlaceInSuperviewAt(text, 16, 44);

    // Text bounds

    bounds.bottom = 16;
    bounds.right  = 144;

    // Create static text

    CreateStaticTextControl(window, &bounds,
			    CFSTR("Sample rate: 11025.0"),
                            NULL, &legend.preferences.sample);

    // Place in group box

    HIViewAddSubview(group, legend.preferences.sample);
    HIViewPlaceInSuperviewAt(legend.preferences.sample, 16, 84);

    // Button bounds

    bounds.bottom = 20;
    bounds.right  = 72;

    // Create push button

    CreatePushButtonControl(window, &bounds, CFSTR("Close"), &button);

    // Set command ID

    HIViewSetCommandID(button, kHICommandClose); 

    // Place in group box

    HIViewAddSubview(group, button);
    HIViewPlaceInSuperviewAt(button, 192, 83);
}

// Focus event handler

OSStatus FocusEventHandler(EventHandlerCallRef next, EventRef event,
			   void *data)
{
    HIViewRef view;
    UInt32 id;

    // Get the view

    GetEventParameter(event, kEventParamDirectObject,
                      typeControlRef, NULL, sizeof(view),
                      NULL, &view);

    // Get command id

    HIViewGetCommandID(view, &id);

    // Copy text

    CFStringRef text = HIViewCopyText(view);

    // Get value

    double value = CFStringGetDoubleValue(text);

    CFRelease(text);

    switch (id)
    {
    case kCommandReference:
	audio.reference = value;
	HIViewSetValue(arrow.reference, value * 10);
	HIViewSetNeedsDisplay(display.view, true);
	break;

    case kCommandCorrection:
	audio.correction = value;
	HIViewSetValue(arrow.correction, value * 100000);
	break;
    }

    return eventNotHandledErr;
}

// Text event handler

OSStatus TextEventHandler(EventHandlerCallRef next, EventRef event,
			  void *data)
{
    return eventNotHandledErr;
}

// Change reference

OSStatus ChangeReference(EventRef event,
			  HICommandExtended command,
			  UInt32 value)
{
    CFStringRef text = HIViewCopyText(command.source.control);

    CFRelease(text);

    eventNotHandledErr;
}

// Reference action proc

void ReferenceActionProc(HIViewRef view, ControlPartCode part)
{
    SInt32 value = HIViewGetValue(view);
    SInt32 step = 0;

    GetControlData(view, 0, kControlLittleArrowsIncrementValueTag,
		   sizeof(step), &step, nil);

    switch (part)
    {
    case kControlUpButtonPart:
	value += step;
	break;

    case kControlDownButtonPart:
	value -= step;
	break;
    }

    HIViewSetValue(view, value);

    static char s[64];

    audio.reference = (double)value / 10.0;
    sprintf(s, "%6.2lf", audio.reference);
    HIViewSetText(legend.preferences.reference,
		  CFStringCreateWithCString(kCFAllocatorDefault, s,
					    kCFStringEncodingMacRoman));

    HIViewSetNeedsDisplay(display.view, true);

    CFPreferencesSetAppValue(CFSTR("Reference"),
			     CFNumberCreate(kCFAllocatorDefault,
					    kCFNumberCFIndexType,
					    &value),
			     kCFPreferencesCurrentApplication);
}

// Change correction

OSStatus ChangeCorrection(EventRef event,
			  HICommandExtended command,
			  UInt32 value)
{
    CFStringRef text = HIViewCopyText(command.source.control);

    CFRelease(text);

    eventNotHandledErr;
}

// Correction action proc

void CorrectionActionProc(HIViewRef view, ControlPartCode part)
{

    SInt32 value = HIViewGetValue(view);
    SInt32 step = 0;

    GetControlData(view, 0, kControlLittleArrowsIncrementValueTag,
		   sizeof(step), &step, nil);

    switch (part)
    {
    case kControlUpButtonPart:
	value += step;
	break;

    case kControlDownButtonPart:
	value -= step;
	break;
    }

    HIViewSetValue(view, value);

    static char s[64];

    audio.correction = (double)value / 100000.0;

    sprintf(s, "%6.5lf", audio.correction);
    HIViewSetText(legend.preferences.correction,
		  CFStringCreateWithCString(kCFAllocatorDefault, s,
					    kCFStringEncodingMacRoman));

    sprintf(s, "Correction: %6.5lf", audio.correction);
    HIViewSetText(legend.status.correction,
		  CFStringCreateWithCString(kCFAllocatorDefault, s,
					    kCFStringEncodingMacRoman));

    sprintf(s, "Sample rate: %6.1lf", 11025.0 / audio.correction);
    HIViewSetText(legend.preferences.sample,
		  CFStringCreateWithCString(kCFAllocatorDefault, s,
					    kCFStringEncodingMacRoman));

    HIViewSetText(legend.status.sample,
		  CFStringCreateWithCString(kCFAllocatorDefault, s,
					    kCFStringEncodingMacRoman));
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
