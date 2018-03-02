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
     kRange = kSamples * 3 / 8,
     kStep = kSamples / kOversample};

// Tuner reference values
enum
    {kA5Reference = 440,
     kC5Offset    = 57,
     kOctave      = 12};

// Event value
enum
    {kEventAudioUpdate = 1,
     kEventAudioRate   = 2};

// Maximum
typedef struct
{
    float f;
    float fr;
    int n;
} maximum;

// App
AppDelegate *app;

// Scope data
typedef struct
{
    float *data;
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
    float f;
    float r;
    float l;
    float h;
    float *data;
    float *values;
} SpectrumData;
SpectrumData spectrumData;
SpectrumView *spectrumView;

// Display data
typedef struct
{
    maximum *maxima;
    float f;
    float fr;
    float c;
    bool lock;
    bool zoom;
    bool multiple;
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
    float c;
} StrobeData;
StrobeData strobeData;
StrobeView *strobeView;

// Meter data
typedef struct
{
    float c;
} MeterData;
MeterData meterData;
MeterView *meterView;

// Status data
typedef struct
{
    float sample;
    float actual;
} StatusData;
StatusData statusData;
StatusView *statusView;

// Audio data
typedef struct
{
    AudioUnit output;
    AudioDeviceID id;
    int divisor;
    int frames;
    float *buffer;
    float sample;
    float reference;
    pthread_mutex_t mutex;
    bool filter;
    bool downsample;
} AudioData;
AudioData audioData;

// Audio filter
typedef struct
{
    bool note[12];
    bool octave[8];
} Filter;
// Filter filter =
//     {{true, true, true, true,
//       true, true, true, true,
//       true, true, true, true},
//      {true, true, true, true,
//       true, true, true, true}};

// Functions
OSStatus SetupAudio(void);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);
char *AudioUnitErrString(OSStatus status);
void (^ProcessAudio)();
