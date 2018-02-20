//
//  Audio.h
//  Tuner
//
//  Created by Bill Farmer on 18/02/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
//

#include <AppKit/AppKit.h>
#include <Foundation/Foundation.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>

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

// Event value
enum
    {kEventAudioUpdate = 1};

typedef struct
{
    AudioUnit output;
    AudioDeviceID id;
    int divisor;
    int frames;
    float *buffer;
    float sample;
    Boolean filter;
} Audio;

OSStatus SetupAudio(void);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);
char *AudioUnitErrString(OSStatus status);
