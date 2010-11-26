////////////////////////////////////////////////////////////////////////////////
//
//  SigGen - An audio signal generator written in C.
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
#include <CoreAudio/CoreAudio.h>
#include <Accelerate/Accelerate.h>

// Macros

#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

// Audio out values

enum
    {kSampleRate       = 44100,
     kSamples          = 4096,
     kMaxLevel         = 1,
     kBytesPerPacket   = 4,
     kBytesPerFrame    = 4,
     kChannelsPerFrame = 1};

// Frequency scale

enum
    {kFrequencyScale = 250,
     kFrequencyMax   = 850,
     kFrequencyMin   = 0};

// Fine slider

enum
    {kFineMax  = 100,
     kFineRef  = 50,
     kFineMin  = 0};

// Level slider

enum
    {kLevelMax  = 100,
     kLevelRef  = 20,
     kLevelMin  = 0};

// Waveform

enum
    {kSine,
     kSquare,
     kSawtooth};

// Command IDs

enum
    {kCommandFrequency = 'Freq',
     kCommandFine      = 'Fine',
     kCommandLevel     = 'Levl',
     kCommandSine      = 'Sine',
     kCommandSquare    = 'Squa',
     kCommandSawtooth  = 'Sawt',
     kCommandMute      = 'Mute'};

// Keycodes

enum
    {kKeyboardUpKey    = 0x7e,
     kKeyboardDownKey  = 0x7d,
     kKeyboardLeftKey  = 0x7b,
     kKeyboardRightKey = 0x7c};

// Global data

typedef struct
{
    HIViewRef view;
    float value;
} Scale;

Scale scale =
    {NULL, kFrequencyScale * 2.0};

typedef struct
{
    HIViewRef view;
    float frequency;
    float decibels;
} Display;

Display display =
    {NULL, 1000.0, -20.0};

typedef struct
{
    HIViewRef fine;
    HIViewRef level;
} Sliders;

Sliders sliders;

typedef struct
{
    HIViewRef sine;
    HIViewRef square;
    HIViewRef sawtooth;
} Buttons;

Buttons buttons;

typedef struct
{
    AudioUnit output;
    Boolean mute;
    Boolean flag;
    int waveform;
    float level;
    float rate;
} Audio;

Audio audio;
 
// Function prototypes.

OSStatus ScaleDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus DisplayDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus KnobDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus WindowEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus CommandEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus MouseEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus KeyboardEventHandler(EventHandlerCallRef, EventRef, void *);

OSStatus SetupAudio(void);
OSStatus DisplayAlert(CFStringRef, CFStringRef);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);

OSStatus ChangeFrequency(UInt32);
OSStatus ChangeLevel(UInt32);

OSStatus UpdateFrequency(void);

HIRect DrawEdge(CGContextRef, HIRect);

void FineActionProc(HIViewRef, ControlPartCode);
void LevelActionProc(HIViewRef, ControlPartCode);

// Function main

