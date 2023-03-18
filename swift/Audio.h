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

#undef SWIFT_NORETURN

#include "Tuner-Swift.h"

// Macros
#define Length(a) (sizeof(a) / sizeof(a[0]))

// Constants
#define kMin        0.5
#define kScale   2048.0
#define kTimerDelay 0.1

// Audio in values
enum
    {kSampleRate       = 44100,
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
     kAOffset     = 9,
     kOctave      = 12,
     kEqual       = 8};

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

// Expand and colour tags
enum
    {kExpand = 'Expd',
     kColour = 'Colr'};

// Reference tags
enum
    {kRefText = 'RefT',
     kRefStep = 'RefS'};

// Transpose and temperament tags
enum
    {kTrans = 'Tran',
     kTemp  = 'Temp',
     kKey   = 'Key '};

// Accidentals
enum
    {kNatural = 'Nat ',
     kSharp   = 'Shrp',
     kFlat    = 'Flat'};

// Maximum
typedef struct
{
    double f;
    double fr;
    int n;
} maximum;

ScopeView *scopeView;
SpectrumView *spectrumView;
DisplayView *displayView;
StrobeView *strobeView;
StaffView *staffView;
MeterView *meterView;

// Scope data
typedef struct
{
    int length;
    double *data;
} Scope;
Scope scope;

// Spectrum data
typedef struct
{
    int length;
    int count;
    double *data;
    double *values;
} Spectrum;
Spectrum spectrum;

// Display data
typedef struct
{
    int count;
    maximum *maxima;
} Display;
Display disp;

// Temperaments data
#include "Temperaments.h"

// Audio data
typedef struct
{
    AudioUnit output;
    AudioDeviceID id;
    int frames;
    int temper;
    int key;
    double *buffer;
    double sample;
    double reference;
    bool fund;
    bool filt;
    bool note;
    bool down;
} Audio;
Audio audio;

// Audio filter
typedef struct
{
    bool note[12];
    bool octave[9];
} Filters;
Filters filters;

// Functions
OSStatus SetupAudio(void);
OSStatus ShutdownAudio(void);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);
char *AudioUnitErrString(OSStatus);
void (^ProcessAudio)(void);

// Boolean array access functions
bool getNote(int);
void setNote(bool, int);
bool getOctave(int);
void setOctave(bool, int);

#endif /* Audio_h */
