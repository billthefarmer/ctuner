////////////////////////////////////////////////////////////////////////////////
//
//  Tuner - A Tuner written in C.
//
//  Copyright (C) 2010	Bill Farmer
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
//  Bill Farmer	 william j farmer [at] yahoo [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

#include "Tuner.h"

// Global data
Scope scope;
Spectrum spectrum;
Display display;
Strobe strobe;
Meter meter;
Legend legend;
Check check;
Arrow arrow;
Audio audio;

// Function main
int main(int argc, char *argv[])
{
    WindowRef window;
    HIViewRef content;
    HIViewRef button;
    HIViewRef group;

    MenuRef menu;

    GetPreferences();

    // Window bounds
    Rect bounds = {0, 0, 394, 320};

    // Create window
    CreateNewWindow(kDocumentWindowClass,
		    kWindowStandardFloatingAttributes |
		    kWindowFullZoomAttribute |
		    kWindowFrameworkScaledAttribute |
		    kWindowStandardHandlerAttribute |
		    kWindowCompositingAttribute,
		    &bounds, &window);

    // Set the title
    SetWindowTitleWithCFString(window, CFSTR("Tuner"));

    // Set resize limits
    HISize min = {320, 394};
    HISize max = {640, 788};

    SetWindowResizeLimits(window, &min, &max);

    // Create an application menu
    CreateNewMenu(0, 0, &menu);

    // Set menu title
    CFStringRef apple = CFSTR("\024");

    SetMenuTitleWithCFString(menu, apple);

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
    DisposeMenu(menu);

    // Create a standard window menu
    CreateStandardWindowMenu(0, &menu);

    // Insert the menu
    InsertMenu(menu, 0);
    DisposeMenu(menu);

    // Show and position the window
    ShowWindow(window);
    RepositionWindow(window, NULL, kWindowAlertPositionOnMainScreen);

    // Find the window content
    HIViewFindByID(HIViewGetRoot(window),
		   kHIViewWindowContentID,
		   &content);

    // Bounds of scope
    bounds.bottom = 32;
    bounds.right  = 280;

    // Create scope pane
    CreateUserPaneControl(window, &bounds, 0, &scope.view);

    // Set command ID
    HIViewSetCommandID(scope.view, kCommandFilter);

    // Set help tag
    HMHelpContentRec help =
	{kMacHelpVersion,
	 {0, 0, 0, 0},
	 kHMInsideLeftCenterAligned,
	 {{kHMCFStringContent,
	   CFSTR("Scope, click to filter audio")},
	  {kHMNoContent, NULL}}};

    HMSetControlHelpContent(scope.view, &help);

    // Place in the window
    HIViewAddSubview(content, scope.view);
    HIViewPlaceInSuperviewAt(scope.view, 20, 20);

    // Bounds of spectrum
    bounds.bottom = 32;
    bounds.right  = 280;

    // Create spectrum pane
    CreateUserPaneControl(window, &bounds, 0, &spectrum.view);

    // Set command ID
    HIViewSetCommandID(spectrum.view, kCommandZoom);

    // Set help tag
    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Spectrum, click to zoom");
    HMSetControlHelpContent(spectrum.view, &help);

    // Place in the window
    HIViewAddSubview(content, spectrum.view);
    HIViewPlaceInSuperviewAt(spectrum.view, 20, 60);

    // Bounds of display
    bounds.bottom = 102;
    bounds.right  = 280;

    // Create display pane
    CreateUserPaneControl(window, &bounds, 0, &display.view);

    // Set command ID
    HIViewSetCommandID(display.view, kCommandLock);

    // Set help tag
    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Display, click to lock");
    HMSetControlHelpContent(display.view, &help);

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

    // Set help tag
    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Strobe, click to display");
    HMSetControlHelpContent(strobe.view, &help);

    // Place in the window
    HIViewAddSubview(content, strobe.view);
    HIViewPlaceInSuperviewAt(strobe.view, 20, 210);

    // Bounds of meter
    bounds.bottom = 52;
    bounds.right  = 280;

    // Create meter pane
    CreateUserPaneControl(window, &bounds,
			  kHIViewFeatureAllowsSubviews, &meter.view);

    // Set help tag
    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Cents meter");
    HMSetControlHelpContent(meter.view, &help);

    // Place in the window
    HIViewAddSubview(content, meter.view);
    HIViewPlaceInSuperviewAt(meter.view, 20, 262);

    // Bounds of slider
    bounds.bottom = 16;
    bounds.right  = 264;

    // Create meter slider
    CreateSliderControl(window, &bounds, kMeterValue, kMeterMin, kMeterMax,
			kControlSliderPointsUpOrLeft, 0, false,
			NULL, &meter.slider);
    // Control size
    ControlSize small = kControlSizeSmall;

    // Set control size
    SetControlData(meter.slider, kControlEntireControl, kControlSizeTag,
		   sizeof(ControlSize), &small);

    // Set help tag
    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Cents meter");
    HMSetControlHelpContent(meter.slider, &help);

    // Place in the window
    HIViewAddSubview(meter.view, meter.slider);
    HIViewPlaceInSuperviewAt(meter.slider, 8, 28);

    HIViewSetZOrder(meter.slider, kHIViewZOrderAbove, meter.view);

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
			    CFSTR("Actual rate: 11025.0"),
			    &style, &legend.status.actual);

    // Place in group box
    HIViewAddSubview(group, legend.status.actual);
    HIViewPlaceInSuperviewAt(legend.status.actual, 164, 2);

    // Layout info
    HILayoutInfo layout =
	{kHILayoutInfoVersionZero};

    // Iterate through views
    HIViewRef view = HIViewGetFirstSubview(content);

    while (view != NULL) 
    {
	// Get content view bounds
	HIRect parent;
	HIViewGetBounds(content, &parent);

	// Get view bounds
	HIRect bounds;
	HIViewGetBounds(view, &bounds);

	// Get layout info
	HIViewGetLayoutInfo(view, &layout);

	// Calculate the scaling
	layout.scale.x.ratio = bounds.size.width / parent.size.width;
	layout.scale.y.ratio = bounds.size.height / parent.size.height;

	// Set layout info
	HIViewSetLayoutInfo(view, &layout);

	// Iterate through subviews
	HIViewRef subview = HIViewGetFirstSubview(view);

	while (subview != NULL)
	{
	    // Get view bounds
	    HIViewGetBounds(view, &parent);

	    // Get subview bounds
	    HIViewGetBounds(subview, &bounds);

	    // Get layout info
	    HIViewGetLayoutInfo(subview, &layout);

	    // Calculate the scaling
	    layout.scale.x.ratio = bounds.size.width / parent.size.width;
	    layout.scale.y.ratio = bounds.size.height / parent.size.height;

	    // Set layout info
	    HIViewSetLayoutInfo(subview, &layout);

	    subview = HIViewGetNextView(subview);
	}

	view = HIViewGetNextView(view);
    }

    // Window events type spec
    EventTypeSpec windowEvents[] =
	{{kEventClassWindow, kEventWindowClose},
	 {kEventClassWindow, kEventWindowZoomed}};

    // Install event handler
    InstallWindowEventHandler(window, NewEventHandlerUPP(WindowEventHandler),
			      Length(windowEvents), windowEvents,
			      NULL, NULL);

    // Command events type spec
    EventTypeSpec commandEvents[] =
	{{kEventClassCommand, kEventCommandProcess}};

    // Install event handler
    InstallApplicationEventHandler(NewEventHandlerUPP(CommandEventHandler),
				   Length(commandEvents), commandEvents,
				   window, NULL);
    // Mouse events type spec
    EventTypeSpec mouseEvents[] =
	{{kEventClassMouse, kEventMouseDown}};

    // Install event handler
    InstallWindowEventHandler(window, NewEventHandlerUPP(MouseEventHandler),
			      Length(mouseEvents), mouseEvents,
			      window, NULL);
    // Text events type spec
    EventTypeSpec textEvents[] =
	{{kEventClassTextInput, kEventTextInputUnicodeForKeyEvent}};

    // Install event handler
    InstallApplicationEventHandler(NewEventHandlerUPP(TextEventHandler),
				   Length(textEvents), textEvents,
				   window, NULL);
    // Audio events type spec
    EventTypeSpec audioEvents[] =
	{{kEventClassApplication, kEventAudioUpdate},
	 {kEventClassApplication, kEventAudioRate}};

    // Install event handler
    InstallApplicationEventHandler(NewEventHandlerUPP(AudioEventHandler),
				   Length(audioEvents), audioEvents,
				   window, NULL);
    // Draw events type spec
    EventTypeSpec drawEvents[] =
	{{kEventClassControl, kEventControlDraw}};

    // Install event handlers
    InstallControlEventHandler(scope.view,
			       NewEventHandlerUPP(ScopeDrawEventHandler),
			       Length(drawEvents), drawEvents,
			       scope.view, NULL);

    InstallControlEventHandler(spectrum.view,
			       NewEventHandlerUPP(SpectrumDrawEventHandler),
			       Length(drawEvents), drawEvents,
			       spectrum.view, NULL);

    InstallControlEventHandler(display.view,
			       NewEventHandlerUPP(DisplayDrawEventHandler),
			       Length(drawEvents), drawEvents,
			       display.view, NULL);

    InstallControlEventHandler(strobe.view,
			       NewEventHandlerUPP(StrobeDrawEventHandler),
			       Length(drawEvents), drawEvents,
			       strobe.view, NULL);

    InstallControlEventHandler(meter.view,
			       NewEventHandlerUPP(MeterDrawEventHandler),
			       Length(drawEvents), drawEvents,
			       meter.view, NULL);

    // Set up timer
    InstallEventLoopTimer(GetMainEventLoop(), kTimerDelay, kTimerDelay,
			  NewEventLoopTimerUPP(TimerProc),
			  NULL, &display.timer);
    // Set up audio
    SetupAudio();

    // Run the application event loop
    RunApplicationEventLoop();

    return 0;
}

// Get preferences