int main(int argc, char *argv[])
{
    WindowRef window;
    HIViewRef content;
    HIViewRef button;
    HIViewRef group;
    HIViewRef knob;

    MenuRef menu;

    // GetPreferences();

    // Window bounds

    Rect bounds = {0, 0, 248, 338};

    // Create window

    CreateNewWindow(kDocumentWindowClass,
		    kWindowStandardFloatingAttributes |
		    kWindowFrameworkScaledAttribute |
		    kWindowStandardHandlerAttribute |
		    kWindowCompositingAttribute,
		    &bounds, &window);

    // Set the title

    SetWindowTitleWithCFString(window, CFSTR("Audio Signal Generator"));

    // Create an application menu

    CreateNewMenu(0, 0, &menu);

    // Set menu title

    CFStringRef apple =
	CFStringCreateWithPascalString(kCFAllocatorDefault,
				       "\p\024",
				       kCFStringEncodingMacRoman);

    SetMenuTitleWithCFString(menu, apple);
    CFRelease(apple);

    // Create an about item

    AppendMenuItemTextWithCFString(menu, CFSTR("About Signal Generator"),
                                   0, kHICommandAbout, NULL);
    // Insert the menu

    InsertMenu(menu, 0);
    ReleaseMenu(menu);

    // Create a standard window menu

    CreateStandardWindowMenu(0, &menu);

    // Insert the menu

    InsertMenu(menu, 0);
    ReleaseMenu(menu);

    // Show and position the window

    ShowWindow(window);
    RepositionWindow(window, NULL, kWindowAlertPositionOnMainScreen);

    // Find the window content

    HIViewFindByID(HIViewGetRoot(window),
                   kHIViewWindowContentID,
                   &content);

    // Bounds of scale

    bounds.bottom = 40;
    bounds.right  = 160;

    // Create scale pane

    CreateUserPaneControl(window, &bounds, 0, &scale.view);

    // Set help tag

    HMHelpContentRec help =
	{kMacHelpVersion,
	 {0, 0, 0, 0},
	 kHMInsideLeftCenterAligned,
	 {{kHMCFStringContent,
	   CFSTR("Frequency scale")},
	  {kHMNoContent, NULL}}};

    HMSetControlHelpContent(scale.view, &help);

    // Place in the window

    HIViewAddSubview(content, scale.view);
    HIViewPlaceInSuperviewAt(scale.view, 20, 20);

    // Bounds of display

    bounds.bottom = 60;
    bounds.right  = 130;

    // Create display pane

    CreateUserPaneControl(window, &bounds, 0, &display.view);

    // Set help tag

    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Frequency display");
    HMSetControlHelpContent(display.view, &help);

    // Place in the window

    HIViewAddSubview(content, display.view);
    HIViewPlaceInSuperviewAt(display.view, 188, 20);

    // Bounds of knob

    bounds.bottom = 168;
    bounds.right  = 168;

    // Create display pane

    CreateUserPaneControl(window, &bounds, 0, &knob);

    // Set command ID

    HIViewSetCommandID(knob, kCommandFrequency);

    // Set help tag

    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Frequency adjust");
    HMSetControlHelpContent(knob, &help);

    // Place in the window

    HIViewAddSubview(content, knob);
    HIViewPlaceInSuperviewAt(knob, 16, 64);

    // Bounds of slider

    bounds.bottom = 140;
    bounds.right  = 15;

    // Create slider

    CreateSliderControl(window, &bounds, kFineRef, kFineMin, kFineMax,
                        kControlSliderPointsDownOrRight, 0, true,
			FineActionProc, &sliders.fine);
    // Control size

    ControlSize small = kControlSizeSmall;

    // Set control size

    SetControlData(sliders.fine, kControlEntireControl, kControlSizeTag,
		   sizeof(small), &small);

    // Set command ID

    HIViewSetCommandID(sliders.fine, kCommandFine);

    // Set help tag

    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Frequency adjust");
    HMSetControlHelpContent(sliders.fine, &help);

    // Place in the window

    HIViewAddSubview(content, sliders.fine);
    HIViewPlaceInSuperviewAt(sliders.fine, 188, 88);

    // Bounds of slider

    bounds.bottom = 140;
    bounds.right  = 14;

    // Create slider

    CreateSliderControl(window, &bounds, kLevelRef, kLevelMin, kLevelMax,
                        kControlSliderPointsUpOrLeft, 0, true,
			LevelActionProc, &sliders.level);
    // Set control size

    SetControlData(sliders.level, kControlEntireControl, kControlSizeTag,
		   sizeof(small), &small);

    // Set command ID

    HIViewSetCommandID(sliders.level, kCommandLevel);

    // Set help tag

    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Level adjust");
    HMSetControlHelpContent(sliders.level, &help);

    // Place in the window

    HIViewAddSubview(content, sliders.level);
    HIViewPlaceInSuperviewAt(sliders.level, 211, 88);

    // Bounds of group

    bounds.bottom = 160;
    bounds.right = 85;

    // Create group

    CreateRadioGroupControl(window, &bounds, &group);

    // Place in the window

    HIViewAddSubview(content, group);
    HIViewPlaceInSuperviewAt(group, 233, 88);

    // Bounds of button

    bounds.bottom = 20;
    bounds.right  = 85;

    // Create sine button

    CreateRadioButtonControl(window, &bounds, CFSTR("Sine"),
			     kControlRadioButtonCheckedValue,
			     false, &buttons.sine);
    // Set command ID

    HIViewSetCommandID(buttons.sine, kCommandSine); 

    // Place in the group

    HIViewAddSubview(group, buttons.sine);
    HIViewPlaceInSuperviewAt(buttons.sine, 0, 0);

    // Create square button

    CreateRadioButtonControl(window, &bounds, CFSTR("Square"),
			     kControlRadioButtonUncheckedValue,
			     false, &buttons.square);
    // Set command ID

    HIViewSetCommandID(buttons.square, kCommandSquare); 

    // Place in the group

    HIViewAddSubview(group, buttons.square);
    HIViewPlaceInSuperviewAt(buttons.square, 0, 26);

    // Create sawtooth button

    CreateRadioButtonControl(window, &bounds, CFSTR("Sawtooth"),
			     kControlRadioButtonUncheckedValue,
			     false, &buttons.sawtooth);
    // Set command ID

    HIViewSetCommandID(buttons.sawtooth, kCommandSawtooth); 

    // Place in the window

    HIViewAddSubview(group, buttons.sawtooth);
    HIViewPlaceInSuperviewAt(buttons.sawtooth, 0, 52);

    // Create mute button

    CreateCheckBoxControl(window, &bounds, CFSTR("Mute"),
			  kControlCheckBoxUncheckedValue,
			  true, &button);
    // Set command ID

    HIViewSetCommandID(button, kCommandMute); 

    // Place in the window

    HIViewAddSubview(content, button);
    HIViewPlaceInSuperviewAt(button, 233, 168);

    // Bounds of quit button

    bounds.bottom = 20;
    bounds.right  = 85;

    // Create push button

    CreatePushButtonControl(window, &bounds, CFSTR("Quit"), &button);

    // Set command ID

    HIViewSetCommandID(button, kHICommandQuit); 

    // Place in the window

    HIViewAddSubview(content, button);
    HIViewPlaceInSuperviewAt(button, 233, 208);

    // Draw events type spec

    EventTypeSpec drawEvents[] =
	{{kEventClassControl, kEventControlDraw}};

    // Install event handlers

    InstallControlEventHandler(scale.view,
			       NewEventHandlerUPP(ScaleDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       scale.view, NULL);

    InstallControlEventHandler(display.view,
			       NewEventHandlerUPP(DisplayDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       display.view, NULL);

    InstallControlEventHandler(knob,
			       NewEventHandlerUPP(KnobDrawEventHandler),
			       LENGTH(drawEvents), drawEvents,
			       knob, NULL);

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
    // Mouse events type spec

    EventTypeSpec mouseEvents[] =
        {{kEventClassMouse, kEventMouseDragged}};

    // Install event handler

    InstallWindowEventHandler(window, NewEventHandlerUPP(MouseEventHandler),
			      LENGTH(mouseEvents), mouseEvents,
			      window, NULL);

    // Keyboard events type spec

    EventTypeSpec keyboardEvents[] =
        {{kEventClassKeyboard, kEventRawKeyDown},
	 {kEventClassKeyboard, kEventRawKeyRepeat}};

    // Install event handler

    InstallApplicationEventHandler(NewEventHandlerUPP(KeyboardEventHandler),
                                   LENGTH(keyboardEvents), keyboardEvents,
                                   window, NULL);
    // Set up audio

    SetupAudio();

    // Run the application event loop

    RunApplicationEventLoop();

    return 0;
}

OSStatus SetupAudio()
{
    // Specify an output unit

    ComponentDescription dc =
	{kAudioUnitType_Output,
        kAudioUnitSubType_DefaultOutput,
        kAudioUnitManufacturer_Apple,
        0, 0};

    // Find an output unit

    Component cp
	= FindNextComponent(NULL, &dc);

    if (cp == NULL)
    {
	DisplayAlert(CFSTR("FindNextComponent"), 
		     CFSTR("Can't find an output audio unit"));
	return -1;
    }

    // Open it

    OSStatus status = OpenAComponent(cp, &audio.output);

    if (status != noErr)
    {
	DisplayAlert(CFSTR("OpenAComponent"), 
		     CFSTR("Can't open an output audio unit"));
	return status;
    }

    UInt32 frames = kSamples;
    UInt32 size = sizeof(frames);

    // Set the max frames

    status = AudioUnitSetProperty(audio.output,
				  kAudioUnitProperty_MaximumFramesPerSlice,
				  kAudioUnitScope_Global, 0,
				  &frames, sizeof(frames));
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitSetProperty"), 
		     CFSTR("Can't set output audio unit maximum frames"));
	return status;
    }

    // Set the buffer size

    status = AudioUnitSetProperty(audio.output,
				  kAudioDevicePropertyBufferFrameSize,
				  kAudioUnitScope_Global, 0,
				  &frames, sizeof(frames));
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitSetProperty"), 
		     CFSTR("Can't set output audio unit buffer size"));
	return status;
    }

    AudioStreamBasicDescription format;
    size = sizeof(format);

    // Get stream format

    status = AudioUnitGetProperty(audio.output,
				  kAudioUnitProperty_StreamFormat,
				  kAudioUnitScope_Input, 0,
				  &format, &size);
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitGetProperty"), 
		     CFSTR("Can't get output audio unit stream format"));
	return status;
    }

    format.mBytesPerPacket = kBytesPerPacket;
    format.mBytesPerFrame = kBytesPerFrame;
    format.mChannelsPerFrame = kChannelsPerFrame;

    // Set stream format

    status = AudioUnitSetProperty(audio.output,
				  kAudioUnitProperty_StreamFormat,
				  kAudioUnitScope_Input, 0,
				  &format, sizeof(format));
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitSetProperty"), 
		     CFSTR("Can't set output audio unit stream format"));
	return status;
    }

    // Save sample rate

    audio.rate = format.mSampleRate;

    AURenderCallbackStruct input =
	{InputProc, &audio.output};

    // Set callback

    status = AudioUnitSetProperty(audio.output,
    				  kAudioUnitProperty_SetRenderCallback,
    				  kAudioUnitScope_Input, 0,
    				  &input, sizeof(input));
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitSetProperty"), 
		     CFSTR("Can't set output audio unit input callback"));
	return status;
    }

    // Start the audio unit

    status = AudioUnitInitialize(audio.output);

    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitInitialize"), 
		     CFSTR("Can't initialise output audio unit"));
	return status;
    }

    AudioOutputUnitStart(audio.output);

    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioOutputUnitStart"), 
		     CFSTR("Can't start output audio unit"));
	return status;
    }

    return noErr;
}

