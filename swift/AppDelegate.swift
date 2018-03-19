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

    // applicationDidFinishLaunching
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

        window.setContentSize(NSMakeSize(400, 480))
        window.contentMinSize = NSMakeSize(400, 480)
        window.contentAspectRatio = NSMakeSize(1.0, 1.2)
        window.showsResizeIndicator = false

        scopeView = ScopeView()
        spectrumView = SpectrumView()
        displayView = DisplayView()
        strobeView = StrobeView()
        meterView = MeterView()
        // statusView = StatusView()

        stack = NSStackView(views: [scopeView, spectrumView, displayView,
                                    strobeView, meterView])

        let spectrumHeight = NSLayoutConstraint(item: spectrumView,
                                                attribute: .height,
                                                relatedBy: .equal,
                                                toItem: scopeView,
                                                attribute: .height,
                                                multiplier: 1,
                                                constant: 0)

        let displayHeight = NSLayoutConstraint(item: displayView,
                                               attribute: .height,
                                               relatedBy: .equal,
                                               toItem: spectrumView,
                                               attribute: .height,
                                               multiplier: 3.25,
                                               constant: 0)

        let strobeHeight = NSLayoutConstraint(item: strobeView,
                                              attribute: .height,
                                              relatedBy: .equal,
                                              toItem: spectrumView,
                                              attribute: .height,
                                              multiplier: 1.375,
                                              constant: 0)

        let meterHeight = NSLayoutConstraint(item: meterView,
                                             attribute: .height,
                                             relatedBy: .equal,
                                             toItem: strobeView,
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

        // container = NSStackView(views: [stack, statusView])

        // let statusHeight = NSLayoutConstraint(item: statusView,
        //                                       attribute: .height,
        //                                       relatedBy: .equal,
        //                                       toItem: stack,
        //                                       attribute: .height,
        //                                       multiplier: 0.05,
        //                                       constant: 0)
        // container.addConstraint(statusHeight)
        // container.orientation = .vertical

        window.contentView = stack

        scopeView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        spectrumView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        displayView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        strobeView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        // meterView.layerContentsRedrawPolicy = .onSetNeedsDisplay

        // Start audio
        let result = SetupAudio()
        if (result != noErr)
        {
            DisplayAlert("Tuner", "Audio initialisation failed", result)
        }
    }

    // DisplayAlert
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

    // applicationWillTerminate
    func applicationWillTerminate(_ aNotification: Notification)
    {
        // Insert code here to tear down your application
        ShutdownAudio()
    }

    // keyDown
    func keyDown(event: NSEvent)
    {
        let key = event.characters
        switch key!.lowercased()
        {
        case "d":
            audioData.downsample = !audioData.downsample

        case "f":
            audioData.filter = !audioData.filter

        case "l":
            displayData.lock = !displayData.lock

        case "m":
            displayData.multiple = !displayData.multiple

        case "s":
            strobeData.enable = !strobeData.enable

        case "z":
            spectrumData.zoom = !spectrumData.zoom

        default:
            NSLog("Key %s", key!)
        }
    }
}

class WindowDelegate: NSObject, NSWindowDelegate
{
    func window(_ window: NSWindow, 
                willUseFullScreenPresentationOptions proposedOptions:
                  NSApplication.PresentationOptions = [])
      -> NSApplication.PresentationOptions
    {
        var options = proposedOptions
        options.remove(.fullScreen)
        return options
    }
}