void GetPreferences()
{
    Boolean found;
    Boolean flag;

    CFIndex value;

    strobe.enable = true;
    spectrum.expand = 1;
    audio.reference = kA5Reference;

    // Zoom
    flag = CFPreferencesGetAppBooleanValue(CFSTR("Zoom"),
					   kCFPreferencesCurrentApplication,
					   &found);
    if (found)
	spectrum.zoom = flag;

    // Strobe
    flag = CFPreferencesGetAppBooleanValue(CFSTR("Strobe"),
					   kCFPreferencesCurrentApplication,
					   &found);
    if (found)
	strobe.enable = flag;

    // Colours
    value = CFPreferencesGetAppIntegerValue(CFSTR("Colours"),
					    kCFPreferencesCurrentApplication,
					    &found);
    if (found)
	strobe.colours = value;

    // Filter
    flag = CFPreferencesGetAppBooleanValue(CFSTR("Filter"),
					   kCFPreferencesCurrentApplication,
					   &found);
    if (found)
	audio.filter = flag;

    // Downsample
    flag = CFPreferencesGetAppBooleanValue(CFSTR("Downsample"),
					   kCFPreferencesCurrentApplication,
					   &found);
    if (found)
	audio.downsample = flag;

    // Reference
    value = CFPreferencesGetAppIntegerValue(CFSTR("Reference"),
					    kCFPreferencesCurrentApplication,
					    &found);
    if (found)
	audio.reference = (float)value / 10.0;
}

// Setup audio

OSStatus SetupAudio()
{
    // Specify an output unit
    ComponentDescription dc =
	{kAudioUnitType_Output,
	 kAudioUnitSubType_HALOutput,
	 kAudioUnitManufacturer_Apple,
	 0, 0};

    // Find an output unit
    Component cp
	= FindNextComponent(NULL, &dc);

    if (cp == NULL)
    {
	DisplayAlert(CFSTR("FindNextComponent"),
		     CFSTR("Can't find an output audio unit"),
		     0);
	return -1;
    }

    // Open it
    OSStatus status = OpenAComponent(cp, &audio.output);

    if (status != noErr)
    {
	DisplayAlert(CFSTR("OpenAComponent"),
		     CFSTR("Can't open an output audio unit"),
		     status);
	
	return status;
    }

    UInt32 enable;
    UInt32 size;

    // Enable input
    enable = true;
    status = AudioUnitSetProperty(audio.output,
				  kAudioOutputUnitProperty_EnableIO,
				  kAudioUnitScope_Input,
				  1, &enable, sizeof(enable));
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitSetProperty"),
		     CFSTR("Can't set an output audio unit property"),
		     status);
	return status;
    }

    // Disable output
    enable = false;
    status = AudioUnitSetProperty(audio.output,
				  kAudioOutputUnitProperty_EnableIO,
				  kAudioUnitScope_Output,
				  0, &enable, sizeof(enable));
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitSetProperty"),
		     CFSTR("Can't set an output audio unit property"),
		     status);
	return status;
    }

    AudioDeviceID id;
    size = sizeof(id);

    // Get the default input device
    status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,
				      &size, &id);
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioHardwareGetProperty"), 
		     CFSTR("Can't get the default input device"),
		     status);
	return status;
    }

    // Set the audio unit device
    status = AudioUnitSetProperty(audio.output,
				  kAudioOutputUnitProperty_CurrentDevice, 
				  kAudioUnitScope_Global, 0, &id, sizeof(id));
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitSetProperty"),
		     CFSTR("Can't set output audio unit current device"),
		     status);
	return status;
    }

    // Get nominal sample rates size
    status =
	AudioDeviceGetPropertyInfo(id, 0, true,
				   kAudioDevicePropertyAvailableNominalSampleRates,
				   &size, NULL);
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioDeviceGetPropertyInfo"),
		     CFSTR("Can't get audio device sample rates size"),
		     status);
	return status;
    }

    // Get nominal sample rates
    AudioValueRange *rates = malloc(size);

    status = AudioDeviceGetProperty(id, 0, true,
				    kAudioDevicePropertyAvailableNominalSampleRates,
				    &size, rates);
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioDeviceGetProperty"),
		     CFSTR("Can't get audio device sample rates"),
		     status);

	free(rates);
	return status;
    }

    // See if we can change the sample rate
    bool inrange = false;
    Float64 rate;

    for (int i = 0; i < size / sizeof(AudioValueRange); i++)
    {
	if ((rates[i].mMinimum <= kSampleRate1) &&
	    (rates[i].mMaximum >= kSampleRate1))
	{
	    inrange = true;
	    rate = kSampleRate1;
	    break;
	}

	if ((rates[i].mMinimum <= kSampleRate2) &&
	    (rates[i].mMaximum >= kSampleRate2))
	{
	    inrange = true;
	    rate = kSampleRate2;
	    break;
	}
    }

    // Free rates range

    free(rates);

    // Set the sample rate, if in range
    if (inrange)
	status = AudioDeviceSetProperty(id, NULL, 0, true,
					kAudioDevicePropertyNominalSampleRate,
					sizeof(rate), &rate);
    Float64 nominal;
    size = sizeof(nominal);

    // Get the sample rate
    status = AudioDeviceGetProperty(id, 0, true,
				    kAudioDevicePropertyNominalSampleRate,
				    &size, &nominal);
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioDeviceGetProperty"),
		     CFSTR("Can't get audio device nominal sample rate"),
		     status);
	return status;
    }

    // Set the divisor
    audio.divisor = round(nominal / ((kSampleRate1 + kSampleRate2) / 2));

    // Set the rate
    audio.sample = nominal / audio.divisor;

    // Set the status text
    CFStringRef text =
	CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
				 CFSTR("Sample rate: %6.1lf\n"),
				 audio.sample);

    HIViewSetText(legend.status.sample, text);
    CFRelease(text);

    text =
	CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
				 CFSTR("Actual rate: %6.1lf\n"),
				 audio.sample);

    HIViewSetText(legend.status.actual, text);
    CFRelease(text);

    // Get the buffer size range
    AudioValueRange sizes;
    size = sizeof(sizes);

    status = AudioDeviceGetProperty(id, 0, true,
				    kAudioDevicePropertyBufferFrameSizeRange,
				    &size, &sizes);
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioDeviceGetProperty"),
		     CFSTR("Can't get audio device frame sizes"),
		     status);
	return status;
    }

    UInt32 frames = kStep * audio.divisor;
    size = sizeof(frames);

    while (!((sizes.mMaximum >= frames) &&
	     (sizes.mMinimum <= frames)))
	frames /= 2;

    // Set the max frames
    status = AudioUnitSetProperty(audio.output,
				  kAudioUnitProperty_MaximumFramesPerSlice,
				  kAudioUnitScope_Global, 0,
				  &frames, sizeof(frames));
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitSetProperty"),
		     CFSTR("Can't set output audio unit maximum frames"),
		     status);
	return status;
    }

    // Set the buffer size

    status = AudioDeviceSetProperty(id, NULL, 0, true,
				    kAudioDevicePropertyBufferFrameSize,
				    sizeof(frames), &frames);
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioDeviceSetProperty"),
		     CFSTR("Can't set audio device buffer size"),
		     status);
	return status;
    }

    // Get the frames
    status = AudioUnitGetProperty(audio.output,
				  kAudioUnitProperty_MaximumFramesPerSlice,
				  kAudioUnitScope_Global, 0, &frames, &size);
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitGetProperty"),
		     CFSTR("Can't get output audio unit maximum frames"),
		     status);
	return status;
    }

    audio.frames = frames;

    AudioStreamBasicDescription format;
    size = sizeof(format);

    // Get stream format
    status = AudioUnitGetProperty(audio.output,
				  kAudioUnitProperty_StreamFormat,
				  kAudioUnitScope_Input, 1,
				  &format, &size);
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitGetProperty"),
		     CFSTR("Can't get output audio unit stream format"),
		     status);
	return status;
    }

    format.mSampleRate = nominal;
    format.mBytesPerPacket = kBytesPerPacket;
    format.mBytesPerFrame = kBytesPerFrame;
    format.mChannelsPerFrame = kChannelsPerFrame;

    // Set stream format
    status = AudioUnitSetProperty(audio.output,
				  kAudioUnitProperty_StreamFormat,
				  kAudioUnitScope_Output, 1,
				  &format, sizeof(format));
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitSetProperty"),
		     CFSTR("Can't set output audio unit stream format"),
		     status);
	return status;
    }

    AURenderCallbackStruct input =
	{InputProc, &audio.output};

    // Set callback
    status = AudioUnitSetProperty(audio.output,
				  kAudioOutputUnitProperty_SetInputCallback,
				  kAudioUnitScope_Global, 0,
				  &input, sizeof(input));
    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitSetProperty"),
		     CFSTR("Can't set output audio unit input callback"),
		     status);
	return status;
    }

    // Init the audio unit
    status = AudioUnitInitialize(audio.output);

    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioUnitInitialize"),
		     CFSTR("Can't initialise output audio unit"),
		     status);
	return status;
    }

    // Start the audio unit
    status = AudioOutputUnitStart(audio.output);

    if (status != noErr)
    {
	DisplayAlert(CFSTR("AudioOutputUnitStart"),
		     CFSTR("Can't start output audio unit"),
		     status);
	return status;
    }

    return status;
}

// Display alert
OSStatus DisplayAlert(CFStringRef error, CFStringRef explanation,
		      OSStatus status)
{
    DialogRef dialog;

    if (status == 0)
	CreateStandardAlert(kAlertStopAlert, error, explanation, NULL, &dialog);

    else
    {
	CFStringRef exp;

	if (status > 0)
	{
	    char s[8];

	    CFStringRef stat = UTCreateStringForOSType(status);
	    CFStringGetCString(stat, s, sizeof(s), kCFStringEncodingMacRoman);
	    CFRelease(stat);
    
	    exp =
		CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
					 CFSTR("%s: '%s' (0x%lx)"),
					 CFStringGetCStringPtr(explanation,
							       kCFStringEncodingMacRoman),
					 s, status);
	}

	else
	{
	    exp =
		CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
					 CFSTR("%s: %ld (0x%lx)"),
					 CFStringGetCStringPtr(explanation,
							       kCFStringEncodingMacRoman),
					 status, status);
	}

	CreateStandardAlert(kAlertStopAlert, error, exp, NULL, &dialog);
    }

    SetWindowTitleWithCFString(GetDialogWindow(dialog), CFSTR("Tuner"));
    RunStandardAlert(dialog, NULL, NULL);

    CFRelease(exp);
    return noErr;
}