// Input proc

OSStatus InputProc(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
		   const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
		   UInt32 inNumberFrames, AudioBufferList *ioData)
{
    static float K;
    static float q = 0.0;
    static float f;
    static float l;

    if (K == 0)
    {
	K = 2.0 * M_PI / audio.rate;
	f = display.frequency;
	audio.level = (float)kMaxLevel * 0.2;
    }

    float *buffer = ioData->mBuffers[0].mData;

    for (int i = 0; i < inNumberFrames; i++)
    {
	f += ((display.frequency - f) / (float)kSamples);
	l +=  audio.mute? -l / (float)kSamples: 
	    (audio.level - l) / (float)kSamples;

	q += (q < M_PI)? f * K: (f * K) - (2.0 * M_PI);

	switch (audio.waveform)
	{
	case kSine:
	    buffer[i] = sin(q) * l;
	    break;

	case kSquare:
	    buffer[i] = (q > 0.0)? l: -l;
	    break;

	case kSawtooth:
	    buffer[i] = (q / M_PI) * l;
	    break;
	}
    }

    return noErr;
}

// Display alert

OSStatus DisplayAlert(CFStringRef error, CFStringRef explanation)
{
    DialogRef dialog;

    CreateStandardAlert(kAlertStopAlert, error, explanation, NULL, &dialog);
    SetWindowTitleWithCFString(GetDialogWindow(dialog), CFSTR("Tuner"));
    RunStandardAlert(dialog, NULL, NULL);

    return noErr;
}

