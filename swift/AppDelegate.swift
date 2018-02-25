//
//  AppDelegate.swift
//  Tuner
//
//  Created by Bill Farmer on 07/10/2017.
//  Copyright © 2017 Bill Farmer. All rights reserved.
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

import AppKit

var received = false;

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate
{
    var window: NSWindow!

    var menu: NSMenu!

    var container: NSStackView!
    var stack: NSStackView!

    @objc var scope: Scope!
    @objc var spectrum: Spectrum!
    @objc var display: Display!
    @objc var strobe: Strobe!
    @objc var meter: Meter!
    @objc var status: Status!

    func applicationDidFinishLaunching(_ aNotification: Notification)
    {
        // Insert code here to initialize your application
        app = self

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

        scope.layerContentsRedrawPolicy = .onSetNeedsDisplay
        spectrum.layerContentsRedrawPolicy = .onSetNeedsDisplay
        display.layerContentsRedrawPolicy = .onSetNeedsDisplay
        strobe.layerContentsRedrawPolicy = .onSetNeedsDisplay
        meter.layerContentsRedrawPolicy = .onSetNeedsDisplay
        status.layerContentsRedrawPolicy = .onSetNeedsDisplay

        // Start audio
        let result = SetupAudio()
        if (result != noErr)
        {
            DisplayAlert("Tuner", "Audio initialisation failed", result)
        }
    }

    func DisplayAlert(_ message: String, _ informativeText: String,
                      _ status: OSStatus)
    {
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.messageText = message

        let error = (status > 0) ? UTCreateStringForOSType(OSType(status))
          .takeRetainedValue() as String :
          String(utf8String: AudioUnitErrString(status))!

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