// Input proc
OSStatus InputProc(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
		   const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
		   UInt32 inNumberFrames, AudioBufferList *ioData)
{
    static AudioBufferList abl =
	{1, {1, 0, NULL}};

    // Initialise data structs
    static Float32 buffer[kSamples];

    if (audio.buffer == NULL)
	audio.buffer = buffer;

    // Render data
    OSStatus status
	= AudioUnitRender(*(AudioUnit *)inRefCon, ioActionFlags,
			  inTimeStamp, inBusNumber,
			  inNumberFrames, &abl);
    if (status != noErr)
	return status;

    // Copy the input data
    memmove(buffer, buffer + (audio.frames / audio.divisor),
	    (kSamples - (audio.frames / audio.divisor)) * sizeof(Float32));

    Float32 *data = abl.mBuffers[0].mData;

    // Butterworth filter, 3dB/octave
    for (int i = 0; i < (audio.frames / audio.divisor); i++)
    {
	static float G = 3.023332184e+01;
	static float K = 0.9338478249;

	static float xv[2];
	static float yv[2];

	xv[0] = xv[1];
	xv[1] = data[i * audio.divisor] / G;

	yv[0] = yv[1];
	yv[1] = (xv[0] + xv[1]) + (K * yv[0]);

	// Choose filtered/unfiltered data
	buffer[(kSamples - (audio.frames / audio.divisor)) + i] =
	    audio.filter? yv[1]: data[i * audio.divisor];
    }

    // Create an event to post to the main event queue
    EventRef event;

    CreateEvent(kCFAllocatorDefault, kEventClassApplication,
		kEventAudioUpdate, 0,
		kEventAttributeUserEvent, &event);

    PostEventToQueue(GetMainEventQueue(), event,
		     kEventPriorityHigh);

    ReleaseEvent(event);

    return noErr;
}

// Audio event handler
OSStatus AudioEventHandler(EventHandlerCallRef next,
			   EventRef event, void *data)
{
    enum
    {kTimerCount = 16};

    // Arrays for processing input

    static float xa[kRange];
    static float xp[kRange];
    static float xq[kRange];
    static float xf[kRange];

    static float x2[kRange / 2];
    static float x3[kRange / 3];
    static float x4[kRange / 4];
    static float x5[kRange / 5];

    static float dxa[kRange];
    static float dxp[kRange];

    static maximum maxima[kMaxima];
    static float   values[kMaxima];

    static float window[kSamples];
    static float input[kSamples];

    static float re[kSamples2];
    static float im[kSamples2];

    static DSPSplitComplex x =
	{re, im};

    static FFTSetup setup;

    static float fps;
    static float expect;

    // Get the event kind
    UInt32 kind = GetEventKind(event);

    // Switch on event kind
    switch (kind)
    {
	// Update fps
    case kEventAudioRate:
	fps = audio.sample / (float)kSamples;
	return noErr;
	break;

	// Audio update
    case kEventAudioUpdate:
	break;
    }

    // Initialise structures
    if (scope.data == NULL)
    {
	scope.data = audio.buffer + kSamples - (audio.frames / audio.divisor);
	scope.length = audio.frames / audio.divisor;

	spectrum.data = xa;
	spectrum.length = kRange;
	spectrum.values = values;

	display.maxima = maxima;

	fps = audio.sample / (float)kSamples;
	expect = 2.0 * M_PI * (float)(audio.frames / audio.divisor) /
	    (float)kSamples;

	// Init Hamming window
	vDSP_hamm_window(window, kSamples, 0);

	// Init FFT
	setup = vDSP_create_fftsetup(kLog2Samples, kFFTRadix2);
    }

    // Maximum data value
    static float dmax;

    if (dmax < 0.125)
	dmax = 0.125;

    // Calculate normalising value
    float norm = dmax;

    // Get max magitude
    vDSP_maxmgv(audio.buffer, 1, &dmax, kSamples);

    // Divide by normalisation
    vDSP_vsdiv(audio.buffer, 1, &norm, input, 1, kSamples);

    // Multiply by window
    vDSP_vmul(input, 1, window, 1, input, 1, kSamples);

    // Copy input to split complex vector
    vDSP_ctoz((COMPLEX *)input, 2, &x, 1, kSamples2);

    // Do FFT
    vDSP_fft_zrip(setup, &x, 1, kLog2Samples, kFFTDirection_Forward);

    // Zero the zeroth part
    x.realp[0] = 0.0;
    x.imagp[0] = 0.0;

    // Scale the output
    float scale = kScale;

    vDSP_vsdiv(x.realp, 1, &scale, x.realp, 1, kSamples2);
    vDSP_vsdiv(x.imagp, 1, &scale, x.imagp, 1, kSamples2);

    // Magnitude
    vDSP_vdist(x.realp, 1, x.imagp, 1, xa, 1, kRange);

    // Phase
    vDSP_zvphas(&x, 1, xq, 1, kRange);

    // Phase difference
    vDSP_vsub(xp, 1, xq, 1, dxp, 1, kRange);

    for (int i = 1; i < kRange; i++)
    {
	// Do frequency calculation
	float dp = dxp[i];

	// Calculate phase difference
	dp -= (float)i * expect;

	int qpd = dp / M_PI;

	if (qpd >= 0)
	    qpd += qpd & 1;

	else
	    qpd -= qpd & 1;

	dp -=  M_PI * (float)qpd;

	// Calculate frequency difference
	float df = kOversample * dp / (2.0 * M_PI);

	// Calculate actual frequency from slot frequency plus
	// frequency difference
	xf[i] = i * fps + df * fps;

	// Calculate differences for finding maxima
	dxa[i] = xa[i] - xa[i - 1];
    }

    // Copy phase vector
    memmove(xp, xq, kRange * sizeof(float));

    // Downsample
    if (audio.downsample)
    {
	// x2 = xa << 2
	for (int i = 0; i < Length(x2); i++)
	{
	    x2[i] = 0.0;

	    for (int j = 0; j < 2; j++)
		x2[i] += xa[(i * 2) + j];
	}

	// x3 = xa << 3
	for (int i = 0; i < Length(x3); i++)
	{
	    x3[i] = 0.0;

	    for (int j = 0; j < 3; j++)
		x3[i] += xa[(i * 3) + j];
	}

	// x4 = xa << 4
	for (int i = 0; i < Length(x4); i++)
	{
	    x4[i] = 0.0;

	    for (int j = 0; j < 4; j++)
		x4[i] += xa[(i * 4) + j];
	}

	// x5 = xa << 5
	for (int i = 0; i < Length(x5); i++)
	{
	    x5[i] = 0.0;

	    for (int j = 0; j < 5; j++)
		x5[i] += xa[(i * 5) + j];
	}

	// Add downsamples
	for (int i = 0; i < Length(xa); i++)
	{
            xa[i] *= (i < Length(x2))? x2[i]: 0.0;
            xa[i] *= (i < Length(x3))? x3[i]: 0.0;
            xa[i] *= (i < Length(x4))? x4[i]: 0.0;
            xa[i] *= (i < Length(x5))? x5[i]: 0.0;

	    // Calculate differences for finding maxima
	    dxa[i] = xa[i] - xa[i - 1];

	}
    }

    // Maximum FFT output
    float  max;
    UInt32 imax;

    vDSP_maxmgvi(xa, 1, &max, &imax, kRange);

    float f = xf[imax];

    int count = 0;
    int limit = kRange - 1;

    // Find maximum value, and list of maxima
    for (int i = 1; i < limit; i++)
    {
	// If display not locked, find maxima and add to list
	if (!display.lock && count < Length(maxima) &&
	    xa[i] > kMin && xa[i] > (max / 2) &&
	    dxa[i] > 0.0 && dxa[i + 1] < 0.0)
	{
	    maxima[count].f = xf[i];

	    // Cents relative to reference
	    float cf =
		-12.0 * log2f(audio.reference / xf[i]);

	    // Reference note
	    maxima[count].fr = audio.reference * powf(2.0, round(cf) / 12.0);

	    // Note number
	    maxima[count].n = round(cf) + kC5Offset;

	    // Set limit to octave above
	    if (!audio.downsample && (limit > i * 2))
		limit = i * 2 - 1;

	    count++;
	}
    }

    // Reference note frequency and lower and upper limits

    float fr = 0.0;
    float fl = 0.0;
    float fh = 0.0;

    // Note number
    int n = 0;

    // Found flag and cents value
    bool found = false;
    float c = 0.0;

    // Do the note and cents calculations
    if (max > kMin)
    {
	found = true;

	// Frequency
	if (!audio.downsample)
	    f = maxima[0].f;

	// Cents relative to reference
	float cf =
	    -12.0 * log2f(audio.reference / f);

	// Reference note
	fr = audio.reference * powf(2.0, round(cf) / 12.0);

	// Lower and upper freq
	fl = audio.reference * powf(2.0, (round(cf) - 0.55) / 12.0);
	fh = audio.reference * powf(2.0, (round(cf) + 0.55) / 12.0);

	// Note number
	n = round(cf) + kC5Offset;

	if (n < 0)
	    found = false;

	// Find nearest maximum to reference note
	float df = 1000.0;

	for (int i = 0; i < count; i++)
	{
	    if (fabs(maxima[i].f - fr) < df)
	    {
		df = fabsf(maxima[i].f - fr);
		f = maxima[i].f;
	    }
	}

	// Cents relative to reference note
	c = -12.0 * log2f(fr / f);

	// Ignore silly values
	if (!isfinite(c))
	    c = 0.0;

	// Ignore if not within 50 cents of reference note
	if (fabsf(c) > 0.5)
	    found = false;
    }

    // If display not locked
    if (!display.lock)
    {
	// Update scope window
	HIViewSetNeedsDisplay(scope.view, true);

	// Update spectrum window
	for (int i = 0; i < count; i++)
	    values[i] = maxima[i].f / fps;

	spectrum.count = count;

	if (found)
	{
	    spectrum.f = f  / fps;
	    spectrum.r = fr / fps;
	    spectrum.l = fl / fps;
	    spectrum.h = fh / fps;
	}

	HIViewSetNeedsDisplay(spectrum.view, true);
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

	    // Update display
	    HIViewSetNeedsDisplay(display.view, true);

	    // Update meter
	    meter.c = c;

	    // Update strobe
	    strobe.c = c;
	}

	// Reset count;
	timer = 0;
    }

    else
    {
	// If display not locked
	if (!display.lock)
	{

	    if (timer == kTimerCount)
	    {
		display.f = 0.0;
		display.fr = 0.0;
		display.c = 0.0;
		display.n = 0;
		display.count = 0;

		// Update display
		HIViewSetNeedsDisplay(display.view, true);

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
	}
    }

    timer++;

    return noErr;
}

#ifdef DEBUG

