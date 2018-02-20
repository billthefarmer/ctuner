//
//  Audio.c
//  Tuner
//
//  Created by Bill Farmer on 18/02/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
//

#include "Audio.h"

void NSLog(CFStringRef format, ...);

Audio audio;

// Setup audio
OSStatus SetupAudio()
{
    // Specify an output unit

    AudioComponentDescription dc =
	{kAudioUnitType_Output,
	 kAudioUnitSubType_HALOutput,
	 kAudioUnitManufacturer_Apple,
	 0, 0};

    // Find an output unit
    AudioComponent cp
	= AudioComponentFindNext(NULL, &dc);

    if (cp == NULL)
    {
        // AudioComponentFindNext
        NSLog(CFSTR("Error in AudioComponentFindNext"));
	return -1;
    }

    // Open it
    OSStatus status = AudioComponentInstanceNew(cp, &audio.output);
    if (status != noErr)
    {
        // AudioComponentInstanceNew
        NSLog(CFSTR("Error in AudioComponentInstanceNew %s (%d)"),
              AudioUnitErrString(status), status);
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
        // AudioUnitSetProperty
        NSLog(CFSTR("Error in AudioUnitSetProperty: " 
                    "kAudioOutputUnitProperty_EnableIO %d"), status);
        return status;
    }

    // Disable output
    enable = false;
    status = AudioUnitSetProperty(audio.output,
				  kAudioOutputUnitProperty_EnableIO,
				  kAudioUnitScope_Output,
				  0, &enable, sizeof(enable));
    if (status != noErr)
        return status;

    AudioDeviceID id;
    size = sizeof(id);

    // Get the default input device
    AudioObjectPropertyAddress inputDeviceAOPA =
        {kAudioHardwarePropertyDefaultInputDevice,
         kAudioObjectPropertyScopeGlobal,
         kAudioObjectPropertyElementMaster};

    // Get device
    status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                        &inputDeviceAOPA,
                                        0, NULL, &size, &id);
    if (status != noErr)
        return status;

    // Set the audio unit device
    status = AudioUnitSetProperty(audio.output,
                                  kAudioOutputUnitProperty_CurrentDevice, 
                                  kAudioUnitScope_Global,
                                  0, &id, size);
    if (status != noErr)
        return status;

    // Get nominal sample rates size
    AudioObjectPropertyAddress audioDeviceAOPA =
        {kAudioDevicePropertyAvailableNominalSampleRates,
         kAudioObjectPropertyScopeGlobal,
         kAudioObjectPropertyElementMaster};

    status = AudioObjectGetPropertyDataSize(id, &audioDeviceAOPA,
                                            0, nil, &size);
    if (status != noErr)
        return status;

    // Get nominal sample rates
    AudioValueRange *rates = malloc(size);

    if (rates == NULL)
        return -1;

    status = AudioObjectGetPropertyData(id, &audioDeviceAOPA, 0, nil,
                                        &size, rates);
    if (status != noErr)
    {
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

    // Get sample rate
    audioDeviceAOPA.mSelector = kAudioDevicePropertyNominalSampleRate;

    Float64 nominal;
    size = sizeof(nominal);

    // Set the sample rate, if in range
    if (inrange)
	status = AudioObjectSetPropertyData(id, &audioDeviceAOPA, 0, NULL,
                                            size, &nominal);
    // Get the sample rate
    status = AudioObjectGetPropertyData(id, &audioDeviceAOPA, 0, NULL,
                                        &size, &nominal);
    if (status != noErr)
        return status;

    // Set the divisor
    audio.divisor = round(nominal / ((kSampleRate1 + kSampleRate2) / 2));

    // Set the rate
    audio.sample = nominal / audio.divisor;

    // Get the buffer size range

    AudioValueRange sizes;
    size = sizeof(sizes);

    // Get the buffer size range size
    audioDeviceAOPA.mSelector = kAudioDevicePropertyBufferFrameSizeRange;

    // Get the buffer size range
    status = AudioObjectGetPropertyData(id, &audioDeviceAOPA, 0, NULL,
                                        &size, &sizes);
    if (status != noErr)
        return status;

    NSLog(CFSTR("Frames %f, %f\n"), sizes.mMinimum, sizes.mMaximum);

    UInt32 frames = kStep * audio.divisor;
    size = sizeof(frames);

    while (!((sizes.mMaximum >= frames) &&
	     (sizes.mMinimum <= frames)))
	frames /= 2;

    NSLog(CFSTR("Frames %d\n"), frames);

    // Set the max frames
    status = AudioUnitSetProperty(audio.output,
				  kAudioUnitProperty_MaximumFramesPerSlice,
				  kAudioUnitScope_Global, 0,
				  &frames, sizeof(frames));
    if (status != noErr)
        return status;

    // Get the buffer frame size
    audioDeviceAOPA.mSelector = kAudioDevicePropertyBufferFrameSize;

    // Set the buffer frame size
    AudioObjectSetPropertyData(id, &audioDeviceAOPA, 0, NULL,
                               size, &frames);
    if (status != noErr)
        return status;

    audio.frames = frames;

    // Get the frames
    status = AudioUnitGetProperty(audio.output,
				  kAudioUnitProperty_MaximumFramesPerSlice,
				  kAudioUnitScope_Global, 0, &frames, &size);
    if (status != noErr)
        return status;

    audio.frames = frames;

    AudioStreamBasicDescription format;
    size = sizeof(format);

    // Get stream format
    status = AudioUnitGetProperty(audio.output,
				  kAudioUnitProperty_StreamFormat,
				  kAudioUnitScope_Input, 1,
				  &format, &size);
    if (status != noErr)
	return status;

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
	return status;

    AURenderCallbackStruct input =
	{InputProc, &audio.output};

    // Set callback
    status = AudioUnitSetProperty(audio.output,
				  kAudioOutputUnitProperty_SetInputCallback,
				  kAudioUnitScope_Global, 0,
				  &input, sizeof(input));
    if (status != noErr)
	return status;

    // Init the audio unit
    status = AudioUnitInitialize(audio.output);

    if (status != noErr)
	return status;

    // Start the audio unit
    status = AudioOutputUnitStart(audio.output);

    if (status != noErr)
	return status;

    return status;
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

    // static Boolean rendered = false;
    // if (!rendered)
    // {
    //     NSLog(CFSTR("Rendered, frames %d\n"), inNumberFrames);
    //     rendered = true;
    // }

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

    // Send an event to post to the main event queue
    // SendEvent();

    return noErr;
}

