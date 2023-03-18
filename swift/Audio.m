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
    AudioComponent cp = AudioComponentFindNext(nil, &dc);

    if (cp == nil)
    {
        // AudioComponentFindNext
        NSLog(@"Error in AudioComponentFindNext");
	return '!obj';
    }

    // Open it
    OSStatus status = AudioComponentInstanceNew(cp, &audio.output);
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
    status = AudioUnitSetProperty(audio.output,
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
    status = AudioUnitSetProperty(audio.output,
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
         kAudioObjectPropertyElementMain};

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
    status = AudioUnitSetProperty(audio.output,
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

    Float64 nominal = kSampleRate;
    size = sizeof(nominal);

    // AudioObjectPropertyAddress
    AudioObjectPropertyAddress audioDeviceAOPA =
        {kAudioDevicePropertyNominalSampleRate,
         kAudioObjectPropertyScopeGlobal,
         kAudioObjectPropertyElementMain};

    // Set the sample rate, if in range, ignore errors
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

    // Set the rate
    audio.sample = nominal;

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
                    "kAudioDevicePropertyBufferFrameSizeRange %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    UInt32 frames = kStep;
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
    {
        // AudioUnitSetProperty
        NSLog(@"Error in AudioUnitSetProperty: " 
                    "kAudioUnitProperty_MaximumFramesPerSlice %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Get the frames
    status = AudioUnitGetProperty(audio.output,
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
    status = AudioUnitSetProperty(audio.output,
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

    AURenderCallbackStruct input =
	{InputProc, &audio.output};

    // Set callback
    status = AudioUnitSetProperty(audio.output,
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
    status = AudioUnitInitialize(audio.output);

    if (status != noErr)
    {
        // AudioUnitInitialize
        NSLog(@"Error in AudioUnitInitialize %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    // Start the audio unit
    status = AudioOutputUnitStart(audio.output);

    if (status != noErr)
    {
        // AudioOutputUnitStart
        NSLog(@"Error in AudioOutputUnitStart %s (%d)",
              AudioUnitErrString(status), status);
        return status;
    }

    return status;
}

// ShutdownAudio
OSStatus ShutdownAudio()
{
    AudioOutputUnitStop(audio.output);
    AudioUnitUninitialize(audio.output);

    return noErr;
}

// Input proc
OSStatus InputProc(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
		   const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
		   UInt32 inNumberFrames, AudioBufferList *ioData)
{
    static AudioBufferList abl =
	{1, {1, 0, nil}};

    // Initialise data structs
    static double buffer[kSamples];

    if (audio.buffer == nil)
	audio.buffer = buffer;

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
    memmove(buffer, buffer + inNumberFrames,
	    (kSamples - inNumberFrames) * sizeof(double));

    Float32 *data = abl.mBuffers[0].mData;

    // Butterworth filter, 3dB/octave
    for (int i = 0; i < inNumberFrames; i++)
    {
	static double G = 3.023332184e+01;
	static double K = 0.9338478249;

	static double xv[2];
	static double yv[2];

	xv[0] = xv[1];
	xv[1] = data[i] / G;

	yv[0] = yv[1];
	yv[1] = (xv[0] + xv[1]) + (K * yv[0]);

	// Choose filtered/unfiltered data
	buffer[(kSamples - inNumberFrames) + i] =
	    audio.filt? yv[1]: data[i];
    }

    // Run in main queue
    dispatch_async(dispatch_get_main_queue(), ProcessAudio);

    return noErr;
}

// Process audio
void (^ProcessAudio)(void) = ^
{
    enum
    {kTimerCount = 64,
     kDelayCount = 16};

    // Arrays for processing input
    static double xa[kRange];
    static double xp[kRange];
    static double xq[kRange];
    static double xf[kRange];

    static double x2[kRange / 2];
    static double x3[kRange / 3];
    static double x4[kRange / 4];
    static double x5[kRange / 5];

    static double dxa[kRange];
    static double dxp[kRange];

    static maximum maxima[kMaxima];
    static double  values[kMaxima];

    static double window[kSamples];
    static double input[kSamples];

    static double re[kSamples2];
    static double im[kSamples2];

    static DSPDoubleSplitComplex x =
	{re, im};

    static FFTSetupD setup;

    static double fps;
    static double expect;

    // Initialise structures
    if (scope.data == nil)
    {
	scope.data = audio.buffer + kSamples2;
	scope.length = audio.frames;

	spectrum.data = xa;
	spectrum.length = kRange;
	spectrum.values = values;

	disp.maxima = maxima;

	fps = audio.sample / (double)kSamples;
	expect = 2.0 * M_PI * (double)audio.frames / (double)kSamples;

	// Init Hamming window
	vDSP_hamm_windowD(window, kSamples, 0);

	// Init FFT
	setup = vDSP_create_fftsetupD(kLog2Samples, kFFTRadix2);
    }

    // Maximum data value
    static double dmax;

    if (dmax < 0.125)
	dmax = 0.125;

    // Calculate normalising value
    double norm = dmax;

    // Get max magitude
    vDSP_maxmgvD(audio.buffer, 1, &dmax, kSamples);

    // Divide by normalisation
    vDSP_vsdivD(audio.buffer, 1, &norm, input, 1, kSamples);

    // Multiply by window
    vDSP_vmulD(input, 1, window, 1, input, 1, kSamples);

    // Copy input to split complex vector
    vDSP_ctozD((DSPDoubleComplex *)input, 2, &x, 1, kSamples2);

    // Do FFT
    vDSP_fft_zripD(setup, &x, 1, kLog2Samples, kFFTDirection_Forward);

    // Zero the zeroth part
    x.realp[0] = 0.0;
    x.imagp[0] = 0.0;

    // Scale the output
    double scale = kScale;

    vDSP_vsdivD(x.realp, 1, &scale, x.realp, 1, kSamples2);
    vDSP_vsdivD(x.imagp, 1, &scale, x.imagp, 1, kSamples2);

    // Magnitude
    vDSP_vdistD(x.realp, 1, x.imagp, 1, xa, 1, kRange);

    // Phase
    vDSP_zvphasD(&x, 1, xq, 1, kRange);

    // Phase difference
    vDSP_vsubD(xp, 1, xq, 1, dxp, 1, kRange);

    for (int i = 1; i < kRange; i++)
    {
	// Do frequency calculation
	double dp = dxp[i];

	// Calculate phase difference
	dp -= (double)i * expect;

	int qpd = dp / M_PI;

	if (qpd >= 0)
	    qpd += qpd & 1;

	else
	    qpd -= qpd & 1;

	dp -=  M_PI * (double)qpd;

	// Calculate frequency difference
	double df = kOversample * dp / (2.0 * M_PI);

	// Calculate actual frequency from slot frequency plus
	// frequency difference
	xf[i] = i * fps + df * fps;

	// Calculate differences for finding maxima
	dxa[i] = xa[i] - xa[i - 1];
    }

    // Copy phase vector
    memmove(xp, xq, kRange * sizeof(double));

    // Downsample
    if (audio.down)
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
    double  max;
    vDSP_Length imax;

    vDSP_maxmgviD(xa, 1, &max, &imax, kRange);

    double f = xf[imax];

    int count = 0;
    int limit = kRange - 1;

    // Find maximum value, and list of maxima
    for (int i = 1; i < limit; i++)
    {
        // Clear maxima
        maxima[count].f  = 0.0;
        maxima[count].fr = 0.0;
        maxima[count].n  = 0;

        values[count] = 0.0;

        // Cents relative to reference
        double cf = -12.0 * log2(audio.reference / xf[i]);
        int note = round(cf) + kC5Offset;

        // Ignore negative
        if (note < 0)
            continue;

        // Fundamental filter
        if ((audio.fund) && (count > 0) &&
            ((note % kOctave) != (maxima[0].n % kOctave)))
            continue;

        // Note filter
        if (audio.note)
        {
            // Get note and octave
            int n = note % kOctave;
            int o = note / kOctave;

            // Ignore too high
            if (o >= Length(filters.octave))
                continue;

            // Ignore if filtered
            if (!filters.note[n] ||
                !filters.octave[o])
                continue;
        }

        // If display not locked, find maxima and add to list
	if (!displayView.lock && count < Length(maxima) &&
	    xa[i] > kMin && xa[i] > (max / 4) &&
	    dxa[i] > 0.0 && dxa[i + 1] < 0.0)
	{
            // Frequency
	    maxima[count].f = xf[i];

	    // Note number
	    maxima[count].n = note;

	    // Octave note number
	    int n = (note - audio.key + kOctave) % kOctave;
	    // A note number
	    int a = (kAOffset - audio.key + kOctave) % kOctave;

	    // Temperament ratio
	    double temperRatio = temperamentValues[audio.temper][n] /
	      temperamentValues[audio.temper][a];
	    // Equal ratio
	    double equalRatio = temperamentValues[kEqual][n] /
	      temperamentValues[kEqual][a];

	    // Temperament adjustment
	    double temperAdjust = temperRatio / equalRatio;

	    // Reference note
	    maxima[count].fr = audio.reference *
                pow(2.0, round(cf) / 12.0) * temperAdjust;

	    // Set limit to octave above
	    if (!audio.down && (limit > i * 2))
		limit = i * 2 - 1;

	    count++;
	}
    }

    // Reference note frequency and lower and upper limits
    double fr = 0.0;
    double fl = 0.0;
    double fh = 0.0;

    // Note number
    int note = 0;

    // Found flag and cents value
    bool found = false;
    double c = 0.0;

    // Do the note and cents calculations
    if (max > kMin)
    {
	found = true;

	// Frequency
	if (!audio.down)
	    f = maxima[0].f;

	// Cents relative to reference
	double cf = -12.0 * log2(audio.reference / f);

        // Don't count silly values
        if (isnan(cf))
        {
            cf = 0.0;
            found = false;
        }

	// Note number
	note = round(cf) + kC5Offset;

	if (note < 0)
	    found = false;

	// Octave note number
	int n = (note - audio.key + kOctave) % kOctave;
	// A note number
	int a = (kAOffset - audio.key + kOctave) % kOctave;

	// Temperament ratio
	double temperRatio = temperamentValues[audio.temper][n] /
            temperamentValues[audio.temper][a];
	// Equal ratio
	double equalRatio = temperamentValues[kEqual][n] /
            temperamentValues[kEqual][a];

        // Temperament adjustment
        double temperAdjust = temperRatio / equalRatio;

	// Reference note
	fr = audio.reference * pow(2.0, round(cf) / 12.0) * temperAdjust;

	// Lower and upper freq
	fl = audio.reference * pow(2.0, (round(cf) - 0.60) /
				       12.0) * temperAdjust;
	fh = audio.reference * pow(2.0, (round(cf) + 0.60) /
				       12.0) * temperAdjust;

	// Find nearest maximum to reference note
	double df = 1000.0;

	for (int i = 0; i < count; i++)
	{
	    if (fabs(maxima[i].f - fr) < df)
	    {
		df = fabs(maxima[i].f - fr);
		f = maxima[i].f;
	    }
	}

	// Cents relative to reference note
	c = -12.0 * log2(fr / f);

	// Ignore silly values
	if (isnan(c))
        {
	    c = 0.0;
            found = false;
        }

	// Ignore if not within 50 cents of reference note
	if (fabs(c) > 0.6)
	    found = false;
    }

    // If display not locked
    if (!displayView.lock)
    {
        static long delay;

        // Update scope window
	if ((delay % kDelayCount) == 0)
	    scopeView.needsDisplay = true;

        if (found && (delay % kDelayCount) == 0)
        {
            // Update spectrum window
            for (int i = 0; i < count; i++)
                values[i] = maxima[i].f / fps;

            spectrum.count = count;

	    spectrumView.f = f  / fps;
	    spectrumView.r = fr / fps;
	    spectrumView.l = fl / fps;
	    spectrumView.h = fh / fps;
	}

	if ((delay % kDelayCount) == 0)
	    spectrumView.needsDisplay = true;

        // Increment delay
        delay++;
    }

    // Timer
    static long timer;

    // Found
    if (found)
    {
        static long delay;

	// If display not locked
	if (!displayView.lock && (delay % kDelayCount) == 0)
	{
	    // Update the display struct
	    displayView.f = f;
	    displayView.fr = fr;
	    displayView.c = c;
	    displayView.n = note;
	    disp.count = count;

	    // Update display
            displayView.needsDisplay = true;

	    // Update staff
	    staffView.note = note;

	    // Update meter
	    meterView.c = c;

	    // Update strobe
	    strobeView.c = c;
	}

        // Increment delay
        delay++;

        // Reset count;
	timer = 0;
    }

    // Not found
    else
    {
	// If display not locked
	if (!displayView.lock)
	{

	    if (timer == kTimerCount)
	    {
		displayView.f = 0.0;
		displayView.fr = 0.0;
		displayView.c = 0.0;
		displayView.n = 0;
		disp.count = 0;

		// Update display
                displayView.needsDisplay = true;

		// Update staff
		staffView.note = 0;

		// Update meter
		meterView.c = 0.0;

		// Update strobe
		strobeView.c = 0.0;

		// Update spectrum
		spectrumView.f = 0.0;
		spectrumView.r = 0.0;
		spectrumView.l = 0.0;
		spectrumView.h = 0.0;
                spectrumView.needsDisplay = true;
	    }
	}
    }

    timer++;
};

// Boolean array access functions to work around Swift limitations
// getNote
bool getNote(int index)
{
    return filters.note[index];
}

// setNote
void setNote(bool value, int index)
{
    filters.note[index] = value;
}

// getOctave
bool getOctave(int index)
{
    return filters.octave[index];
}

// setOctave
void setOctave(bool value, int index)
{
    filters.octave[index] = value;
}

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

    return "UnknownErr";
}