// Copy info
OSStatus CopyInfo(EventRef event)
{
    AudioDeviceID id;
    UInt32 size = sizeof(id);

    // Get the default input device
    AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,
			     &size, &id);

    // Get sample rates size
    AudioDeviceGetPropertyInfo(id, 0, true,
			       kAudioDevicePropertyAvailableNominalSampleRates,
			       &size, NULL);

    // Allocate memory
    AudioValueRange *range = malloc(size);

    // Get the sample rates
    AudioDeviceGetProperty(id, 0, true,
			   kAudioDevicePropertyAvailableNominalSampleRates,
			   &size, range);
    char s[64];
    char *text = malloc(16384);

    strcpy(text, "Sample rates:\n");

    for (int i = 0; i < size / sizeof(AudioValueRange); i++)
    {
	sprintf(s, "%d: %6.1lf - %6.1lf\n", i,
		range[i].mMinimum, range[i].mMaximum);

	strcat(text, s);
    }

    // Free memory
    free(range);

    Float64 rate;
    size = sizeof(rate);

    // Get the nominal sample rate
    AudioDeviceGetProperty(id, 0, true,
			   kAudioDevicePropertyNominalSampleRate,
			   &size, &rate);

    sprintf(s, "Nominal sample rate: %6.1lf\n", rate);
    strcat(text, s);

    // Get the actual sample rate
    AudioDeviceGetProperty(id, 0, true,
			   kAudioDevicePropertyActualSampleRate,
			   &size, &rate);

    sprintf(s, "Actual sample rate: %6.1lf\n", rate);
    strcat(text, s);

    sprintf(s, "Audio divisor: %d\n", audio.divisor);
    strcat(text, s);

    // Get frame sizes size
    AudioDeviceGetPropertyInfo(id, 0, true,
			       kAudioDevicePropertyBufferFrameSizeRange,
			       &size, NULL);
    // Allocate memory
    range = malloc(size);

    // Get the frame sizes
    AudioDeviceGetProperty(id, 0, true,
			   kAudioDevicePropertyBufferFrameSizeRange,
			   &size, range);
   
    strcat(text, "Frame sizes:\n");

    for (int i = 0; i < size / sizeof(AudioValueRange); i++)
    {
	sprintf(s, "%d: %6.1lf - %6.1lf\n", i,
		range[i].mMinimum, range[i].mMaximum);

	strcat(text, s);
    }

    // Free memory
    free(range);

    // Frames
    UInt32 frames;
    size = sizeof(frames);

    AudioUnitGetProperty(audio.output,
			 kAudioUnitProperty_MaximumFramesPerSlice,
			 kAudioUnitScope_Global, 0, &frames, &size);

    sprintf(s, "Maximum frames: %ld\n", frames);
    strcat(text, s);

    // Create a pasteboard
    PasteboardRef paste;

    PasteboardCreate(kPasteboardClipboard, &paste);
    PasteboardClear(paste);

    CFDataRef data = CFDataCreate(kCFAllocatorDefault,
				  (UInt8 *)text, strlen(text));

    PasteboardPutItemFlavor(paste, (PasteboardItemID)data,
			    kUTTypeUTF8PlainText, data,
			    kPasteboardFlavorNoFlags);
    // Free resources
    free(text);
    CFRelease(data);
    CFRelease(paste);

    return noErr;
}

#endif

// Timer proc

void TimerProc(EventLoopTimerRef timer, void *data)
{
    static long iterations;
    static float mc;

    if (strobe.enable)
	HIViewSetNeedsDisplay(strobe.view, true);

    // Do meter calculation
    mc = ((mc * 7.0) + meter.c) / 8.0;

    int value = round(mc * kMeterMax) + kMeterValue;

    // Update meter
    HIViewSetValue(meter.slider, value);

    // Update sample rate

    if ((++iterations % kFrames) == 0)
    {
	AudioDeviceID id;
	UInt32 size = sizeof(id);

	// Get the default input device
	AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,
				 &size, &id);
	Float64 rate;
	size = sizeof(rate);

	// Get the actual sample rate
	AudioDeviceGetProperty(id, 0, true,
			       kAudioDevicePropertyActualSampleRate,
			       &size, &rate);

	// Update the notional rate
	audio.sample = rate / audio.divisor;

	CFStringRef text =
	    CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
				     CFSTR("Actual rate: %6.1lf"),
				     audio.sample);

	HIViewSetText(legend.status.actual, text);
	CFRelease(text);

	// Create an event to post to the main event queue
	EventRef event;

	CreateEvent(kCFAllocatorDefault, kEventClassApplication,
		    kEventAudioRate, 0,
		    kEventAttributeUserEvent, &event);

	PostEventToQueue(GetMainEventQueue(), event,
			 kEventPriorityStandard);

	ReleaseEvent(event);
    }
}

// Draw edge

HIRect DrawEdge(CGContextRef context, HIRect bounds)
{
    // Scale context and adjust bounds
    if (display.zoom)
    {
	CGContextScaleCTM(context, 2.0, 2.0);

	bounds.size.width /= 2.0;
	bounds.size.height /= 2.0;
    }

    CGContextSetShouldAntialias(context, true);
    CGContextSetLineWidth(context, 3);
    CGContextSetGrayStrokeColor(context, 0.8, 1);

    // Draw edge
    StrokeRoundRect(context, bounds, 7);

    // Create inset

    CGRect inset = CGRectInset(bounds, 2, 2);
    CGContextClipToRect(context, inset);

    return inset;
}

OSStatus StrokeRoundRect(CGContextRef context, CGRect rect, float radius)
{
    CGPoint point = rect.origin;
    CGSize size = rect.size;

    CGContextBeginPath(context);

    CGContextMoveToPoint(context, point.x + radius, point.y);
    CGContextAddLineToPoint(context, point.x + size.width - radius, point.y);
    CGContextAddArcToPoint(context, point.x + size.width, point.y,
			   point.x + size.width, point.y + radius, radius);
    CGContextAddLineToPoint(context, point.x + size.width,
			    point.y + size.height - radius);
    CGContextAddArcToPoint(context, point.x + size.width,
			   point.y + size.height,
			   point.x + size.width - radius,
			   point.y + size.height, radius);
    CGContextAddLineToPoint(context, point.x + radius, point.y + size.height);
    CGContextAddArcToPoint(context, point.x, point.y + size.height,
			   point.x, point.y + size.height - radius, radius);
    CGContextAddLineToPoint(context, point.x, point.y + radius);
    CGContextAddArcToPoint(context, point.x, point.y, point.x + radius,
			   point.x, radius);

    CGContextStrokePath(context);

    return noErr;
}

OSStatus ScopeDrawEventHandler(EventHandlerCallRef next,
			       EventRef event, void *data)
{
    enum
    {kTextSize = 10};

    CGContextRef context;
    HIRect bounds, inset;
    HIViewRef view;

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

    CGContextSetShouldAntialias(context, false);
    CGContextSetLineWidth(context, 1);

    // Black background
    CGContextSetGrayFillColor(context, 0, 1);
    CGContextFillRect(context, inset);

    CGContextTranslateCTM(context, inset.origin.x, inset.origin.y);

    // Dark green graticule
    CGContextSetRGBStrokeColor(context, 0, 0.6, 0, 1);

    // Draw graticule
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

    // No trace if no data
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

    // Green trace
    CGContextSetRGBStrokeColor(context, 0, 1, 0, 1);

    // Move the origin
    CGContextTranslateCTM(context, 0, height / 2);

    // Calculate scale
    static float max;

    if (max < 0.125)
	max = 0.125;

    float yscale = max / (float)(height / 2);

    max = 0.0;

    // Draw the trace

    CGContextBeginPath(context);
    CGContextMoveToPoint(context, 0, 0);

    for (int i = 0; i < width; i++)
    {
	if (max < fabs(scope.data[n + i]))
	    max = fabs(scope.data[n + i]);

	float y = scope.data[n + i] / yscale;
	CGContextAddLineToPoint(context, i, y);
    }

    CGContextStrokePath(context);

    // Show F if filtered
    if (audio.filter)
    {
	// Yellow text
	CGContextSetRGBStrokeColor(context, 1, 1, 0, 1);
	CGContextSetRGBFillColor(context, 1, 1, 0, 1);

	// Select font
	CGContextSelectFont(context, "Arial Bold", kTextSize,
			    kCGEncodingMacRoman);

	CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1, -1));
	CGContextSetTextDrawingMode(context, kCGTextFill);
	CGContextSetShouldAntialias(context, true);

	CGContextShowTextAtPoint(context, 0, height / 2 - 2, "F", 1);
    }

    return noErr;
}

