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
    let kRefText = 17
    let kRefStep = 18

    var window: NSWindow!
    var prefWindow: NSWindow? = nil
    var noteWindow: NSWindow? = nil

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

        window.contentView = stack
        window.makeKeyAndOrderFront(self)
        window.makeMain()

        scopeView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        spectrumView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        displayView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        strobeView.layerContentsRedrawPolicy = .onSetNeedsDisplay

        // Get preferences
        getPreferences()

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

    var refText: NSTextField!
    var refStep: NSStepper!

    // showPreferences
    @objc func showPreferences(sender: Any)
    {
        if (prefWindow != nil)
        {
            prefWindow?.makeKeyAndOrderFront(self)
            return
        }

        let labels = ["Zoom spectrum", "Filter audio",
                      "Multiple notes", "Fundamental filter",
                      "Display strobe", "Downsample",
                      "Lock display", "Note filter"]

        let values = [spectrumData.zoom, audioData.filt,
                      displayData.mult, audioData.fund,
                      strobeData.enable, audioData.down,
                      displayData.lock, audioData.note]

        var leftButtons: [NSButton] = []
        var rightButtons: [NSButton] = []
        for (index, label) in labels.enumerated()
        {
            let button = NSButton()
            button.title = label
            button.setButtonType(.switch)
            button.tag = index
            button.state = values[index] ? .on : .off
            button.target = self
            button.action = #selector(buttonClicked)

            if (index < labels.count / 2)
            {
                leftButtons.append(button)
            }

            else
            {
            rightButtons.append(button)
            }
        }

        let lStack = NSStackView(views: leftButtons)
        lStack.orientation = .vertical
        lStack.spacing = 8
        lStack.alignment = .left

        let rStack = NSStackView(views: rightButtons)
        rStack.orientation = .vertical
        rStack.spacing = 8
        rStack.alignment = .left

        let hStack = NSStackView(views: [lStack, rStack])
        hStack.spacing = 8
        let stackWidth = NSLayoutConstraint(item: lStack,
                                            attribute: .width,
                                            relatedBy: .equal,
                                            toItem: rStack,
                                            attribute: .width,
                                            multiplier: 1,
                                            constant: 0)
        hStack.addConstraint(stackWidth)
        hStack.edgeInsets = NSEdgeInsets(top: 0, left: 40,
                                         bottom: 0, right: 40)

        let label = NSTextField()
        label.stringValue = "Ref:"
        label.isEditable = false
        label.isBordered = false
        label.drawsBackground = false
        refText = NSTextField()
        refText.tag = kRefText
        refText.doubleValue = audioData.reference
        refText.preferredMaxLayoutWidth = 24
        refText.target = self
        refText.action = #selector(refChanged)
        refStep = NSStepper()
        refStep.tag = kRefStep
        refStep.maxValue = 480.0
        refStep.minValue = 420.0
        refStep.increment = 1.0
        refStep.doubleValue = audioData.reference
        refStep.target = self
        refStep.action = #selector(refChanged)
        let button = NSButton()
        button.title = "Filters…"
        button.setButtonType(.momentaryPushIn)
        button.bezelStyle = .rounded
        button.target = self
        button.action = #selector(showNotes)
        let row = NSStackView()
        row.setViews([label, refText, refStep], in: .leading)
        row.setViews([button], in: .trailing)

        stack = NSStackView(views: [hStack, row])
        stack.orientation = .vertical
        stack.spacing = 20
        stack.edgeInsets = NSEdgeInsets(top: 40, left: 40,
                                        bottom: 40, right: 40)
        
        prefWindow = NSWindow(contentRect: .zero,
                              styleMask: [.titled, .closable],
                              backing: .buffered,
                              defer: true)
        prefWindow?.title = "Preferences"

        prefWindow?.contentView = stack
        prefWindow?.isReleasedWhenClosed = false
        prefWindow?.cascadeTopLeft(from: window.cascadeTopLeft(from: .zero))
        prefWindow?.makeKeyAndOrderFront(self)
    }

    @objc func showNotes(sender: NSButton)
    {
        if (noteWindow != nil)
        {
            noteWindow?.makeKeyAndOrderFront(self)
            return
        }

        let labels = ["C", "C#", "D", "Eb", "E", "F",
                      "F#", "G", "Ab", "A", "Bb", "B"]

        var notesL: [NSButton] = []
        var notesR: [NSButton] = []
        for (index, label) in labels.enumerated()
        {
            let button = NSButton()
            button.title = label
            button.tag = index
            button.setButtonType(.switch)
            button.state = getNote(index) ? .on : .off
            button.target = self
            button.action = #selector(noteClicked)

            if (index < labels.count / 2)
            {
                notesL.append(button)
            }

            else
            {
                notesR.append(button)
            }
        }

        let lStack = NSStackView(views: notesL)
        lStack.orientation = .vertical
        lStack.spacing = 8
        lStack.alignment = .left
        lStack.edgeInsets = NSEdgeInsets(top: 40, left: 0,
                                         bottom: 40, right: 0)

        let lmStack = NSStackView(views: notesR)
        lmStack.orientation = .vertical
        lmStack.spacing = 8
        lmStack.alignment = .left
        lmStack.edgeInsets = NSEdgeInsets(top: 40, left: 0,
                                          bottom: 40, right: 0)
        var octavesL: [NSButton] = []
        var octavesR: [NSButton] = []        
        for (index, value) in octaves.enumerated()
        {
            let button = NSButton()
            button.title = String(format: "Octave %d", index)
            button.tag = index
            button.setButtonType(.switch)
            button.state = getOctave(index) ? .on : .off
            button.target = self
            button.action = #selector(octaveClicked)

            if (index < 5)
            {
                octavesL.append(button)
            }

            else
            {
                octavesR.append(button)
            }
        }

        let rmStack = NSStackView(views: octavesL)
        rmStack.orientation = .vertical
        rmStack.spacing = 8
        rmStack.alignment = .left
        rmStack.edgeInsets = NSEdgeInsets(top: 40, left: 0,
                                          bottom:40, right: 0)

        let rStack = NSStackView(views: octavesR)
        rStack.orientation = .vertical
        rStack.spacing = 8
        rStack.alignment = .left
        rStack.edgeInsets = NSEdgeInsets(top: 40, left: 0,
                                         bottom:40, right: 0)

        let hStack = NSStackView(views: [lStack, lmStack, rmStack, rStack])
        hStack.spacing = 8
        hStack.alignment = .top
        hStack.edgeInsets = NSEdgeInsets(top: 0, left: 60,
                                         bottom: 0, right: 60)
        let stackWidth = NSLayoutConstraint(item: lStack,
                                             attribute: .width,
                                             relatedBy: .equal,
                                             toItem: lmStack,
                                             attribute: .width,
                                             multiplier: 1,
                                             constant: 0)
        hStack.addConstraint(stackWidth)

        noteWindow = NSWindow(contentRect: .zero,
                              styleMask: [.titled, .closable],
                              backing: .buffered,
                              defer: true)
        noteWindow?.title = "Note Filters"
        noteWindow?.contentView = hStack
        noteWindow?.isReleasedWhenClosed = false
        noteWindow?.cascadeTopLeft(from:
                                     prefWindow!.cascadeTopLeft(from: .zero))
        noteWindow?.makeKeyAndOrderFront(self)
    }

    // This function compiles and appears to work but produces the
    // wrong result for a boolean array. Replaced by ObjC access
    // functions.
    // arrayFromTuple
    func arrayFromTuple<T, R>(tuple: T) -> [R]
    {
        let reflection = Mirror(reflecting: tuple)
        var array: [R] = []
        for i in reflection.children
        {
            array.append(i.value as! R)
        }

        return array
    }

    @objc func buttonClicked(sender: NSButton)
    {
        enum flags: Int
        {
            case kZoom
            case kFilt
            case kMult
            case kFund
            case kStrobe
            case kDown
            case kLock
            case kNote
        }

        print("Sender", sender, sender.state)
        switch sender.tag
        {
        case .kZoom :
            spectrumData.zoom = sender.state

        case .kFilter :
            audioData.filt = sender.state

        case .kMult :
            displayData.mult = sender.state

        case .kFund :
            audioData.fund = sender.state

        case .kStrobe :
            strobeData.enable = sender.state

        case .kDown :
            audioData.down = sender.state

        case .kLock :
            displayData.lock = sender.state

        case .kNote :
            audioData.note = sender.state

        default:
            break
        }
    }

    @objc func refChanged(sender: NSControl)
    {
        print("Sender", sender, sender.doubleValue)
        refText.doubleValue = sender.doubleValue
        refStep.doubleValue = sender.doubleValue
        audioData.reference = sender.doubleValue
        displayView.needsDisplay = true
    }

    @objc func noteClicked(sender: NSButton)
    {
        print("Sender", sender, sender.state)
        setNote(sender.state, sender.tag);
    }

    @objc func octaveClicked(sender: NSButton)
    {
        print("Sender", sender, sender.state)
        setNote(sender.state, sender.tag)
    }

    func getPreferences()
    {
        enum flags: Int
        {
            case kZoom
            case kFilter
            case kMult
            case kFund
            case kStrobe
            case kDown
            case kLock
            case kNote
        }

        let keys = ["Zoom", "Filter", "Mult", "Fund",
                    "Strobe", "Down", "Lock", "Note"]

        // Check defaults
        let defaults = UserDefaults.standard
        let ref = defaults.double(forKey: "Ref")
        if (ref == 0)
        {
            return
        }

        audioData.reference = ref
        for (index, key) in keys.enumerated()
        {
            switch index
            {
            case .kZoom :
                spectrumData.zoom = defaults.bool(forKey: key)

            case .kFilter :
                audioData.filter = defaults.bool(forKey: key)

            case .kMult :
                displayData.mult = defaults.bool(forKey: key)

            case .kFund :
                audioData.fund = defaults.bool(forKey: key)

            case .kStrobe :
                spectrumData.zoom = defaults.bool(forKey: key)

            case .kDown :
                spectrumData.zoom = defaults.bool(forKey: key)

            case .kLock :
                spectrumData.zoom = defaults.bool(forKey: key)

            case .kNote :
                spectrumData.zoom = defaults.bool(forKey: key)

            default :
                break
            }
        }
    }

    func savePreferences()
    {
        let keys = ["Zoom", "Filter", "Mult", "Fund",
                    "Strobe", "Down", "Lock", "Note"]
        let values = [spectrumData.zoom, audioData.filt,
                      displayData.mult, audioData.fund,
                      strobeData.enable, audioData.down,
                      displayData.lock, audioData.note]

        let defaults = UserDefaults.standard
        defaults.set(audioData.reference, "Ref")
        for (index, key) in keys.enumerated()
        {
            defaults.set(values[index], key)
        }
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
        savePreferences()
        timer.invalidate()
        ShutdownAudio()
    }
}
