//
//  Audio.m
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

#include "Audio.h"

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
	= AudioComponentFindNext(nil, &dc);

    if (cp == nil)
    {
        // AudioComponentFindNext
        NSLog(@"Error in AudioComponentFindNext");
	return -1;
    }

    // Open it
    OSStatus status = AudioComponentInstanceNew(cp, &audioData.output);
    if (status != noErr)
    {
        // AudioComponentInstanceNew
        NSLog(@"Error in AudioComponentInstanceNew %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    UInt32 enable;
    UInt32 size;

    // Enable input
    enable = true;
    status = AudioUnitSetProperty(audioData.output,
				  kAudioOutputUnitProperty_EnableIO,
				  kAudioUnitScope_Input,
				  1, &enable, sizeof(enable));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioOutputUnitProperty_EnableIO %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Disable output
    enable = false;
    status = AudioUnitSetProperty(audioData.output,
				  kAudioOutputUnitProperty_EnableIO,
				  kAudioUnitScope_Output,
				  0, &enable, sizeof(enable));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioOutputUnitProperty_EnableIO %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

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
                                        0, nil, &size, &id);
    if (status != noErr)
    {
        // AudioObjectGetPropertyData
        NSLog(@"Error in AudioObjectGetPropertyData: " 
                    "kAudioHardwarePropertyDefaultInputDevice %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Set the audio unit device
    status = AudioUnitSetProperty(audioData.output,
                                  kAudioOutputUnitProperty_CurrentDevice, 
                                  kAudioUnitScope_Global,
                                  0, &id, size);
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioOutputUnitProperty_CurrentDevice %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Get nominal sample rates size
    AudioObjectPropertyAddress audioDeviceAOPA =
        {kAudioDevicePropertyAvailableNominalSampleRates,
         kAudioObjectPropertyScopeGlobal,
         kAudioObjectPropertyElementMaster};

    status = AudioObjectGetPropertyDataSize(id, &audioDeviceAOPA,
                                            0, nil, &size);
    if (status != noErr)
    {
        // AudioObjectGetPropertyDataSize
        NSLog(@"Error in AudioObjectGetPropertyDataSize: " 
                    "kAudioDevicePropertyAvailableNominalSampleRates %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Get nominal sample rates
    AudioValueRange *rates = malloc(size);

    if (rates == nil)
        return -1;

    status = AudioObjectGetPropertyData(id, &audioDeviceAOPA, 0, nil,
                                        &size, rates);
    if (status != noErr)
    {
        // AudioObjectGetPropertyDataSize
        NSLog(@"Error in AudioObjectGetPropertyData: " 
                    "kAudioDevicePropertyAvailableNominalSampleRates %s (%d)",
              AudioUnitErrString(status), status);
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

    // Set the sample rate, if in range, ignore errors
    if (inrange)
	status = AudioObjectSetPropertyData(id, &audioDeviceAOPA, 0, nil,
                                            size, &nominal);
    // Get the sample rate
    status = AudioObjectGetPropertyData(id, &audioDeviceAOPA, 0, nil,
                                        &size, &nominal);
    if (status != noErr)
    {
        // AudioObjectGetPropertyData
        NSLog(@"Error in AudioObjectGetPropertyData: " 
                    "kAudioDevicePropertyNominalSampleRate %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Set the divisor
    audioData.divisor = round(nominal / ((kSampleRate1 + kSampleRate2) / 2));

    // Set the rate
    audioData.sample = nominal / audioData.divisor;

    // Get the buffer size range

    AudioValueRange sizes;
    size = sizeof(sizes);

    // Get the buffer size range size
    audioDeviceAOPA.mSelector = kAudioDevicePropertyBufferFrameSizeRange;

    // Get the buffer size range
    status = AudioObjectGetPropertyData(id, &audioDeviceAOPA, 0, nil,
                                        &size, &sizes);
    if (status != noErr)
    {
        // AudioObjectGetPropertyData
        NSLog(@"Error in AudioObjectGetPropertyData: " 
                    "kAudioDevicePropertyNominalSampleRate %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    UInt32 frames = kStep * audioData.divisor;
    size = sizeof(frames);

    while (!((sizes.mMaximum >= frames) &&
	     (sizes.mMinimum <= frames)))
	frames /= 2;

    // Set the max frames
    status = AudioUnitSetProperty(audioData.output,
				  kAudioUnitProperty_MaximumFramesPerSlice,
				  kAudioUnitScope_Global, 0,
				  &frames, sizeof(frames));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioUnitProperty_MaximumFramesPerSlice %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Get the buffer frame size
    audioDeviceAOPA.mSelector = kAudioDevicePropertyBufferFrameSize;

    // Set the buffer frame size
    AudioObjectSetPropertyData(id, &audioDeviceAOPA, 0, nil,
                               size, &frames);
    if (status != noErr)
        return status;

    audioData.frames = frames;

    // Get the frames
    status = AudioUnitGetProperty(audioData.output,
				  kAudioUnitProperty_MaximumFramesPerSlice,
				  kAudioUnitScope_Global, 0, &frames, &size);
    if (status != noErr)
    {
        // AudioUnitGetProperty
        NSLog(@"Error in AudioUnitGetProperty: " 
                    "kAudioUnitProperty_MaximumFramesPerSlice %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    audioData.frames = frames;

    AudioStreamBasicDescription format;
    size = sizeof(format);

    // Get stream format
    status = AudioUnitGetProperty(audioData.output,
				  kAudioUnitProperty_StreamFormat,
				  kAudioUnitScope_Input, 1,
				  &format, &size);
    if (status != noErr)
    {
        // AudioUnitGetProperty
        NSLog(@"Error in AudioUnitGetProperty: " 
                    "kAudioUnitProperty_StreamFormat %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Set format
    format.mSampleRate = nominal;
    format.mBytesPerPacket = kBytesPerPacket;
    format.mBytesPerFrame = kBytesPerFrame;
    format.mChannelsPerFrame = kChannelsPerFrame;

    // Set stream format
    status = AudioUnitSetProperty(audioData.output,
				  kAudioUnitProperty_StreamFormat,
				  kAudioUnitScope_Output, 1,
				  &format, sizeof(format));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioUnitProperty_StreamFormat %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Create the mutex for locking
    pthread_mutex_init(&audioData.mutex, nil);

    AURenderCallbackStruct input =
	{InputProc, &audioData.output};

    // Set callback
    status = AudioUnitSetProperty(audioData.output,
				  kAudioOutputUnitProperty_SetInputCallback,
				  kAudioUnitScope_Global, 0,
				  &input, sizeof(input));
    if (status != noErr)
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioUnitProperty_SetInputCallback %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Init the audio unit
    status = AudioUnitInitialize(audioData.output);

    if (status != noErr)
    {
        // AudioUnitInitialize
        NSLog(@"Error in AudioUnitInitialize %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Start the audio unit
    status = AudioOutputUnitStart(audioData.output);

    if (status != noErr)
    {
        // AudioOutputUnitStart
        NSLog(@"Error in AudioOutputUnitStart %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    return status;
}

// Input proc
OSStatus InputProc(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
		   const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
		   UInt32 inNumberFrames, AudioBufferList *ioData)
{
    static AudioBufferList abl =
	{1, {1, 0, nil}};

    // Initialise data structs
    static Float32 buffer[kSamples];

    if (audioData.buffer == nil)
	audioData.buffer = buffer;

    // Render data
    OSStatus status
	= AudioUnitRender(*(AudioUnit *)inRefCon, ioActionFlags,
			  inTimeStamp, inBusNumber,
			  inNumberFrames, &abl);
    if (status != noErr)
    {
        // AudioUnitRender
        static Boolean once = false;
        if (!once)
        {
            NSLog(@"Error in AudioUnitRender %s (%d)",
                  AudioUnitErrString(status), status);
            once = true;
        }
        return status;
    }

    // Copy the input data
    memmove(buffer, buffer + (audioData.frames / audioData.divisor),
	    (kSamples - (audioData.frames / audioData.divisor)) *
            sizeof(Float32));

    Float32 *data = abl.mBuffers[0].mData;

    // Lock the mutex
    pthread_mutex_lock(&audioData.mutex);

    // Butterworth filter, 3dB/octave
    for (int i = 0; i < (audioData.frames / audioData.divisor); i++)
    {
	static float G = 3.023332184e+01;
	static float K = 0.9338478249;

	static float xv[2];
	static float yv[2];

	xv[0] = xv[1];
	xv[1] = data[i * audioData.divisor] / G;

	yv[0] = yv[1];
	yv[1] = (xv[0] + xv[1]) + (K * yv[0]);

	// Choose filtered/unfiltered data
	buffer[(kSamples - (audioData.frames / audioData.divisor)) + i] =
	    audioData.filter? yv[1]: data[i * audioData.divisor];
    }

    // Unlock the mutex
    pthread_mutex_unlock(&audioData.mutex);

    // Run in main queue
    dispatch_async(dispatch_get_main_queue(), ProcessAudio);

    return noErr;
}

// Process audio
void (^ProcessAudio)() = ^
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

    // Initialise structures
    if (scopeData.data == nil)
    {
	scopeData.data = audioData.buffer + kSamples -
        (audioData.frames / audioData.divisor);
	scopeData.length = audioData.frames / audioData.divisor;

	spectrumData.data = xa;
	spectrumData.length = kRange;
	spectrumData.values = values;

	displayData.maxima = maxima;

	fps = audioData.sample / (float)kSamples;
	expect = 2.0 * M_PI * (float)(audioData.frames / audioData.divisor) /
	    (float)kSamples;

	// Init Hamming window
	vDSP_hamm_window(window, kSamples, 0);

	// Init FFT
	setup = vDSP_create_fftsetup(kLog2Samples, kFFTRadix2);
    }

    // Refresh scope
    scopeView.needsDisplay = true;

    // Maximum data value
    static float dmax;

    if (dmax < 0.125)
	dmax = 0.125;

    // Calculate normalising value
    float norm = dmax;

    // Get max magitude
    vDSP_maxmgv(audioData.buffer, 1, &dmax, kSamples);

    // Divide by normalisation
    vDSP_vsdiv(audioData.buffer, 1, &norm, input, 1, kSamples);

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
    if (audioData.downsample)
    {
	// x2 = xa << 2
	for (int i = 0; i < Length(x2); i++)
	{
	    x2[i] = 0.0;

	    for (int j = 0; j < 2; j++)
		x2[i] += xa[(i * 2) + j] / 2.0;
	}

	// x3 = xa << 3
	for (int i = 0; i < Length(x3); i++)
	{
	    x3[i] = 0.0;

	    for (int j = 0; j < 3; j++)
		x3[i] += xa[(i * 3) + j] / 3.0;
	}

	// x4 = xa << 4
	for (int i = 0; i < Length(x4); i++)
	{
	    x4[i] = 0.0;

	    for (int j = 0; j < 4; j++)
		x2[i] += xa[(i * 4) + j] / 4.0;
	}

	// x5 = xa << 5
	for (int i = 0; i < Length(x5); i++)
	{
	    x5[i] = 0.0;

	    for (int j = 0; j < 5; j++)
		x5[i] += xa[(i * 5) + j] / 5.0;
	}

	// Add downsamples
	for (int i = 0; i < Length(xa); i++)
	{
	    if (i < Length(x2))
		xa[i] += x2[i];

	    if (i < Length(x3))
		xa[i] += x3[i];

	    if (i < Length(x4))
		xa[i] += x4[i];

	    if (i < Length(x5))
		xa[i] += x5[i];

	    // Calculate differences for finding maxima
	    dxa[i] = xa[i] - xa[i - 1];

	}
    }

    // Maximum FFT output
    float  max;
    vDSP_Length imax;

    vDSP_maxmgvi(xa, 1, &max, &imax, kRange);

    float f = xf[imax];

    int count = 0;
    int limit = kRange - 1;

    // Find maximum value, and list of maxima
    for (int i = 1; i < limit; i++)
    {
	// If display not locked, find maxima and add to list

	if (!displayData.lock && count < Length(maxima) &&
	    xa[i] > kMin && xa[i] > (max / 2) &&
	    dxa[i] > 0.0 && dxa[i + 1] < 0.0)
	{
	    maxima[count].f = xf[i];

	    // Cents relative to reference
	    float cf =
		-12.0 * log2f(audioData.reference / xf[i]);

	    // Reference note
	    maxima[count].fr = audioData.reference * powf(2.0, round(cf) / 12.0);

	    // Note number
	    maxima[count].n = round(cf) + kC5Offset;

	    // Set limit to octave above
	    if (!audioData.downsample && (limit > i * 2))
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
	if (!audioData.downsample)
	    f = maxima[0].f;

	// Cents relative to reference
	float cf =
	    -12.0 * log2f(audioData.reference / f);

	// Reference note
	fr = audioData.reference * powf(2.0, round(cf) / 12.0);

	// Lower and upper freq
	fl = audioData.reference * powf(2.0, (round(cf) - 0.55) / 12.0);
	fh = audioData.reference * powf(2.0, (round(cf) + 0.55) / 12.0);

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
    if (!displayData.lock)
    {
	// Update scope window
	scopeView.needsDisplay = true;

	// Update spectrum window
	for (int i = 0; i < count; i++)
	    values[i] = maxima[i].f / fps;

	spectrumData.count = count;

	if (found)
	{
	    spectrumData.f = f  / fps;
	    spectrumData.r = fr / fps;
	    spectrumData.l = fl / fps;
	    spectrumData.h = fh / fps;
	}

	spectrumView.needsDisplay = true;
    }

    // Timer
    static long timer;

    // Found
    if (found)
    {
	// If display not locked
	if (!displayData.lock)
	{
	    // Update the display struct
	    displayData.f = f;
	    displayData.fr = fr;
	    displayData.c = c;
	    displayData.n = n;
	    displayData.count = count;

	    // Update display
            displayView.needsDisplay = true;

	    // Update meter
	    meterData.c = c;

	    // Update strobe
	    strobeData.c = c;
	}

	// Reset count;
	timer = 0;
    }

    // Not found
    else
    {
	// If display not locked
	if (!displayData.lock)
	{

	    if (timer == kTimerCount)
	    {
		displayData.f = 0.0;
		displayData.fr = 0.0;
		displayData.c = 0.0;
		displayData.n = 0;
		displayData.count = 0;

		// Update display
                displayView.needsDisplay = true;

		// Update meter
		meterData.c = 0.0;

		// Update strobe
		strobeData.c = 0.0;

		// Update spectrum
		spectrumData.f = 0.0;
		spectrumData.r = 0.0;
		spectrumData.l = 0.0;
		spectrumData.h = 0.0;
	    }
	}
    }

    timer++;
};

// AudioUnitErrString
char *AudioUnitErrString(OSStatus status)
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

    static char *audioUnitErrStrings[] =
        {"AudioUnitErr_CannotDoInCurrentContext",
         "AudioUnitErr_FailedInitialization",
         "AudioUnitErr_FileNotSpecified",
         "AudioUnitErr_FormatNotSupported",
         "AudioUnitErr_Initialized",
         "AudioUnitErr_InvalidElement",
         "AudioUnitErr_InvalidFile",
         "AudioUnitErr_InvalidOfflineRender",
         "AudioUnitErr_InvalidParameter",
         "AudioUnitErr_InvalidProperty",
         "AudioUnitErr_InvalidPropertyValue",
         "AudioUnitErr_InvalidScope",
         "AudioUnitErr_NoConnection",
         "AudioUnitErr_PropertyNotInUse",
         "AudioUnitErr_PropertyNotWritable",
         "AudioUnitErr_TooManyFramesToProcess",
         "AudioUnitErr_Unauthorized",
         "AudioUnitErr_Uninitialized",
         "AudioUnitErr_UnknownFileType",
         "AudioUnitErr_RenderTimeout"};

    for (int i = 0; i < sizeof(audioUnitErrCodes) / sizeof(UInt32); i++)
        if (audioUnitErrCodes[i] == status)
            return audioUnitErrStrings[i];

    return "";
}