OSStatus SpectrumDrawEventHandler(EventHandlerCallRef next,
				  EventRef event, void *data)
{
    enum
    {kTextSize = 10};

    static char s[16];

    CGContextRef context;
    HIRect bounds, inset;
    HIViewRef view;

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

    int width = inset.size.width;
    int height = inset.size.height;

    CGContextSetShouldAntialias(context, false);
    CGContextSetLineWidth(context, 1);

    // Black background
    CGContextSetGrayFillColor(context, 0, 1);
    CGContextFillRect(context, inset);

    CGContextTranslateCTM(context, inset.origin.x, inset.origin.y);

    // Dark green graticule
    CGContextSetRGBStrokeColor(context, 0, 0.6, 0, 1);

    // Draw graticule
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

    // Don't attempt the trace until there's a buffer
    if (spectrum.data == NULL)
	return noErr;

    // Move the origin
    CGContextTranslateCTM(context, 0, height - 1);

    static float max;

    if (max < 1.0)
	max = 1.0;

    // Calculate the scaling
    float yscale = (float)height / max;

    max = 0.0;

    // Green pen for spectrum trace
    CGContextSetRGBStrokeColor(context, 0, 1, 0, 1);

    // Draw the spectrum
    CGContextBeginPath(context);
    CGContextMoveToPoint(context, 0, 0);

    if (spectrum.zoom)
    {
	// Calculate scale
	float xscale = ((float)width / (spectrum.r - spectrum.l)) / 2.0;

	// Draw trace
	for (int i = floorf(spectrum.l); i <= ceilf(spectrum.h); i++)
	{
	    if (i > 0 && i < spectrum.length)
	    {
		float value = spectrum.data[i];

		if (max < value)
		    max = value;

		float y = -value * yscale;
		float x = ((float)i - spectrum.l) * xscale; 

		CGContextAddLineToPoint(context, x, y);
	    }
	}

	CGContextMoveToPoint(context, width / 2, 0);
	CGContextAddLineToPoint(context, width / 2, -height);
	CGContextStrokePath(context);

	// Yellow pen for frequency trace
	CGContextSetRGBStrokeColor(context, 1, 1, 0, 1);
	CGContextSetRGBFillColor(context, 1, 1, 0, 1);
	CGContextBeginPath(context);

	// Draw line for nearest frequency
	for (int i = 0; i < spectrum.count; i++)
	{
	    // Draw line for values that are in range

	    if (spectrum.values[i] > spectrum.l &&
		spectrum.values[i] < spectrum.h)
	    {
		float x = (spectrum.values[i] - spectrum.l) * xscale;
		CGContextMoveToPoint(context, x, 0);
		CGContextAddLineToPoint(context, x, -height);
	    }
	}

	CGContextStrokePath(context);

	// Select font
	CGContextSelectFont(context, "Arial Bold", kTextSize,
			    kCGEncodingMacRoman);

	CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1, -1));
	CGContextSetTextDrawingMode(context, kCGTextFill);
	CGContextSetShouldAntialias(context, true);

	for (int i = 0; i < spectrum.count; i++)
	{
	    // Show value for values that are in range
	    if (spectrum.values[i] > spectrum.l &&
		spectrum.values[i] < spectrum.h)
	    {
		float f = display.maxima[i].f;

		// Reference freq
		float fr = display.maxima[i].fr;

		float c = -12.0 * log2f(fr / f);

		// Ignore silly values
		if (!isfinite(c))
		    continue;

		float x = (spectrum.values[i] - spectrum.l) * xscale;

		sprintf(s, "%+0.0f", c * 100.0);
		CentreTextAtPoint(context, x, -1, s, strlen(s));
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

	    float y = -value * yscale;

	    CGContextAddLineToPoint(context, x, y);
	}

	CGContextStrokePath(context);

	// Yellow pen for frequency trace
	CGContextSetRGBStrokeColor(context, 1, 1, 0, 1);
	CGContextSetRGBFillColor(context, 1, 1, 0, 1);
	CGContextBeginPath(context);

	for (int i = 0; i < spectrum.count; i++)
	{
	    // Draw line for values
	    float x = spectrum.values[i] / xscale;
	    CGContextMoveToPoint(context, x, 0);
	    CGContextAddLineToPoint(context, x, -height);
	}

	CGContextStrokePath(context);

	// Select font
	CGContextSelectFont(context, "Arial Bold", kTextSize,
			    kCGEncodingMacRoman);

	CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1, -1));
	CGContextSetTextDrawingMode(context, kCGTextFill);
	CGContextSetShouldAntialias(context, true);

	for (int i = 0; i < spectrum.count; i++)
	{
	    // Show value for values
	    float f = display.maxima[i].f;

	    // Reference freq
	    float fr = display.maxima[i].fr;

	    float c = -12.0 * log2f(fr / f);

	    // Ignore silly values
	    if (!isfinite(c))
		continue;

	    float x = spectrum.values[i] / xscale;

	    sprintf(s, "%+0.0f", c * 100.0);
	    CentreTextAtPoint(context, x, -1, s, strlen(s));
	}

	if (spectrum.expand > 1)
	{
	    sprintf(s, "x%d", spectrum.expand);
	    CGContextShowTextAtPoint(context, 0, -1, s, strlen(s));
	}
    }

    if (audio.downsample)
	CGContextShowTextAtPoint(context, 0, 8 - height, "D", 1);

    return noErr;
}

// Centre text at point
OSStatus CentreTextAtPoint(CGContextRef context, float x, float y,
			   const char * bytes, size_t length)
{
    CGContextSetTextDrawingMode(context, kCGTextInvisible);
    CGContextShowTextAtPoint(context, x, y, bytes, length);

    CGPoint point = CGContextGetTextPosition(context);

    float dx = (point.x - x) / 2.0;
    float dy = (point.y - y) / 2.0;

    CGContextSetTextDrawingMode(context, kCGTextFill);
    CGContextShowTextAtPoint(context, x - dx, y - dy, bytes, length);

    return noErr;
}

// Right justify text
OSStatus RightJustifyTextAtPoint(CGContextRef context, float x, float y,
				 const char * bytes, size_t length)
{
    CGContextSetTextDrawingMode(context, kCGTextInvisible);
    CGContextShowTextAtPoint(context, x, y, bytes, length);

    CGPoint point = CGContextGetTextPosition(context);

    float dx = point.x - x;
    float dy = point.y - y;

    CGContextSetTextDrawingMode(context, kCGTextFill);
    CGContextShowTextAtPoint(context, x - dx, y - dy, bytes, length);

    return noErr; 
}

OSStatus DisplayDrawEventHandler(EventHandlerCallRef next,
				 EventRef event, void *data)
{
    enum
    {kTextSizeLarger = 48,
     kTextSizeLarge  = 36,
     kTextSizeMusic  = 26,
     kTextSizeMedium = 24,
     kTextSizeSmall  = 12};

    static char *notes[] =
	{"C", "C", "D", "E", "E", "F",
	 "F", "G", "A", "A", "B", "B"};

    static char *sharps[] =
	{"", "#", "", "b", "", "",
	 "#", "", "b", "", "b", ""};

    static char s[64];

    CGContextRef context;
    HIRect bounds, inset;
    HIViewRef view;

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

    int width  = inset.size.width;
    int height = inset.size.height;

    CGContextTranslateCTM(context, inset.origin.x, inset.origin.y);
    CGContextSetLineWidth(context, 1);

    CGContextSetShouldAntialias(context, true);
    CGContextSetTextMatrix(context, CGAffineTransformMakeScale(1, -1));
    CGContextSetTextDrawingMode(context, kCGTextFill);

    if (display.multiple)
    {
	// Select font
	CGContextSelectFont(context, "Arial", kTextSizeSmall,
			    kCGEncodingMacRoman);

	if (display.count == 0)
	{
	    // Display note
	    sprintf(s, "%s%s%d", notes[display.n % Length(notes)],
		    sharps[display.n % Length(sharps)], display.n / 12);
	    CGContextShowTextAtPoint(context, 0, kTextSizeSmall,
				     s, strlen(s));
	    // Display cents
	    sprintf(s, "%+4.2lf\242", display.c * 100.0);
	    CGContextShowTextAtPoint(context, 30, kTextSizeSmall,
				     s, strlen(s));
	    // Display reference
	    sprintf(s, "%4.2lfHz", display.fr);
	    CGContextShowTextAtPoint(context, 76, kTextSizeSmall,
				     s, strlen(s));
	    // Display frequency
	    sprintf(s, "%4.2lfHz", display.f);
	    CGContextShowTextAtPoint(context, 146, kTextSizeSmall,
				     s, strlen(s));
	    // Display difference
	    sprintf(s, "%+4.2lfHz", display.f - display.fr);
	    CGContextShowTextAtPoint(context, 220, kTextSizeSmall,
				     s, strlen(s));
	}

	for (int i = 0; i < display.count; i++)
	{
	    float f = display.maxima[i].f;

	    // Reference freq
	    float fr = display.maxima[i].fr;

	    int n = display.maxima[i].n;

	    if (n < 0)
		n = 0;

	    float c = -12.0 * log2f(fr / f);

	    // Ignore silly values
	    if (!isfinite(c))
		continue;

	    // Display note
	    sprintf(s, "%s%s%d", notes[n % Length(notes)],
		    sharps[n % Length(sharps)], n / 12);
	    CGContextShowTextAtPoint(context, 0, (i + 1) * kTextSizeSmall,
				     s, strlen(s));
	    // Display cents
	    sprintf(s, "%+4.2lf\242", c * 100.0);
	    CGContextShowTextAtPoint(context, 30, (i + 1) * kTextSizeSmall,
				     s, strlen(s));
	    // Display reference
	    sprintf(s, "%4.2lfHz", fr);
	    CGContextShowTextAtPoint(context, 76, (i + 1) * kTextSizeSmall,
				     s, strlen(s));
	    // Display frequency
	    sprintf(s, "%4.2lfHz", f);
	    CGContextShowTextAtPoint(context, 146, (i + 1) * kTextSizeSmall,
				     s, strlen(s));
	    // Display difference
	    sprintf(s, "%+4.2lfHz", f - fr);
	    CGContextShowTextAtPoint(context, 220, (i + 1) * kTextSizeSmall,
				     s, strlen(s));
	}
    }

    else
    {
	// Select font
	CGContextSelectFont(context, "Arial Bold", kTextSizeLarger,
			    kCGEncodingMacRoman);

	int y = 40;

	sprintf(s, "%s", notes[display.n % Length(notes)]);
	CGContextShowTextAtPoint(context, 8, y, s, strlen(s));
	
	CGPoint point = CGContextGetTextPosition(context);

	CGContextSetFontSize(context, kTextSizeMedium);
	sprintf(s, "%d", display.n / 12); 
	CGContextShowText(context, s, strlen(s));

	CGContextSelectFont(context, "Musica", kTextSizeMusic,
			    kCGEncodingMacRoman);

	sprintf(s, "%s", sharps[display.n % Length(sharps)]);
	CGContextShowTextAtPoint(context, point.x + 8, point.y -
				 20, s, strlen(s));

	CGContextSelectFont(context, "Arial Bold", kTextSizeLarge,
			    kCGEncodingMacRoman);

	sprintf(s, "%+4.2lf\242", display.c * 100.0);
	RightJustifyTextAtPoint(context, width - 8, y, s, strlen(s));

	y += kTextSizeMedium + 4;
	CGContextSetFontSize(context, kTextSizeMedium);

	sprintf(s, "%4.2lfHz", display.fr);
	CGContextShowTextAtPoint(context, 8, y, s, strlen(s));

	sprintf(s, "%4.2lfHz", display.f);
	RightJustifyTextAtPoint(context, width - 8, y, s, strlen(s));

	y += kTextSizeMedium;

	sprintf(s, "%4.2lfHz", (audio.reference == 0)?
		kA5Reference: audio.reference);
	CGContextShowTextAtPoint(context, 8, y, s, strlen(s));

	sprintf(s, "%+4.2lfHz", display.f - display.fr);
	RightJustifyTextAtPoint(context, width - 8, y, s, strlen(s));
    }

    // Draw lock if locked
    if (display.lock)
	DrawLock(context, 0, height);

    return noErr;
}

// Draw lock

OSStatus DrawLock(CGContextRef context, int x, int y)
{
    // Translate context
    CGContextTranslateCTM(context, x, y);
    CGContextSetShouldAntialias(context, false);
    CGContextSetGrayStrokeColor(context, 0, 1);
    CGContextSetLineWidth(context, 1);

    // Draw rect
    CGContextStrokeRect(context, CGRectMake(2, -8, 6, 6));

    // Draw hasp
    CGContextBeginPath(context);

    CGContextMoveToPoint(context, 3, -8);
    CGContextAddLineToPoint(context, 3, -10);

    CGContextMoveToPoint(context, 7, -8);
    CGContextAddLineToPoint(context, 7, -10);

    CGContextMoveToPoint(context, 4, -11);
    CGContextAddLineToPoint(context, 6, -11);

    CGContextStrokePath(context);

    return noErr;
}

