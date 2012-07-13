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
//  Bill Farmer  william j farmer [at] yahoo [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

#include <Carbon/Carbon.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <Accelerate/Accelerate.h>

// Macros

#define Length(a) (sizeof(a) / sizeof(a[0]))

#define kMin        0.5
#define kScale   2048.0
#define kTimerDelay 0.1

#define DEBUG

// Audio in values

enum
    {kSampleRate1      = 11025,
     kSampleRate2      = 12000,
     kBytesPerPacket   = 4,
     kBytesPerFrame    = 4,
     kChannelsPerFrame = 1};

// Audio processing values

enum
    {kOversample = 16,
     kSamples = 16384,
     kLog2Samples = 14,
     kSamples2 = kSamples / 2,
     kMaxima = 8,
     kFrames = 512,
     kRange = kSamples * 3 / 8,
     kStep = kSamples / kOversample};

// Tuner reference values

enum
    {kA5Reference = 440,
     kC5Offset    = 57,
     kOctave      = 12};

// Slider values

enum
    {kMeterMax   = 200,
     kMeterValue = 100,
     kMeterMin   = 0};

// Arrows values

enum
    {kReferenceMax   = 4500,
     kReferenceValue = 4400,
     kReferenceMin   = 4300,
     kReferenceStep  = 1};

// Command IDs

enum
    {kCommandReference  = 'Rfnc',
     kCommandStrobe     = 'Strb',
     kCommandMultiple   = 'Mult',
     kCommandZoom       = 'Zoom',
     kCommandLock       = 'Lock',
     kCommandFilter     = 'Fltr'};

// Audio event constants

enum
    {kEventAudioUpdate = 'Updt',
     kEventAudioRate   = 'Rate'};

// Structs

typedef struct
{
    float f;
    float fr;
    int n;
} maximum;

typedef struct
{
    HIViewRef view;
    float *data;
    int length;
} Scope;

typedef struct
{
    HIViewRef view;
    int length;
    int count;
    bool zoom;
    float f;
    float r;
    float l;
    float h;
    float *data;
    float *values;
} Spectrum;

typedef struct
{
    HIViewRef view;
    EventLoopTimerRef timer;
    maximum *maxima;
    float f;
    float fr;
    float c;
    bool lock;
    bool zoom;
    bool multiple;
    int count;
    int n;
} Display;

typedef struct
{
    HIViewRef view;
    float c;
    bool enable;
} Strobe;

typedef struct
{
    HIViewRef view;
    HIViewRef slider; 
    float c;
} Meter;

typedef struct
{
    struct
    {
	HIViewRef sample;
	HIViewRef actual;
    } status;

    struct
    {
	HIViewRef reference;
    } preferences;
} Legend;

typedef struct
{
    HIViewRef zoom;
    HIViewRef lock;
    HIViewRef strobe;
    HIViewRef filter;
    HIViewRef multiple;
} Check;

typedef struct
{
    HIViewRef reference;
} Arrow;

typedef struct
{
    AudioUnit output;
    AudioDeviceID id;
    float *buffer;
    bool filter;
    int divisor;
    int frames;
    float sample;
    float reference;
} Audio;

// Function prototypes.

OSStatus ScopeDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus SpectrumDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus DisplayDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus StrobeDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus MeterDrawEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus WindowEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus CommandEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus AudioEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus FocusEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus MouseEventHandler(EventHandlerCallRef, EventRef, void *);
OSStatus TextEventHandler(EventHandlerCallRef, EventRef, void *);

OSStatus SetupAudio(void);
OSStatus DisplayAlert(CFStringRef, CFStringRef, OSStatus);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);

OSStatus DisplayPopupMenu(EventRef, HIPoint, void *);
OSStatus DisplayPreferences(EventRef, void *);
OSStatus PostCommandEvent(HIViewRef);
OSStatus WindowZoomed(EventRef, void *);
OSStatus CopyDisplay(EventRef);
OSStatus CopyInfo(EventRef);

OSStatus StrokeRoundRect(CGContextRef, CGRect, float);
OSStatus CentreTextAtPoint(CGContextRef, float, float, const char *, size_t);
OSStatus DrawLock(CGContextRef, int, int);
HIRect DrawEdge(CGContextRef, HIRect);

void TimerProc(EventLoopTimerRef, void *);
void ReferenceActionProc(HIViewRef, ControlPartCode);

void GetPreferences(void);
