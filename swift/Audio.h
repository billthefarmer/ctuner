//
//  Audio.h
//  Tuner
//
//  Created by Bill Farmer on 18/02/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef Audio_h
#define Audio_h

#include <pthread.h>
#include <Cocoa/Cocoa.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <Accelerate/Accelerate.h>

#include "Tuner-Swift.h"

// Macros
#define Length(a) (sizeof(a) / sizeof(a[0]))

// Constants
#define kMin        0.5
#define kScale   2048.0
#define kTimerDelay 0.1

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
     kRange = kSamples * 7 / 16,
     kStep = kSamples / kOversample};

// Tuner reference values
enum
    {kA5Reference = 440,
     kC5Offset    = 57,
     kOctave      = 12};

// Checkbox tags
enum
    {kZoom   = 'Zoom',
     kFilt   = 'Filt',
     kMult   = 'Mult',
     kFund   = 'Fund',
     kStrobe = 'Strb',
     kDown   = 'Down',
     kLock   = 'Lock',
     kNote   = 'Note'};

// Reference tags
enum
    {kRefText = 'RefT',
     kRefStep = 'RefS'};

// Maximum
typedef struct
{
    double f;
    double fr;
    int n;
} maximum;

// App
// AppDelegate *app;

// Scope data
typedef struct
{
    double *data;
    int length;
} ScopeData;
ScopeData scopeData;
ScopeView *scopeView;

// Spectrum data
typedef struct
{
    int length;
    int expand;
    int count;
    bool zoom;
    double f;
    double r;
    double l;
    double h;
    double *data;
    double *values;
} SpectrumData;
SpectrumData spectrumData;
SpectrumView *spectrumView;

// Display data
typedef struct
{
    maximum *maxima;
    double f;
    double fr;
    double c;
    bool lock;
    bool zoom;
    bool mult;
    int count;
    int n;
} DisplayData;
DisplayData displayData;
DisplayView *displayView;

// Strobe data
typedef struct
{
    bool changed;
    bool enable;
    int colours;
    double c;
} StrobeData;
StrobeData strobeData;
StrobeView *strobeView;

// Meter data
typedef struct
{
    double c;
} MeterData;
MeterData meterData;
MeterView *meterView;

// Audio data
typedef struct
{
    AudioUnit output;
    AudioDeviceID id;
    int divisor;
    int frames;
    double *buffer;
    double sample;
    double reference;
    bool fund;
    bool filt;
    bool note;
    bool down;
} AudioData;
AudioData audioData;

// Audio filter
typedef struct
{
    bool note[12];
    bool octave[9];
} FilterData;
FilterData filterData;

// Functions
OSStatus SetupAudio(void);
OSStatus ShutdownAudio(void);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);
char *AudioUnitErrString(OSStatus);
void (^ProcessAudio)();

// Boolean array access functions
bool getNote(int);
void setNote(bool, int);
bool getOctave(int);
void setOctave(bool, int);

#endif /* Audio_h */