OSStatus StrobeDrawEventHandler(EventHandlerCallRef next,
				EventRef event, void *data)
{
    CGContextRef context;
    HIRect bounds, inset;
    HIViewRef view;

    // Colours
    static CGFloat colours[][2][4] =
	{{{0.25, 0.25, 1, 1}, {0.25, 1, 1, 1}},
	 {{0.5, 0.5, 0, 1}, {0.5, 1, 0.85, 1}},
	 {{1, 0.25, 1, 1}, {1, 1, 0.25, 1}}};

    static CGColorRef foreground;
    static CGColorRef background;

    static CGGradientRef gradient;

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

    // Create patterns
    if (gradient == NULL || strobe.changed)
    {
	if (foreground != NULL)
	    CGColorRelease(foreground);

	if (background != NULL)
	    CGColorRelease(background);

	if (gradient != NULL)
	    CGGradientRelease(gradient);

	// Create colours
	CGColorSpaceRef colourSpace =
	    CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);

	foreground =
	    CGColorCreate(colourSpace, colours[strobe.colours][0]);
	background =
	    CGColorCreate(colourSpace, colours[strobe.colours][1]);

	CGColorSpaceRelease(colourSpace);

	// Create gradient
	CGColorRef gradientColours[] =
	    {foreground, background, foreground};
	CFArrayRef colourArray =
	    CFArrayCreate(kCFAllocatorDefault, (const void **)&gradientColours,
			  3, &kCFTypeArrayCallBacks);

	gradient =
	    CGGradientCreateWithColors(colourSpace, colourArray, NULL);

	strobe.changed = false;
    }

    // Draw edge
    inset = DrawEdge(context, bounds);

    static float mc = 0.0;
    static float mx = 0.0;

    int width = inset.size.width;

    CGContextTranslateCTM(context, inset.origin.x, inset.origin.y);

    if (strobe.enable)
    {
	mc = ((7.0 * mc) + strobe.c) / 8.0;
	mx += mc * 50.0;

	if (mx > 160.0)
	    mx = 0.0;

	if (mx < 0.0)
	    mx = 160.0;

	float rx = mx - 160;


	if (fabsf(mc) > 0.4)
	{
	    CGContextSetFillColorWithColor(context, background);
	    CGContextFillRect(context, CGRectMake(0, 0, width, 10));	    
	}

	else if (fabsf(mc) > 0.2)
	{
	    CGContextSaveGState(context);
	    CGContextClipToRect(context, CGRectMake(0, 0, width, 10));

	    for (float x = fmodf(rx, 20); x <= width; x += 20)
		CGContextDrawLinearGradient(context, gradient,
					    CGPointMake(x, 0),
					    CGPointMake(x + 20, 0), 0);

	    CGContextRestoreGState(context);
	}

	else
	{
	    for (float x = fmodf(rx, 20); x <= width; x += 20)
	    {
		CGContextSetFillColorWithColor(context, foreground);
		CGContextFillRect(context, CGRectMake(x, 0, 10, 10));

		CGContextSetFillColorWithColor(context, background);
		CGContextFillRect(context, CGRectMake(x + 10, 0, 10, 10));
	    }
	}

	if (fabsf(mc) > 0.4)
	{
	    CGContextSaveGState(context);
	    CGContextClipToRect(context, CGRectMake(0, 10, width, 10));

	    for (float x = fmodf(rx, 40); x <= width; x += 40)
		CGContextDrawLinearGradient(context, gradient,
					    CGPointMake(x, 0),
					    CGPointMake(x + 40, 0), 0);

	    CGContextRestoreGState(context);
	}

	else
	{
	    for (float x = fmodf(rx, 40); x <= width; x += 40)
	    {
		CGContextSetFillColorWithColor(context, foreground);
		CGContextFillRect(context, CGRectMake(x, 10, 20, 10));

		CGContextSetFillColorWithColor(context, background);
		CGContextFillRect(context, CGRectMake(x + 20, 10, 20, 10));
	    }
	}

	for (float x = fmodf(rx, 80); x <= width; x += 80)
	{
	    CGContextSetFillColorWithColor(context, foreground);
	    CGContextFillRect(context, CGRectMake(x, 20, 40, 10));

	    CGContextSetFillColorWithColor(context, background);
	    CGContextFillRect(context, CGRectMake(x + 40, 20, 40, 10));
	}

	for (float x = fmodf(rx, 160); x <= width; x += 160)
	{
	    CGContextSetFillColorWithColor(context, foreground);
	    CGContextFillRect(context, CGRectMake(x, 30, 80, 10));

	    CGContextSetFillColorWithColor(context, background);
	    CGContextFillRect(context, CGRectMake(x + 80, 30, 80, 10));
	}
    }

    return noErr;
}

OSStatus MeterDrawEventHandler(EventHandlerCallRef next,
			       EventRef event, void *data)
{
    CGContextRef context;
    HIRect bounds, inset;
    HIViewRef view;

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

    int width = inset.size.width;

    CGContextTranslateCTM(context, 2, 3);
    CGContextSetLineWidth(context, 1);

    CGContextSetGrayStrokeColor(context, 0, 1);
    CGContextSetGrayFillColor(context, 0, 1);

    CGContextTranslateCTM(context, width / 2, 0);
    CGContextSetShouldAntialias(context, true);

    // Select font
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
    UInt32 kind;

    // Get the event kind
    kind = GetEventKind(event);

    // Switch on event kind

    switch (kind)
    {
	// Window zoomed event
    case kEventWindowZoomed:
	WindowZoomed(event, data);
	break;

	// Window close event

    case kEventWindowClose:

	// Close audio unit
	AudioOutputUnitStop(audio.output);
	AudioUnitUninitialize(audio.output);

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

// Window zoomed
OSStatus WindowZoomed(EventRef event, void *data)
{
    WindowRef window;
    HIViewRef content;

    // Change the zoom value
    display.zoom = !display.zoom;

    // Get the window
    GetEventParameter(event, kEventParamDirectObject,
		      typeWindowRef, NULL, sizeof(window),
		      NULL, &window);

    // Find the window content
    HIViewFindByID(HIViewGetRoot(window),
		   kHIViewWindowContentID,
		   &content);

    // Control size
    ControlSize size;

    if (display.zoom)
	size = kControlSizeNormal;

    else
	size = kControlSizeSmall;

    // Iterate through the views
    HIViewRef view = HIViewGetFirstSubview(content);

    while (view != NULL) 
    {
	HIRect bounds;
	HIViewKind kind;

	// Get the bounds
	HIViewGetFrame(view, &bounds);

	// Get the kind
	HIViewGetKind(view, &kind);

	// Move the view
	if (display.zoom)
	    HIViewMoveBy(view, bounds.origin.x, bounds.origin.y);

	else
	    HIViewMoveBy(view, -bounds.origin.x / 2,
			 -bounds.origin.y / 2);

	switch (kind.kind)
	{
	case kControlKindSlider:

	    // Set control size
	    SetControlData(view, kControlEntireControl, kControlSizeTag,
			   sizeof(size), &size);
	    HIViewGetOptimalBounds(view, &bounds, NULL);
	    HIViewSetFrame(view, &bounds);
	    break;
	}

	// Iterate through subviews
	HIViewRef subview = HIViewGetFirstSubview(view);

	while (subview != NULL)
	{
	    // Get the bounds
	    HIViewGetFrame(subview, &bounds);

	    // Get the kind
	    HIViewGetKind(subview, &kind);

	    // Move the view
	    if (display.zoom)
		HIViewMoveBy(subview, bounds.origin.x, bounds.origin.y);

	    else
		HIViewMoveBy(subview, -bounds.origin.x / 2,
			     -bounds.origin.y / 2);

	    // Font style
	    ControlFontStyleRec style;

	    switch (kind.kind)
	    {
	    case kControlKindSlider:

		// Set control size
		SetControlData(subview, kControlEntireControl, kControlSizeTag,
			       sizeof(size), &size);
		HIViewGetOptimalBounds(subview, &bounds, NULL);
		HIViewSetFrame(subview, &bounds);
		break;

	    case kControlKindStaticText:

		// Get the style
		GetControlData(subview, kControlEntireControl,
			       kControlFontStyleTag,
			       sizeof(style), &style, NULL);

		if (display.zoom)
		    style.font = kControlFontBigSystemFont;

		else
		    style.font = kControlFontSmallSystemFont;

		// Set control font size
		SetControlFontStyle(subview, &style);
		break;
	    }

	    subview = HIViewGetNextView(subview);
	}

	view = HIViewGetNextView(view);
    }

    return noErr;
}

OSStatus CommandEventHandler(EventHandlerCallRef next, EventRef event,
			     void *data)
{
    HICommandExtended command;
    WindowRef window;
    UInt32 value;

    // Get the command
    GetEventParameter(event, kEventParamDirectObject,
		      typeHICommand, NULL, sizeof(command),
		      NULL, &command);

    switch (command.attributes)
    {
	// Control
    case kHICommandFromControl:

	// Get the window
	window = HIViewGetWindow(command.source.control);

	// Get the value
	value = HIViewGetValue(command.source.control);

	// Switch on the command ID
	switch (command.commandID)
	{
	    // Zoom
	case kCommandZoom:
	    spectrum.zoom = !spectrum.zoom;
	    HIViewSetValue(check.zoom, spectrum.zoom);
	    HIViewSetNeedsDisplay(spectrum.view, true);

	    CFPreferencesSetAppValue(CFSTR("Zoom"), spectrum.zoom?
				     kCFBooleanTrue: kCFBooleanFalse,
				     kCFPreferencesCurrentApplication);
	    break;

	    // Strobe
	case kCommandStrobe:
	    strobe.enable = !strobe.enable;
	    HIViewSetValue(check.strobe, strobe.enable);
	    HIViewSetNeedsDisplay(strobe.view, true);

	    CFPreferencesSetAppValue(CFSTR("Strobe"), strobe.enable?
				     kCFBooleanTrue: kCFBooleanFalse,
				     kCFPreferencesCurrentApplication);
	    break;

	    // Filter
	case kCommandFilter:
	    audio.filter = !audio.filter;
	    HIViewSetValue(check.filter, audio.filter);
	    HIViewSetNeedsDisplay(scope.view, true);

	    CFPreferencesSetAppValue(CFSTR("Filter"), audio.filter?
				     kCFBooleanTrue: kCFBooleanFalse,
				     kCFPreferencesCurrentApplication);
	    break;

	    // Downsample
	case kCommandDownsample:
	    audio.downsample = !audio.downsample;
	    HIViewSetValue(check.downsample, audio.downsample);
	    HIViewSetNeedsDisplay(scope.view, true);

	    CFPreferencesSetAppValue(CFSTR("Downsample"), audio.downsample?
				     kCFBooleanTrue: kCFBooleanFalse,
				     kCFPreferencesCurrentApplication);
	    break;

	    // Lock
	case kCommandLock:
	    display.lock = !display.lock;
	    HIViewSetValue(check.lock, display.lock);
	    HIViewSetNeedsDisplay(display.view, true);
	    break;

	    // Multiple
	case kCommandMultiple:
	    display.multiple = !display.multiple;
	    HIViewSetValue(check.multiple, display.multiple);
	    HIViewSetNeedsDisplay(display.view, true);
	    break;

	    // Preferences button
	case kHICommandPreferences:
	    DisplayPreferences(event, data);
	    break;

	    // Close
	    // case kHICommandClose:

	    // Quit
	case kHICommandQuit:

	    // Close audio unit
	    AudioOutputUnitStop(audio.output);
	    AudioUnitUninitialize(audio.output);

	    // Flush preferences
	    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);

	    // Let the default handler handle it
	default:
	    return eventNotHandledErr;
	}

	ClearKeyboardFocus(window);
	break;

    case kHICommandFromMenu:

	// Switch on the command ID
	switch (command.commandID)
	{
	    // Zoom
	case kCommandZoom:
	    spectrum.zoom = !spectrum.zoom;
	    HIViewSetValue(check.zoom, spectrum.zoom);
	    HIViewSetNeedsDisplay(spectrum.view, true);

	    CFPreferencesSetAppValue(CFSTR("Zoom"), spectrum.zoom?
				     kCFBooleanTrue: kCFBooleanFalse,
				     kCFPreferencesCurrentApplication);
	    break;

	    // Strobe
	case kCommandStrobe:
	    strobe.enable = !strobe.enable;
	    HIViewSetValue(check.strobe, strobe.enable);
	    HIViewSetNeedsDisplay(strobe.view, true);
	    CFPreferencesSetAppValue(CFSTR("Strobe"), strobe.enable?
				     kCFBooleanTrue: kCFBooleanFalse,
				     kCFPreferencesCurrentApplication);
	    break;

	    // Filter
	case kCommandFilter:
	    audio.filter = !audio.filter;
	    HIViewSetValue(check.filter, audio.filter);
	    HIViewSetNeedsDisplay(scope.view, true);

	    CFPreferencesSetAppValue(CFSTR("Filter"), audio.filter?
				     kCFBooleanTrue: kCFBooleanFalse,
				     kCFPreferencesCurrentApplication);
	    break;

	    // Downsample
	case kCommandDownsample:
	    audio.downsample = !audio.downsample;
	    HIViewSetValue(check.downsample, audio.downsample);
	    HIViewSetNeedsDisplay(scope.view, true);

	    CFPreferencesSetAppValue(CFSTR("Downsample"), audio.downsample?
				     kCFBooleanTrue: kCFBooleanFalse,
				     kCFPreferencesCurrentApplication);
	    break;

	    // Lock
	case kCommandLock:
	    display.lock = !display.lock;
	    HIViewSetValue(check.lock, display.lock);
	    HIViewSetNeedsDisplay(display.view, true);
	    break;

	    // Multiple
	case kCommandMultiple:
	    display.multiple = !display.multiple;
	    HIViewSetValue(check.multiple, display.multiple);
	    HIViewSetNeedsDisplay(display.view, true);
	    break;

	    // Preferences
	case kHICommandPreferences:
	    DisplayPreferences(event, data);
	    break;

	    // Quit
	case kHICommandQuit:

	    // Close audio unit
	    AudioOutputUnitStop(audio.output);
	    AudioUnitUninitialize(audio.output);

	    // Flush preferences
	    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);

	    // Let the default handler handle it
	default:
	    return eventNotHandledErr;

	}
    }

    return noErr;
}

