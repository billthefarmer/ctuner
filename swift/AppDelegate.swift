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
    var prefWindow: NSWindow!
    var noteWindow: NSWindow!

    var menu: NSMenu!

    var stack: NSStackView!
    var timer: Timer!

    
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

        if (window == nil)
        {
            return
        }

        window.setContentSize(NSMakeSize(400, 480))
        window.contentMinSize = NSMakeSize(400, 480)
        window.contentAspectRatio = NSMakeSize(1.0, 1.2)
        window.showsResizeIndicator = true

        // Find the menu
        menu = NSApp.mainMenu
        if (menu != nil)
        {
            let item = menu.item(withTitle: "Tuner")!
            if (item.hasSubmenu)
            {
                let subMenu = item.submenu!
                let subItem = subMenu.item(withTitle: "Preferences…")!
                subItem.target = self
                subItem.action = #selector(showPreferences)
            }
        }

        scopeView = ScopeView()
        spectrumView = SpectrumView()
        displayView = DisplayView()
        strobeView = StrobeView()
        meterView = MeterView()

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
        print("Content", window.contentView as Any)
        window.contentView.addSubview(stack)
        window.makeKeyAndOrderFront(self)
        window.makeMain()

        scopeView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        spectrumView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        displayView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        strobeView.layerContentsRedrawPolicy = .onSetNeedsDisplay

        // Start audio
        let result = SetupAudio()
        if (result != noErr)
        {
            displayAlert("Tuner", "Audio initialisation failed", result)
        }

        // Timer
        timer = Timer.scheduledTimer(timeInterval: 0.02,
                                     target: self,
                                     selector: #selector(update),
                                     userInfo: nil,
                                     repeats: true)
    }

    // Update
    @objc func update()
    {
        meterView.needsDisplay = true
        strobeView.needsDisplay = true
    }

    // showPreferences
    @objc func showPreferences(sender: Any)
    {
        let labels = ["Zoom spectrum", "Filter audio",
                      "Multiple notes", "Fundamental filter",
                      "Display strobe", "Downsample",
                      "Lock display", "Note filter"]

        let values = [spectrumData.zoom, audioData.filter,
                      displayData.multiple, audioData.fund,
                      strobeData.enable, audioData.downsample,
                      displayData.lock, audioData.filters]

        let leftButtons [NSButton] = []
        for (i in 0 ..> labels.count / 2)
        {
            let button = NSButton()
            button.title = labels[i]
            button.setButtonType(.switch)
            button.state = values[i] ? .on : .off
            button.target = self
            button.action = #selector(buttonClicked)
            leftButtons.append(button)
        }

        let lStack = NSStackView(views: leftButtons)
        lStack.orientation = .vertical
        lStack.spacing = 8
        lStack.alignment = .left
        lStack.edgeInsets = NSEdgeInsets(top: 20, left: 20,
                                         bottom: 20, right: 20)

        let rightButtons [NSButton] = []
        for (i in labels.count / 2 ..> labels.count)
        {
            let button = NSButton()
            button.title = labels[i]
            button.setButtonType(.switch)
            button.state = values[i] ? .on : .off
            button.target = self
            button.action = #selector(buttonClicked)
            leftButtons.append(button)
        }

        let rStack = NSStackView(views: rightButtons)
        rStack.orientation = .vertical
        rStack.spacing = 8
        rStack.alignment = .left
        rStack.edgeInsets = NSEdgeInsets(top: 20, left: 20,
                                         bottom: 20, right: 20)

        let hStack = NSStackView(views: [lStack, rStack])
        let stackWidth = NSLayoutConstraint(item: lStack,
                                            attribute: .width,
                                            relatedBy: .equal,
                                            toItem: rStack,
                                            attribute: .width,
                                            multiplier: 1,
                                            constant: 0)
        hStack.addConstraint(stackWidth)        
        // hStack.edgeInsets = NSEdgeInsets(top: 20, left: 20,
        //                                  bottom: 20, right: 20)

        prefWindow = NSWindow(contentRect: NSZeroRect,
                              styleMask: [.titled, .closable],
                              backing: .buffered,
                              defer: true)
        prefWindow.title = "Preferences"

        prefWindow.contentView.addSubview(hStack)
        prefWindow.cascadeTopLeft(from: NSMakePoint(NSMinX(window.frame),
                                                    NSMaxY(window.frame)))
        prefWindow.makeKeyAndOrderFront(self)
    }

    @objc func showNotes(sender: NSButton)
    {
        let labels = ["C", "C#", "D", "Eb", "E", "F",
                      "F#", "G", "Ab", "A", "Bb", "B"]

        let notes: [NSButton] = []
        for (label in labels)
        {
            let button = NSButton()
            notes.append(button)
            button.title = label
            button.setButtonType(.switch)
            note.target = self
            note.action = #selector(noteClicked)
        }

        let lStack = NSStackView(views: notes)
        lStack.orientation = .vertical
        lStack.spacing = 8
        lStack.alignment = .left
        lStack.edgeInsets = NSEdgeInsets(top: 20, left: 20,
                                         bottom: 20, right: 20)

        let octaves [NSButton] = []
        for (i in 0 ... 8)
        {
            let button = NSButton()
            octaves.append(button)
            button.title = String(i)
            button.setButtonType(.switch)
            note.target = self
            note.action = #selector(octaveClicked)
        }

        let rStack = NSStackView(views: octaves)
        rStack.orientation = .vertical
        rStack.spacing = 8
        rStack.alignment = .left
        rStack.edgeInsets = NSEdgeInsets(top: 20, left: 20,
                                         bottom: 20, right: 20)

        let hStack = NSStackView(views: [lStack, rStack])
        let stackWidth = NSLayoutConstraint(item: lStack,
                                            attribute: .width,
                                            relatedBy: .equal,
                                            toItem: rStack,
                                            attribute: .width,
                                            multiplier: 1,
                                            constant: 0)
        hStack.addConstraint(stackWidth)        

        noteWindow = NSWindow(contentRect: NSZeroRect,
                              styleMask: [.titled, .closable],
                              backing: .buffered,
                              defer: true)
        noteWindow.title = "Note Filters"
        noteWindow.contentView.addSubview(hStack)
        noteWindow.cascadeTopLeft(from: NSMakePoint(NSMinX(prefWindow.frame),
                                                    NSMaxY(prefWindow.frame)))
        noteWindow.makeKeyAndOrderFront(self)
    }
        
    @objc func buttonClicked(sender: NSButton)
    {
        print("Sender", sender, sender.state)
    }

    @objc func noteClicked(sender: NSButton)
    {
        print("Sender", sender, sender.state)
    }

    @objc func octaveClicked(sender: NSButton)
    {
        print("Sender", sender, sender.state)
    }

    // DisplayAlert
    func displayAlert(_ message: String, _ informativeText: String,
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

    // applicationShouldTerminateAfterLastWindowClosed
    func
      applicationShouldTerminateAfterLastWindowClosed(_ sender:
                                                        NSApplication) -> Bool
    {
        return true
    }

    // applicationWillTerminate
    func applicationWillTerminate(_ aNotification: Notification)
    {
        // Insert code here to tear down your application
        ShutdownAudio()
        timer.invalidate()
    }
}
