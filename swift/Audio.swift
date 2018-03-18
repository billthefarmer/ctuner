//
//  Audio.swift
//  Tuner
//
//  Created by Bill Farmer on 08/10/2017.
//  Copyright Â© 2017 Bill Farmer. All rights reserved.
//

import AppKit

import Foundation
import AudioUnit
import CoreAudio
import Accelerate

let kSampleRate1 = 11025.0
let kSampleRate2 = 12000.0

let kBytesPerPacket   = 4
let kBytesPerFrame    = 4
let kChannelsPerFrame = 1

let kOversample  = 16
let kSamples     = 16384
let kLog2Samples = 14
let kMaxima      = 8
let kFrames      = 512
let kRange       = kSamples * 3 / 8
let kStep        = kSamples / kOversample

let kA5Reference = 440
let kC5Offset    = 57
let kOctave      = 12

let kEventAudioUpdate = 1

class Audio: NSObject
{
    var output: AudioUnit? = nil
    var id: AudioDeviceID = kAudioObjectUnknown
    var downsample: Bool!
    var filter: Bool!
    var sample: UInt32!
    var divisor: UInt32!
    var frames: UInt32!

    func setup() -> OSStatus
    {
        // Specify an output unit
        var dc = AudioComponentDescription(componentType:
                                             kAudioUnitType_Output,
                                           componentSubType:
                                             kAudioUnitSubType_HALOutput,
                                           componentManufacturer:
	                                     kAudioUnitManufacturer_Apple,
                                           componentFlags: 0,
                                           componentFlagsMask: 0)
        // Find an output unit
        let cp! = AudioComponentFindNext(nil, &dc)

        if (cp == nil)
        {
            // AudioComponentFindNext
            NSLog("Error in AudioComponentFindNext")
            return -1
        }

        var status = AudioComponentInstanceNew(cp, &output)

        if (status != noErr)
        {
            // AudioComponentInstanceNew
            NSLog("Error in AudioComponentInstanceNew %d", status)
            return status
        }

        // Enable input
        var enable = true
        status =
          AudioUnitSetProperty(output,
			       kAudioOutputUnitProperty_EnableIO,
			       kAudioUnitScope_Input,
			       1, &enable,
                               UInt32(MemoryLayout.size(ofValue: enable)))
        if (status != noErr)
        {
            // AudioUnitSetProperty
            NSLog("Error in AudioUnitSetProperty: " +
	            "kAudioOutputUnitProperty_EnableIO %d", status)
            return status
        }

        // Disable output
        enable = false
        status =
          AudioUnitSetProperty(output,
			       kAudioOutputUnitProperty_EnableIO,
			       kAudioUnitScope_Output,
			       0, &enable,
                               UInt32(MemoryLayout.size(ofValue: enable)))
        if (status != noErr)
        {
            // AudioUnitSetProperty
            NSLog("Error in AudioUnitSetProperty: " +
	            "kAudioOutputUnitProperty_EnableIO %d", status)
	    return status
        }

        // Get the default input device
        var inputDeviceAOPA: AudioObjectPropertyAddress =
          AudioObjectPropertyAddress(mSelector:
                                       kAudioHardwarePropertyDefaultInputDevice,
                                     mScope: kAudioObjectPropertyScopeGlobal,
                                     mElement:
                                       kAudioObjectPropertyElementMaster)

        var size: UInt32 = UInt32(MemoryLayout.size(ofValue: id))

        // Get device
        status =
          AudioObjectGetPropertyData(UInt32(kAudioObjectSystemObject),
                                     &inputDeviceAOPA,
                                     0, nil, &size, &id)
        if (status != noErr)
        {
	    // AudioObjectGetPropertyData
            NSLog("Error in AudioObjectGetPropertyData: " +
	            "kAudioHardwarePropertyDefaultInputDevice %d", status)
	    return status
        }

        NSLog("System input device %d", id)

        // Set the audio unit device
        status =
          AudioUnitSetProperty(output,
			       kAudioOutputUnitProperty_CurrentDevice, 
			       kAudioUnitScope_Global,
                               0, &id, size)
        if (status != noErr)
        {
            // AudioUnitSetProperty
            NSLog("Error in AudioUnitSetProperty: " +
	            "kAudioOutputUnitProperty_CurrentDevice " +
                    AudioUnitErrString(status))
	    return status
        }

        // Get nominal sample rates size
        var audioDeviceAOPA: AudioObjectPropertyAddress =
          AudioObjectPropertyAddress(mSelector:
                                       kAudioDevicePropertyAvailableNominalSampleRates,
                                     mScope: kAudioObjectPropertyScopeGlobal,
                                     mElement:
                                       kAudioObjectPropertyElementMaster)

        status = AudioObjectGetPropertyDataSize(id, &audioDeviceAOPA,
                                                0, nil, &size)
        if (status != noErr)
        {
            // AudioObjectGetPropertyDataSize
            NSLog("Error in AudioObjectGetPropertyDataSize: " +
	            "kAudioDevicePropertyAvailableNominalSampleRates %d",
                  status)
            return status
        }

        // Get nominal sample rates
        var rates = Array(repeating: AudioValueRange(mMinimum: 0.0,
                                                     mMaximum: 0.0),
                          count: Int(size) / MemoryLayout<AudioValueRange>.size)

        status = AudioObjectGetPropertyData(id, &audioDeviceAOPA, 0, nil,
                                            &size, &rates)
        if (status != noErr)
        {
            // AudioObjectGetPropertyData
            NSLog("Error in AudioObjectGetPropertyData: " +
                  "kAudioDevicePropertyAvailableNominalSampleRates %d", status)
            return status
        }

        for (_, rate) in rates.enumerated()
        {
            NSLog(String(format: "SampleRates: %f, %f",
                         rate.mMinimum, rate.mMaximum))
        }

        // See if we can change the sample rate
        var inRange = false
        var nominal = 0.0

        for (_, rate) in rates.enumerated()
        {
	    if ((rate.mMinimum <= kSampleRate1) &&
	          (rate.mMaximum >= kSampleRate1))
	    {
	        inRange = true
	        nominal = kSampleRate1
	        break
	    }

	    if ((rate.mMinimum <= kSampleRate2) &&
	          (rate.mMaximum >= kSampleRate2))
	    {
	        inRange = true
	        nominal = kSampleRate2
	        break
	    }
        }

        size = UInt32(MemoryLayout.size(ofValue: nominal))

        // Set the sample rate, if in range
        if (inRange)
        {
	    status = AudioObjectSetPropertyData(id, &audioDeviceAOPA, 0, nil,
                                                size, &nominal)
        }

        // Get nominal sample rate
        status = AudioObjectGetPropertyData(id, &audioDeviceAOPA, 0, nil,
                                            &size, &nominal)
        if (status != noErr)
        {
            // AudioObjectGetPropertyData
            NSLog("Error in AudioObjectGetPropertyData: " +
                  "kAudioDevicePropertyNominalSampleRate %d", status)
            return status
        }

        NSLog("kAudioDevicePropertyNominalSampleRate %f", nominal)

        // Set the divisor
        divisor = UInt32(round(nominal / ((kSampleRate1 + kSampleRate2) / 2)))

        // Set the rate
        sample = UInt32(nominal) / divisor

        // Get the buffer size range size
        audioDeviceAOPA =
          AudioObjectPropertyAddress(mSelector:
                                       kAudioDevicePropertyBufferFrameSizeRange,
                                     mScope: kAudioObjectPropertyScopeGlobal,
                                     mElement:
                                       kAudioObjectPropertyElementMaster)

        var sizes = AudioValueRange(mMinimum: 0.0, mMaximum: 0.0)
        size = UInt32(MemoryLayout.size(ofValue: sizes))

        // Get the buffer size range
        status = AudioObjectGetPropertyData(id, &audioDeviceAOPA, 0, nil,
                                            &size, &sizes)
        if (status != noErr)
        {
            // AudioObjectGetPropertyData
            NSLog("Error in AudioObjectGetPropertyData: " +
                  "kAudioDevicePropertyBufferFrameSizeRange %d", status)
            return status
        }

        NSLog(String(format: "BufferFrameSizeRange: %f, %f",
                     sizes.mMinimum, sizes.mMaximum))

        var frames = UInt32(UInt32(kStep) * divisor)
        size = UInt32(MemoryLayout.size(ofValue: frames))

        while (!((sizes.mMaximum >= Float64(frames)) &&
	           (sizes.mMinimum <= Float64(frames))))
        {
	    frames /= 2
        }

        // Set the max frames
        status = AudioUnitSetProperty(output,
				      kAudioUnitProperty_MaximumFramesPerSlice,
				      kAudioUnitScope_Global, 0,
				      &frames, size)
        if (status != noErr)
        {
            // kAudioUnitProperty_MaximumFramesPerSlice
            NSLog("Error in AudioUnitSetProperty: " +
	            "kAudioUnitProperty_MaximumFramesPerSlice " +
                    AudioUnitErrString(status))
            return status
        }

        // Get the frames
        status = AudioUnitGetProperty(output,
				      kAudioUnitProperty_MaximumFramesPerSlice,
				      kAudioUnitScope_Global, 0,
                                      &frames, &size)
        if (status != noErr)
        {
            // kAudioUnitProperty_MaximumFramesPerSlice
            NSLog("Error in AudioUnitGetProperty: " +
	            "kAudioUnitProperty_MaximumFramesPerSlice " +
                    AudioUnitErrString(status))
            return status
        }

        NSLog("kAudioUnitProperty_MaximumFramesPerSlice %d", frames)

        self.frames = frames

        var format = AudioStreamBasicDescription()
        size = UInt32(MemoryLayout.size(ofValue: format))

        // Get stream format
        status = AudioUnitGetProperty(output,
				      kAudioUnitProperty_StreamFormat,
				      kAudioUnitScope_Input, 1,
				      &format, &size)
        if (status != noErr)
        {
            // kAudioUnitProperty_StreamFormat
            NSLog("Error in AudioUnitGetProperty: " +
	            "kAudioUnitProperty_StreamFormat " +
                    AudioUnitErrString(status))
            return status
        }

        format.mSampleRate = nominal
        format.mBytesPerPacket = UInt32(kBytesPerPacket)
        format.mBytesPerFrame = UInt32(kBytesPerFrame)
        format.mChannelsPerFrame = UInt32(kChannelsPerFrame)

        // Set stream format
        status = AudioUnitSetProperty(output,
				      kAudioUnitProperty_StreamFormat,
				      kAudioUnitScope_Output, 1,
				      &format, size)
        if (status != noErr)
        {
            // kAudioUnitProperty_StreamFormat
            NSLog("Error in AudioUnitSetProperty: " +
	            "kAudioUnitProperty_StreamFormat " +
                    AudioUnitErrString(status))
            return status
        }

        // Init buffer
        buffer = Array(repeating: 0, count: kSamples)

        var input =
          AURenderCallbackStruct(inputProc: InputProc,
                                 inputProcRefCon: output)
        // Set callback
        status = AudioUnitSetProperty(output,
				      kAudioOutputUnitProperty_SetInputCallback,
				      kAudioUnitScope_Global, 0,
				      &input,
                                      UInt32(MemoryLayout.size(ofValue: input)))
        if (status != noErr)
        {
            // kAudioOutputUnitProperty_SetInputCallback
            NSLog("Error in AudioUnitSetProperty: " +
	            "kAudioOutputUnitProperty_SetInputCallback " +
                    AudioUnitErrString(status))
            return status
        }

        // Initialize the audio unit
        status = AudioUnitInitialize(output)
        if (status != noErr)
        {
            // AudioUnitInitialize
            NSLog("Error in AudioUnitInitialize: " +
                    AudioUnitErrString(status))
            return status
        }

        // Start the audio unit
        status = AudioOutputUnitStart(output)
        if (status != noErr)
        {
            // AudioOutputUnitStart
            NSLog("Error in AudioOutputUnitStart: " +
                    AudioUnitErrString(status))
            return status
        }

        NSLog("Audio setup complete")

        NSEvent.addLocalMonitorForEvents
        (matching: .applicationDefined,
         handler:
           {(event) -> NSEvent in
               NSLog("Event received " +
                       String(describing: event))
               return event
           })

        NSLog("Event setup complete")

        // Create an event to post to the main event queue
        let event = NSEvent.otherEvent(with: .applicationDefined,
                                       location: NSZeroPoint,
                                       modifierFlags:
                                         NSEvent.ModifierFlags.function,
                                       timestamp: 0,
                                       windowNumber: 0,
                                       context: nil,
                                       subtype: Int16(kEventAudioUpdate),
                                       data1: 0,
                                       data2: 0)
        NSApp.sendEvent(event!)

        NSLog("Event sent")
        return status
    }