OSStatus DisplayPreferences(EventRef event, void *data)
{
    static WindowRef window;

    HIViewRef content;
    HIViewRef button;
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
    Rect bounds = {0, 0, 178, 320};

    // Create window
    CreateNewWindow(kDocumentWindowClass,
		    kWindowStandardFloatingAttributes |
		    kWindowStandardHandlerAttribute |
		    kWindowCompositingAttribute,
		    &bounds, &window);

    // Set the title
    SetWindowTitleWithCFString(window, CFSTR("Tuner Preferences"));

    // Show and position the window
    ShowWindow(window);
    RepositionWindow(window, parent,
		     kWindowAlertPositionOnParentWindowScreen);

    // Find the window content
    HIViewFindByID(HIViewGetRoot(window),
		   kHIViewWindowContentID,
		   &content);

    // Group box bounds
    bounds.bottom = 138;
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

    // Set help tag

    HMHelpContentRec help =
	{kMacHelpVersion,
	 {0, 0, 0, 0},
	 kHMOutsideBottomLeftAligned,
	 {{kHMCFStringContent,
	   CFSTR("Zoom spectrum, click to change")},
	  {kHMNoContent, NULL}}};

    HMSetControlHelpContent(check.zoom, &help);

    // Place in group box
    HIViewAddSubview(group, check.zoom);
    HIViewPlaceInSuperviewAt(check.zoom, 16, 16);

    // Create  check box
    CreateCheckBoxControl(window, &bounds, CFSTR("Display strobe"),
			  strobe.enable, true, &check.strobe);
    // Set command ID
    HIViewSetCommandID(check.strobe, kCommandStrobe);

    // Set help tag
    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Display strobe, click to change");
    HMSetControlHelpContent(check.strobe, &help);

    // Place in group box
    HIViewAddSubview(group, check.strobe);
    HIViewPlaceInSuperviewAt(check.strobe, 140, 16);

    // Create  check box
    CreateCheckBoxControl(window, &bounds, CFSTR("Audio filter"),
			  audio.filter, true, &check.filter);
    // Set command ID
    HIViewSetCommandID(check.filter, kCommandFilter);

    // Set help tag
    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Audio filter, click to change");
    HMSetControlHelpContent(check.filter, &help);

    // Place in group box
    HIViewAddSubview(group, check.filter);
    HIViewPlaceInSuperviewAt(check.filter, 16, 40);

    // Create  check box
    CreateCheckBoxControl(window, &bounds, CFSTR("Downsample"),
			  audio.downsample, true, &check.downsample);
    // Set command ID
    HIViewSetCommandID(check.downsample, kCommandDownsample);

    // Set help tag
    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Downsample, click to change");
    HMSetControlHelpContent(check.downsample, &help);

    // Place in group box
    HIViewAddSubview(group, check.downsample);
    HIViewPlaceInSuperviewAt(check.downsample, 140, 40);

    // Create  check box
    CreateCheckBoxControl(window, &bounds, CFSTR("Lock display"),
			  display.lock, true, &check.lock);
    // Set command ID
    HIViewSetCommandID(check.lock, kCommandLock);

    // Set help tag
    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Lock display, click to change");
    HMSetControlHelpContent(check.lock, &help);

    // Place in group box
    HIViewAddSubview(group, check.lock);
    HIViewPlaceInSuperviewAt(check.lock, 140, 64);

    // Create  check box
    CreateCheckBoxControl(window, &bounds, CFSTR("Multiple notes"),
			  display.multiple, true, &check.multiple);
    // Set command ID
    HIViewSetCommandID(check.multiple, kCommandMultiple);

    // Set help tag
    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Multiple notes, click to change");
    HMSetControlHelpContent(check.multiple, &help);

    // Place in group box
    HIViewAddSubview(group, check.multiple);
    HIViewPlaceInSuperviewAt(check.multiple, 16, 64);

    // Text bounds
    bounds.bottom = 16;
    bounds.right  = 68;

    // Create static text
    CreateStaticTextControl(window, &bounds,
			    CFSTR("Reference:"),
			    NULL, &text);

    // Place in group box
    HIViewAddSubview(group, text);
    HIViewPlaceInSuperviewAt(text, 16, 103);

    // Edit bounds
    bounds.bottom = 16;
    bounds.right  = 48;

    // Create edit control
    CFStringRef string =
	CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
				 CFSTR("%5.2lf"), audio.reference);

    CreateEditUnicodeTextControl(window, &bounds, string,
				 false, NULL, &legend.preferences.reference);
    CFRelease(string);

    // Focus event type spec
    EventTypeSpec focusEvents[] =
	{kEventClassControl, kEventControlSetFocusPart};

    // Install event handlers
    InstallControlEventHandler(legend.preferences.reference,
			       NewEventHandlerUPP(FocusEventHandler),
			       Length(focusEvents), focusEvents,
			       NULL, NULL);
    // Set control data
    bool single = true;

    SetControlData(legend.preferences.reference,
		   kControlEditTextPart,
		   kControlEditTextSingleLineTag,
		   sizeof(single), &single);

    // Set command ID
    HIViewSetCommandID(legend.preferences.reference, kCommandReference);

    // Set help tag
    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Reference, click to change");
    HMSetControlHelpContent(legend.preferences.reference, &help);

    // Place in group box
    HIViewAddSubview(group, legend.preferences.reference);
    HIViewPlaceInSuperviewAt(legend.preferences.reference, 95, 104);

    // Arrows bounds
    bounds.bottom = 22;
    bounds.right  = 13;

    CreateLittleArrowsControl(window, &bounds,
			      audio.reference * 10, kReferenceMin,
			      kReferenceMax, kReferenceStep,
			      &arrow.reference);
    //Set action proc
    SetControlAction(arrow.reference, ReferenceActionProc);

    // Set help tag
    help.content[kHMMinimumContentIndex].u.tagCFString =
	CFSTR("Reference, click to change");
    HMSetControlHelpContent(arrow.reference, &help);

    // Place in group box
    HIViewAddSubview(group, arrow.reference);
    HIViewPlaceInSuperviewAt(arrow.reference, 154, 101);

    // Button bounds
    bounds.bottom = 20;
    bounds.right  = 72;

    // Create push button
    CreatePushButtonControl(window, &bounds, CFSTR("Close"), &button);

    // Set command ID
    HIViewSetCommandID(button, kHICommandClose); 

    // Place in group box
    HIViewAddSubview(group, button);
    HIViewPlaceInSuperviewAt(button, 192, 102);

    return noErr;
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
    float value = CFStringGetDoubleValue(text);

    CFRelease(text);

    switch (id)
    {
    case kCommandReference:
	audio.reference = value;
	HIViewSetValue(arrow.reference, value * 10);
	HIViewSetNeedsDisplay(display.view, true);
	break;
    }

    return eventNotHandledErr;
}