// Draw edge

HIRect DrawEdge(CGContextRef context, HIRect bounds)
{
    int width = bounds.size.width;
    int height = bounds.size.height;

    CGContextSetShouldAntialias(context, false);
    CGContextSetLineWidth(context, 1);

    // Draw edge

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

    // Create inset

    CGRect inset = CGRectInset(bounds, 2, 2);
    CGContextClipToRect(context, inset);

    return inset;
}

OSStatus ScaleDrawEventHandler(EventHandlerCallRef next,
			       EventRef event, void *data)
{
    CGContextRef context;
    HIRect bounds, inset;
    HIViewRef view;

    enum
    {kTextSize = 12};

    // Get context

    GetEventParameter(event, kEventParamCGContextRef,
		      typeCGContextRef, NULL,
		      sizeof(context), NULL,
		      &context);
    // Get view

    GetEventParameter(event, kEventParamDirectObject,
		      typeControlRef, NULL,
		      sizeof(view), NULL,
		      &view);
    // Get bounds

    HIViewGetBounds(view, &bounds);

    inset = DrawEdge(context, bounds);

    float width = inset.size.width;
    float height = inset.size.height;

    CGContextTranslateCTM(context, 2, 3);

    // Move the origin

    CGContextTranslateCTM(context,  width / 2, height / 2);

    // Draw scale

    CGContextBeginPath(context);
    CGContextSetGrayStrokeColor(context, 0, 1);
    CGContextSetShouldAntialias(context, false);
    CGContextSetLineWidth(context, 1);

    for (int i = 1; i < 11; i++)
    {
	float x = (kFrequencyScale * log10(i)) - scale.value;

	for (int j = 0; j < 4; j++)
	{
	    CGContextMoveToPoint(context, x, 0);
	    CGContextAddLineToPoint(context, x, height / 2);
	    x += kFrequencyScale;
	}
    }

    for (int i = 3; i < 20; i += 2)
    {
	float x = (kFrequencyScale * log10(i / 2.0)) - scale.value;

	for (int j = 0; j < 4; j++)
	{
	    CGContextMoveToPoint(context, x, 6);
	    CGContextAddLineToPoint(context, x, height / 2);
	    x += kFrequencyScale;
	}
    }

    CGContextMoveToPoint(context, 0, -height / 2);
    CGContextAddLineToPoint(context, 0, height / 2);

    CGContextStrokePath(context);

    // Select font

    CGContextSelectFont(context, "Arial Bold", kTextSize,
			kCGEncodingMacRoman);
    CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1, -1));
    CGContextSetTextDrawingMode(context, kCGTextFill);
    CGContextSetShouldAntialias(context, true);

    int a[] = {1, 2, 3, 4, 5, 6, 7, 8};
    for (int i = 0; i < LENGTH(a); i++)
    {
    	float x = (kFrequencyScale * log10(a[i])) - scale.value;

    	for (int j = 0; j < 2; j++)
    	{
	    static char s[8];

	    sprintf(s, "%d", a[i]);
	    CGContextShowTextAtPoint(context, x - 2, -6, s, strlen(s));

	    sprintf(s, "%d", a[i] * 10);
	    CGContextShowTextAtPoint(context, x + kFrequencyScale - 6,
				     -6, s, strlen(s));

    	    x += 2 * kFrequencyScale;
    	}
    }

    return noErr;
}