CFStringRef AudioUnitErrString(OSStatus status)
{
    static UInt32 audioUnitErrCodes[] =
        {kAudioUnitErr_CannotDoInCurrentContext,
         kAudioUnitErr_FailedInitialization,
         kAudioUnitErr_FileNotSpecified,
         kAudioUnitErr_FormatNotSupported,
         kAudioUnitErr_Initialized,
         kAudioUnitErr_InvalidElement,
         kAudioUnitErr_InvalidFile,
         kAudioUnitErr_InvalidOfflineRender,
         kAudioUnitErr_InvalidParameter,
         kAudioUnitErr_InvalidProperty,
         kAudioUnitErr_InvalidPropertyValue,
         kAudioUnitErr_InvalidScope,
         kAudioUnitErr_NoConnection,
         kAudioUnitErr_PropertyNotInUse,
         kAudioUnitErr_PropertyNotWritable,
         kAudioUnitErr_TooManyFramesToProcess,
         kAudioUnitErr_Unauthorized,
         kAudioUnitErr_Uninitialized,
         kAudioUnitErr_UnknownFileType,
         kAudioUnitErr_RenderTimeout};

    static CFStringRef audioUnitErrStrings[] =
        {CFSTR("AudioUnitErr_CannotDoInCurrentContext"),
         CFSTR("AudioUnitErr_FailedInitialization"),
         CFSTR("AudioUnitErr_FileNotSpecified"),
         CFSTR("AudioUnitErr_FormatNotSupported"),
         CFSTR("AudioUnitErr_Initialized"),
         CFSTR("AudioUnitErr_InvalidElement"),
         CFSTR("AudioUnitErr_InvalidFile"),
         CFSTR("AudioUnitErr_InvalidOfflineRender"),
         CFSTR("AudioUnitErr_InvalidParameter"),
         CFSTR("AudioUnitErr_InvalidProperty"),
         CFSTR("AudioUnitErr_InvalidPropertyValue"),
         CFSTR("AudioUnitErr_InvalidScope"),
         CFSTR("AudioUnitErr_NoConnection"),
         CFSTR("AudioUnitErr_PropertyNotInUse"),
         CFSTR("AudioUnitErr_PropertyNotWritable"),
         CFSTR("AudioUnitErr_TooManyFramesToProcess"),
         CFSTR("AudioUnitErr_Unauthorized"),
         CFSTR("AudioUnitErr_Uninitialized"),
         CFSTR("AudioUnitErr_UnknownFileType"),
         CFSTR("AudioUnitErr_RenderTimeout")};

    for (int i = 0; i < sizeof(audioUnitErrCodes) / sizeof(UInt32); i++)
        if (audioUnitErrCodes[i] == status)
            return audioUnitErrStrings[i];

    return CFSTR("");
}