// Mouse event handler
OSStatus MouseEventHandler(EventHandlerCallRef next, EventRef event,
			   void *data)
{
    HIPoint location;
    WindowRef window;
    EventMouseButton button;

    // Get button
    GetEventParameter(event, kEventParamMouseButton,
		      typeMouseButton, NULL, sizeof(button),
		      NULL, &button);

    switch (button)
    {
	// Secondary button
    case kEventMouseButtonSecondary:

	// Get location
	GetEventParameter(event, kEventParamMouseLocation,
			  typeHIPoint, NULL, sizeof(location),
			  NULL, &location);

	// Display menu
	DisplayPopupMenu(event, location, data);
	break;

	// Primary button
    case kEventMouseButtonPrimary:

	// Get window
	GetEventParameter(event, kEventParamWindowRef,
			  typeWindowRef, NULL, sizeof(window),
			  NULL, &window);
	HIViewRef view;
	HIViewKind kind;

	// Get view and kind
	HIViewGetViewForMouseEvent(HIViewGetRoot(window), event, &view);
	HIViewGetKind(view, &kind);

	switch (kind.kind)
	{
	    // User pane
	case kControlKindUserPane:
	    PostCommandEvent(view);
	    break;

	default:
	    return eventNotHandledErr;
	}

    default:
	return eventNotHandledErr;
    }

    return noErr;
}

// Post command event
OSStatus PostCommandEvent(HIViewRef view)
{
    UInt32 id;

    // Get command id
    HIViewGetCommandID(view, &id);

    HICommandExtended command =
	{kHICommandFromControl,
	 id, {view}};

    // Process command
    ProcessHICommand((HICommand *)&command);

    return noErr;
}

// Text event handler
OSStatus TextEventHandler(EventHandlerCallRef next, EventRef event,
			  void *data)
{
    char s[4];
    UInt32 size;

    GetEventParameter(event, kEventParamTextInputSendText,
		      typeUnicodeText, NULL, sizeof(s),
		      &size, &s);
    switch (*s)
    {
	// Copy display
    case 'C':
    case 'c':
    case 0x3:
	CopyDisplay(event);
	break;

	// Downsample
    case 'D':
    case 'd':
	audio.downsample = !audio.downsample;
	HIViewSetValue(check.downsample, audio.downsample);
	HIViewSetNeedsDisplay(scope.view, true);

	CFPreferencesSetAppValue(CFSTR("Downsample"), audio.downsample?
				 kCFBooleanTrue: kCFBooleanFalse,
				 kCFPreferencesCurrentApplication);
	break;

	// Filter
    case 'F':
    case 'f':
	audio.filter = !audio.filter;
	HIViewSetValue(check.filter, audio.filter);
	HIViewSetNeedsDisplay(scope.view, true);

	CFPreferencesSetAppValue(CFSTR("Filter"), audio.filter?
				 kCFBooleanTrue: kCFBooleanFalse,
				 kCFPreferencesCurrentApplication);
	break;

#ifdef DEBUG

	// Copy info
    case 'I':
    case 'i':
	CopyInfo(event);
	break;

#endif
	// Colours
    case 'K':
    case 'k':
	strobe.colours++;

	if (strobe.colours > kColourMagenta)
	    strobe.colours = 0;

	strobe.changed = true;

	CFNumberRef colours =
	    CFNumberCreate(kCFAllocatorDefault,
			   kCFNumberCFIndexType,
			   &strobe.colours);

	CFPreferencesSetAppValue(CFSTR("Colours"), colours,
				 kCFPreferencesCurrentApplication);
	CFRelease(colours);
	break;

	// Lock
    case 'L':
    case 'l':
	display.lock = !display.lock;
	HIViewSetValue(check.lock, display.lock);
	HIViewSetNeedsDisplay(display.view, true);
	break;

	// Preferences
    case 'P':
    case 'p':
	DisplayPreferences(event, data);
	break;

	// Strobe
    case 'S':
    case 's':
	strobe.enable = !strobe.enable;
	HIViewSetValue(check.strobe, strobe.enable);
	HIViewSetNeedsDisplay(strobe.view, true);

	CFPreferencesSetAppValue(CFSTR("Strobe"), strobe.enable?
				 kCFBooleanTrue: kCFBooleanFalse,
				 kCFPreferencesCurrentApplication);
	break;

	// Multiple
    case 'M':
    case 'm':
	display.multiple = !display.multiple;
	HIViewSetValue(check.multiple, display.multiple);
	HIViewSetNeedsDisplay(display.view, true);
	break;

	// Zoom
    case 'Z':
    case 'z':
	spectrum.zoom = !spectrum.zoom;
	HIViewSetValue(check.zoom, spectrum.zoom);
	HIViewSetNeedsDisplay(spectrum.view, true);

	CFPreferencesSetAppValue(CFSTR("Zoom"), spectrum.zoom?
				 kCFBooleanTrue: kCFBooleanFalse,
				 kCFPreferencesCurrentApplication);
	break;

	// Expand
    case '+':
	if (spectrum.expand < 16)
	    spectrum.expand *= 2;

	break;

	// Contract
    case '-':
	if (spectrum.expand > 1)
	    spectrum.expand /= 2;

	break;

    default:
	return eventNotHandledErr;
    }

    return noErr;
}

// Display context menu

OSStatus DisplayPopupMenu(EventRef event, HIPoint location, void *data)
{
    MenuRef menu;
    MenuItemIndex item;

    // Create menu
    CreateNewMenu(0, 0, &menu);

    // Zoom
    AppendMenuItemTextWithCFString(menu, CFSTR("Zoom spectrum"),
				   0, kCommandZoom, &item);
    CheckMenuItem(menu, item, spectrum.zoom);

    // Strobe
    AppendMenuItemTextWithCFString(menu, CFSTR("Display strobe"),
				   0, kCommandStrobe, &item);
    CheckMenuItem(menu, item, strobe.enable);

    // Filter
    AppendMenuItemTextWithCFString(menu, CFSTR("Audio filter"),
				   0, kCommandFilter, &item);
    CheckMenuItem(menu, item, audio.filter);

    // Downsample
    AppendMenuItemTextWithCFString(menu, CFSTR("Downsample"),
				   0, kCommandDownsample, &item);
    CheckMenuItem(menu, item, audio.downsample);

    // Lock
    AppendMenuItemTextWithCFString(menu, CFSTR("Lock display"),
				   0, kCommandLock, &item);
    CheckMenuItem(menu, item, display.lock);

    // Multiple
    AppendMenuItemTextWithCFString(menu, CFSTR("Multiple notes"),
				   0, kCommandMultiple, &item);
    CheckMenuItem(menu, item, display.multiple);

    AppendMenuItemTextWithCFString(menu, NULL,
				   kMenuItemAttrSeparator, 0, NULL);

    // Preferences
    AppendMenuItemTextWithCFString(menu, CFSTR("Preferences..."),
				   0, kHICommandPreferences, &item);

    AppendMenuItemTextWithCFString(menu, NULL,
				   kMenuItemAttrSeparator, 0, NULL);

    // Quit
    AppendMenuItemTextWithCFString(menu, CFSTR("Quit"),
				   0, kHICommandQuit, &item);

    PopUpMenuSelect(menu, location.y, location.x, 0);
    DisposeMenu(menu);

    return noErr;
}

// Copy display

OSStatus CopyDisplay(EventRef event)
{
    static char *notes[] =
	{"C", "C#", "D", "Eb", "E", "F",
	 "F#", "G", "Ab", "A", "Bb", "B"};

    char s[64];
    char *text = malloc(4096);

    // Check if multiple
    if (display.multiple && display.count > 0)
    {
	// For each set of values
	for (int i = 0; i < display.count; i++)
	{
	    float f = display.maxima[i].f;

	    // Reference freq
	    float fr = display.maxima[i].fr;

	    int n = display.maxima[i].n;

	    if (n < 0)
		n = 0;

	    float c = -12.0 * log2f(fr / f);

	    // Ignore silly values
	    if (!isfinite(c))
		continue;

	    // Print the text
	    sprintf(s, "%s%d\t%+6.2lf\t%9.2lf\t%9.2lf\t%+8.2lf\r\n",
		    notes[n % Length(notes)], n / 12,
		    c * 100.0, fr, f, f - fr);

	    // Copy to the memory
	    if (i == 0)
		strcpy(text, s);

	    else
		strcat(text, s);
	}
    }

    else
    {
	// Print the values
	sprintf(s, "%s%d\t%+6.2lf\t%9.2lf\t%9.2lf\t%+8.2lf\r\n",
		notes[display.n % Length(notes)], display.n / 12,
		display.c * 100.0, display.fr, display.f,
		display.f - display.fr);

	// Copy to the memory
	strcpy(text, s);
    }

    // Create a pasteboard
    PasteboardRef paste;

    PasteboardCreate(kPasteboardClipboard, &paste);
    PasteboardClear(paste);

    CFDataRef data = CFDataCreate(kCFAllocatorDefault,
				  (UInt8 *)text, strlen(text));

    PasteboardPutItemFlavor(paste, (PasteboardItemID)data,
			    kUTTypeUTF8PlainText, data,
			    kPasteboardFlavorNoFlags);
    free(text);
    CFRelease(data);
    CFRelease(paste);

    return noErr;
}

// Reference action proc
void ReferenceActionProc(HIViewRef view, ControlPartCode part)
{
    // Get value
    SInt32 value = HIViewGetValue(view);
    SInt32 step;

    // Get step
    GetControlData(view, 0, kControlLittleArrowsIncrementValueTag,
		   sizeof(step), &step, nil);

    // Check direction
    switch (part)
    {
    case kControlUpButtonPart:
	value += step;
	break;

    case kControlDownButtonPart:
	value -= step;
	break;
    }

    // Set the value
    HIViewSetValue(view, value);

    // Update the display
    audio.reference = (float)value / 10.0;
    HIViewSetNeedsDisplay(display.view, true);

    // Update the edit text
    CFStringRef text =
	CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
				 CFSTR("%6.2lf"),
				 audio.reference);

    HIViewSetText(legend.preferences.reference, text);
    CFRelease(text);

    // Update the preferences
    CFNumberRef index =
	CFNumberCreate(kCFAllocatorDefault,
		       kCFNumberCFIndexType,
		       &value);

    CFPreferencesSetAppValue(CFSTR("Reference"), index,
			     kCFPreferencesCurrentApplication);
    CFRelease(index);
}