OSStatus DisplayDrawEventHandler(EventHandlerCallRef next,
				 EventRef event, void *data)
{
    CGContextRef context;
    HIRect bounds, inset;
    HIViewRef view;

    enum
    {kTextSize = 24};

    static char s[16];

    // Get context

    GetEventParameter(event, kEventParamCGContextRef,
		      typeCGContextRef, NULL,
		      sizeof(context), NULL,
		      &context);
    // Get view

    GetEventParameter(event, kEventParamDirectObject,
		      typeControlRef, NULL,
		      sizeof(view), NULL,
		      &view);
    // Get bounds

    HIViewGetBounds(view, &bounds);

    inset = DrawEdge(context, bounds);

    CGContextSetGrayStrokeColor(context, 0, 1);
    CGContextSetGrayFillColor(context, 0, 1);

    CGContextSetShouldAntialias(context, true);
    CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1, -1));
    CGContextSetTextDrawingMode(context, kCGTextFill);

    // Select font

    CGContextSelectFont(context, "Arial Bold", kTextSize,
			kCGEncodingMacRoman);

    sprintf(s, "%5.1lfHz", display.frequency);
    CGContextShowTextAtPoint(context, 8, kTextSize,
			     s, strlen(s));

    sprintf(s, "%+6.1fdB  ", display.decibels);
    CGContextShowTextAtPoint(context, 8, kTextSize * 2,
			     s, strlen(s));

    return noErr;
}

