//
//  AppDelegate.swift
//  Tuner
//
//  Created by Bill Farmer on 07/10/2017.
//  Copyright © 2017 Bill Farmer. All rights reserved.
//

import AppKit

var app: AppDelegate!
var received = false;

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate
{
    var window: NSWindow!

    var menu: NSMenu!

    var container: NSStackView!
    var stack: NSStackView!
    var scope: Scope!
    var spectrum: Spectrum!
    var display: Display!
    var strobe: Strobe!
    var meter: Meter!
    var status: Status!

    func applicationDidFinishLaunching(_ aNotification: Notification)
    {
        // Insert code here to initialize your application

        app = self;

        // Find a window
        if (NSApp.mainWindow != nil)
        {
            window = NSApp.mainWindow
        }

        else if (NSApp.keyWindow != nil)
        {
            window = NSApp.keyWindow
        }

        else if (NSApp.windows.count > 0)
        {
            window = NSApp.windows[0]
        }

        // Find the menu
        if (NSApp.mainMenu != nil)
        {
            menu = NSApp.mainMenu
        }

        if (window == nil)
        {
            return
        }

        scope = Scope()
        spectrum = Spectrum()
        display = Display()
        strobe = Strobe()
        meter = Meter()
        status = Status()

        stack = NSStackView(views: [scope, spectrum, display,
                                    strobe, meter])

        let spectrumHeight = NSLayoutConstraint(item: spectrum,
                                                attribute: .height,
                                                relatedBy: .equal,
                                                toItem: scope,
                                                attribute: .height,
                                                multiplier: 1,
                                                constant: 0)

        let displayHeight = NSLayoutConstraint(item: display,
                                               attribute: .height,
                                               relatedBy: .equal,
                                               toItem: spectrum,
                                               attribute: .height,
                                               multiplier: 3.25,
                                               constant: 0)

        let strobeHeight = NSLayoutConstraint(item: strobe,
                                              attribute: .height,
                                              relatedBy: .equal,
                                              toItem: spectrum,
                                              attribute: .height,
                                              multiplier: 1.375,
                                              constant: 0)

        let meterHeight = NSLayoutConstraint(item: meter,
                                             attribute: .height,
                                             relatedBy: .equal,
                                             toItem: strobe,
                                             attribute: .height,
                                             multiplier: 1.625,
                                             constant: 0)

        stack.addConstraint(spectrumHeight)
        stack.addConstraint(displayHeight)
        stack.addConstraint(strobeHeight)
        stack.addConstraint(meterHeight)

        stack.orientation = .vertical
        stack.spacing = 8
        stack.edgeInsets = NSEdgeInsets(top: 20, left: 20,
                                        bottom: 20, right: 20)

        container = NSStackView(views: [stack, status])

        let statusHeight = NSLayoutConstraint(item: status,
                                              attribute: .height,
                                              relatedBy: .equal,
                                              toItem: stack,
                                              attribute: .height,
                                              multiplier: 0.05,
                                              constant: 0)
        container.addConstraint(statusHeight)
        container.orientation = .vertical

        window.contentView = container

        // Start audio
        let result = SetupAudio()
        if (result != noErr)
        {
            DisplayAlert("Tuner", "Audio initialisation failed", result)
        }

        // Monitor events
        NSEvent.addLocalMonitorForEvents(matching: .applicationDefined,
         handler:
           {(event) -> NSEvent in
                   NSLog("Event received " +
                           String(describing: event))

               return event
           })

    }

    func DisplayAlert(_ message: String, _ informativeText: String,
                      _ status: OSStatus)
    {
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.messageText = message

        let error = (status > 0) ? UTCreateStringForOSType(OSType(status))
          .takeRetainedValue() as String :
          AudioUnitErrString(status).takeRetainedValue() as String

        alert.informativeText = informativeText + ": " + error +
          " (" + String(status) + ")"

        alert.runModal()
    }

    func applicationWillTerminate(_ aNotification: Notification)
    {
        // Insert code here to tear down your application
        // let _ = audio.shutdown()
    }
}

// Send event
func SendEvent()
{
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
}