    // shutdown
    func shutdown() -> OSStatus
    {
        var status: OSStatus!

        status = AudioOutputUnitStop(output)
        if (status != noErr)
        {
            return status
        }

        status = AudioUnitUninitialize(output)

        return status
    }

    func AudioUnitErrString(_ status: OSStatus) -> String
    {
        let audioUnitErrCodes =
          [kAudioUnitErr_CannotDoInCurrentContext,
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
           kAudioUnitErr_RenderTimeout,
           kAudioUnitErr_ExtensionNotFound,
           kAudioUnitErr_MIDIOutputBufferFull]

        let audioUnitErrStrings =
          ["AudioUnitErr_CannotDoInCurrentContext",
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
           "AudioUnitErr_RenderTimeout",
           "AudioUnitErr_ExtensionNotFound",
           "AudioUnitErr_MIDIOutputBufferFull"]

        for (index, value) in audioUnitErrCodes.enumerated()
        {
            if (value == status)
            {
                return audioUnitErrStrings[index]
            }
        }

        return ""
    }
}

// Private data for callback proc
private var abl =
  AudioBufferList(mNumberBuffers: 1,
                  mBuffers: AudioBuffer(mNumberChannels: 1,
                                        mDataByteSize: 0,
                                        mData: nil))

private var buffer: Array<Float32>!