OSStatus KnobDrawEventHandler(EventHandlerCallRef next,
			      EventRef event, void *data)
{
    CGContextRef context;
    HIViewRef view;
    HIRect bounds;

    // Get context

    GetEventParameter(event, kEventParamCGContextRef,
		      typeCGContextRef, NULL,
		      sizeof(context), NULL,
		      &context);
    // Get view

    GetEventParameter(event, kEventParamDirectObject,
		      typeControlRef, NULL,
		      sizeof(view), NULL,
		      &view);
    // Get bounds

    HIViewGetBounds(view, &bounds);

    CGContextSetGrayFillColor(context, 0.9, 1);
    CGContextSetGrayStrokeColor(context, 0.6, 1);

    CGSize offset =
	{4, -4};
    CGContextSetShadow(context, offset, 4);

    CGRect inset = CGRectInset(bounds, 4, 4);

    CGContextFillEllipseInRect(context, inset);

    return noErr;
}

OSStatus WindowEventHandler(EventHandlerCallRef next,
			    EventRef event, void *data)
{
    // Get the event kind

    UInt32 kind = GetEventKind(event);

    // Switch on event kind

    switch (kind)
    {
    case kEventWindowClose:

	// Close audio unit

	AudioOutputUnitStop(audio.output);
	AudioUnitUninitialize(audio.output);

	// Flush preferences

	// CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);

        // Quit the application

        QuitApplicationEventLoop();
        break;

    default:
        return eventNotHandledErr;
    }

    // Return ok

    return noErr;
}

OSStatus CommandEventHandler(EventHandlerCallRef next,
			     EventRef event, void *data)
{
    HICommandExtended command;
    WindowRef window;
    UInt32 value;

    // Get the command

    GetEventParameter(event, kEventParamDirectObject,
                      typeHICommand, NULL, sizeof(command),
                      NULL, &command);

    // Get the window
    
    window = HIViewGetWindow(command.source.control);

    // Get the value

    value = HIViewGetValue(command.source.control);

    // Switch on the command ID

    switch (command.commandID)
    {
	// Fine

    case kCommandFine:
	ChangeFrequency(value);
	break;

	// Level

    case kCommandLevel:
	ChangeLevel(value);
	break;

	// Sine

    case kCommandSine:
	audio.waveform = kSine;
	break;

	// Square

    case kCommandSquare:
	audio.waveform = kSquare;
	break;

	// Sawtooth

    case kCommandSawtooth:
	audio.waveform = kSawtooth;
	break;

	// Mute

    case kCommandMute:
	audio.mute = value;
	break;

	// Quit

    case kHICommandQuit:

	// Close audio unit

	AudioOutputUnitStop(audio.output);
	AudioUnitUninitialize(audio.output);

	// Flush preferences

	// CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);

	// Let the default handler handle it

    default:
	return eventNotHandledErr;
    }

    ClearKeyboardFocus(window);

    return noErr;
}

// Mouse event handler

OSStatus MouseEventHandler(EventHandlerCallRef next,
			   EventRef event, void *data)
{
    EventMouseButton button;

    // Get button

    GetEventParameter(event, kEventParamMouseButton,
		      typeMouseButton, NULL, sizeof(button),
		      NULL, &button);

    switch (button)
    {
	// Primary button

    case kEventMouseButtonPrimary:
	break;

    default:
	return eventNotHandledErr;
    }

    WindowRef window;
    HIViewRef view;
    UInt32 id;

    // Get window

    GetEventParameter(event, kEventParamWindowRef,
		      typeWindowRef, NULL, sizeof(window),
		      NULL, &window);

    // Get view and id

    HIViewGetViewForMouseEvent(HIViewGetRoot(window), event, &view);
    HIViewGetCommandID(view, &id);

    switch (id)
    {
	// Frequency

    case kCommandFrequency:
	break;

    default:
	return eventNotHandledErr;
    }

    HIRect rect;

    // Get bounds

    HIViewGetBounds(view, &rect);

    HIPoint centre =
	{rect.size.width / 2, rect.size.height / 2};

    HIPoint delta;
    HIPoint previous;
    HIPoint location;

    // Get mouse location

    GetEventParameter(event, kEventParamWindowMouseLocation,
		      typeHIPoint, NULL, sizeof(location),
		      NULL, &location);

    // Get delta

    GetEventParameter(event, kEventParamMouseDelta,
		      typeHIPoint, NULL, sizeof(delta),
		      NULL, &delta);

    // Convert point

    HIViewConvertPoint(&location, NULL, view);

    // Calculate previous location

    previous.x = location.x - delta.x;
    previous.y = location.y - delta.y;

    // Previous offset from centre of knob

    float x = previous.x - centre.x;
    float y = previous.y - centre.y;

    // Angle

    float theta = atan2f(x, -y);

    // Current offset from centre

    x = location.x - centre.x;
    y = location.y - centre.y;

    // Change in angle

    float change = atan2f(x, -y) - theta;

    if (change > M_PI)
	change -= 2.0 * M_PI;

    if (change < -M_PI)
	change += 2.0 * M_PI;

    // Change frequency scale

    scale.value += round(change * 100.0 / M_PI);

    if (scale.value < kFrequencyMin)
	scale.value = kFrequencyMin;

    if (scale.value > kFrequencyMax)
	scale.value = kFrequencyMax;

    // Update display

    HIViewSetNeedsDisplay(scale.view, true);
    UpdateFrequency();

    return noErr;
}

// Keyboard event handler

OSStatus KeyboardEventHandler(EventHandlerCallRef next,
			      EventRef event, void *data)
{
    UInt32 code;
    UInt32 value;

    // Get key code

    GetEventParameter(event, kEventParamKeyCode, typeUInt32,
		      NULL, sizeof(code), NULL, &code);

    // Get fine slider value

    value = HIViewGetValue(sliders.fine);

    switch (code)
    {
	// Up

    case kKeyboardUpKey:
	value++;
	HIViewSetValue(sliders.fine, value);
	break;

	// Down

    case kKeyboardDownKey:
	value--;
	HIViewSetValue(sliders.fine, value);
	break;

	// Left

    case kKeyboardLeftKey:
	scale.value--;

	if (scale.value < kFrequencyMin)
	    scale.value = kFrequencyMin;

	HIViewSetNeedsDisplay(scale.view, true);
	break;

	// Right

    case kKeyboardRightKey:
	scale.value++;

	if (scale.value > kFrequencyMax)
	    scale.value = kFrequencyMax;

	HIViewSetNeedsDisplay(scale.view, true);
	break;

    default:
	return eventNotHandledErr;
    }

    UpdateFrequency();
    return noErr;
}

// Fine action proc

void FineActionProc(HIViewRef view, ControlPartCode part)
{
    // Get the slider value

    UInt32 value = HIViewGetValue(view);
    ChangeFrequency(value);
}

// Change frequency

OSStatus ChangeFrequency(UInt32 value)
{
    // Calculate frequency

    float fine = (float)((signed)value - kFineRef) / 10000.0;
    float frequency = powf(10.0, (float)scale.value /
			   (float)kFrequencyScale) * 10.0;

    display.frequency = frequency + (frequency * fine);
    HIViewSetNeedsDisplay(display.view, true);

    return noErr;
}

// Level action proc

void LevelActionProc(HIViewRef view, ControlPartCode part)
{
    // Get the slider value

    UInt32 value = HIViewGetValue(view);
    ChangeLevel(value);
}

// Change level

OSStatus ChangeLevel(UInt32 value)
{
    // Calculate decibels

    audio.level = 1.0 * (float)value / (float)kLevelMax;
    display.decibels = log10f((float)value / 200.0) * 20.0;

    if (display.decibels < -80.0)
	display.decibels = -80.0;

    HIViewSetNeedsDisplay(display.view, true);

    return noErr;
}

OSStatus UpdateFrequency()
{
    // Get the slider value

    UInt32 value = HIViewGetValue(sliders.fine);
    ChangeFrequency(value);

    return noErr;
}