private var xv: Array<Float32> = Array(repeating: 0, count: 2)
private var yv: Array<Float32> = Array(repeating: 0, count: 2)

private var rendered = false

// InputProc
private func InputProc(inRefCon: UnsafeMutableRawPointer,
                       ioActionFlags:
                         UnsafeMutablePointer<AudioUnitRenderActionFlags>,
                       inTimeStamp:
                         UnsafePointer<AudioTimeStamp>,
                       inBusNumber: UInt32,
                       inNumberFrames: UInt32,
                       ioData:
                         UnsafeMutablePointer<AudioBufferList>?) -> OSStatus
{
    // Render data
    let status =
      AudioUnitRender(inRefCon.load(as: AudioUnit.self),
                      ioActionFlags, inTimeStamp,
                      inBusNumber, inNumberFrames,
                      ioData!)
    if (status != noErr)
    {
        // AudioUnitRender
        NSLog("Error in AudioUnitRender: " +
                AudioUnitErrString(status))
	return status
    }

    NSLog("AudioUnitRender")

    if (!rendered)
    {
        NSLog("AudioUnitRender")
        rendered = true
    }

    // Copy the input data
    for index in 0..<buffer.count - kSamples
    {
        buffer[index] = buffer[kSamples + index]
    }

    var data = ioData!.pointee.mBuffers.mData!.load(as: [Float32].self)

    let G: Float32 = 3.023332184e+01
    let K: Float32 = 0.9338478249

    // Butterworth filter, 3dB/octave
    for i in 0..<audio.frames / audio.divisor
    {
	xv[0] = xv[1]
	xv[1] = data[Int(i * audio.divisor)] / G

	yv[0] = yv[1]
	yv[1] = (xv[0] + xv[1]) +
          (K * yv[0])

	// Choose filtered/unfiltered data
 	buffer[(kSamples -
                  Int(audio.frames /
                                  audio.divisor)) + Int(i)] =
	  audio.filter ? yv[1] : data[Int(i * audio.divisor)]
    }

    // Create an event to post to the main event queue
    let event = NSEvent.otherEvent(with: .applicationDefined,
                                   location: NSZeroPoint,
                                   modifierFlags:
                                     NSEvent.ModifierFlags(rawValue: 0),
                                   timestamp: 0,
                                   windowNumber: 0,
                                   context: nil,
                                   subtype: Int16(kEventAudioUpdate),
                                   data1: 0,
                                   data2: 0)
    NSApp.sendEvent(event!)

    return noErr
}
