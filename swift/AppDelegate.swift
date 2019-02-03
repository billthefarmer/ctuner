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

var zoomBox: NSButton!
var filtBox: NSButton!
var multBox: NSButton!
var strbBox: NSButton!
var downBox: NSButton!
var lockBox: NSButton!
var fundBox: NSButton!
var noteBox: NSButton!

var expandPopUp: NSPopUpButton!
var colourPopUp: NSPopUpButton!

var refText: NSTextField!
var refStep: NSStepper!

var transPopUp: NSPopUpButton!
var tempPopUp: NSPopUpButton!
var keyPopUp: NSPopUpButton!

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate
{
    @IBOutlet weak var window: NSWindow!

    var prefWindow: NSWindow!
    var noteWindow: NSWindow!

    var menu: NSMenu!

    var stack: NSStackView!
    var timer: Timer!

    // applicationDidFinishLaunching
    func applicationDidFinishLaunching(_ aNotification: Notification)
    {
        // Insert code here to initialize your application

        // Set up window
        window.setContentSize(NSMakeSize(400, 480))
        window.contentMinSize = NSMakeSize(400, 480)
        window.contentAspectRatio = NSMakeSize(1.0, 1.2)
        window.showsResizeIndicator = true
        window.collectionBehavior.insert(.fullScreenNone)

        // Find the menu
        menu = NSApp.mainMenu
        if (menu != nil)
        {
            var item = menu.item(withTitle: "Tuner")!
            if (item.hasSubmenu)
            {
                let subMenu = item.submenu!
                let subItem = subMenu.item(withTitle: "Preferences…")!
                subItem.target = self
                subItem.action = #selector(showPreferences)
            }

            item = menu.item(withTitle: "File")!
            if (item.hasSubmenu)
            {
                let subMenu = item.submenu!
                let subItem = subMenu.item(withTitle: "Print…")!
                subItem.target = self
                subItem.action = #selector(print)
            }
        }

        // Views
        scopeView = ScopeView()
        spectrumView = SpectrumView()
        displayView = DisplayView()
        strobeView = StrobeView()
        staffView = StaffView()
        meterView = MeterView()

        // Tooltips
        scopeView.toolTip = "Scope - click to filter audio"
        spectrumView.toolTip = "Spectrum - click to zoom"
        displayView.toolTip = "Display - click to lock"
        strobeView.toolTip = "Strobe - click for staff"
        staffView.toolTip = "Staff - click for strobe"
        meterView.toolTip = "Meter - click to lock"

        // Redraw policy
        scopeView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        spectrumView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        displayView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        strobeView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        staffView.layerContentsRedrawPolicy = .onSetNeedsDisplay
        meterView.layerContentsRedrawPolicy = .onSetNeedsDisplay

        // Stack
        stack = NSStackView(views: [scopeView, spectrumView, displayView,
                                    strobeView, staffView, meterView])

        // View height constraints
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

        let staffHeight = NSLayoutConstraint(item: staffView,
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

        // Add constraints
        stack.addConstraint(spectrumHeight)
        stack.addConstraint(displayHeight)
        stack.addConstraint(strobeHeight)
        stack.addConstraint(staffHeight)
        stack.addConstraint(meterHeight)

        // Config stack
        stack.orientation = .vertical
        stack.spacing = 8
        stack.edgeInsets = NSEdgeInsets(top: 20, left: 20,
                                        bottom: 20, right: 20)

        // Window content
        window.contentView = stack
        window.makeKeyAndOrderFront(self)
        window.makeFirstResponder(displayView)
        window.makeMain()

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

    // showPreferences
    @objc func showPreferences(sender: Any)
    {
        if (prefWindow != nil)
        {
            prefWindow.makeKeyAndOrderFront(self)
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

        let tags = [kZoom, kFilt, kMult, kFund,
                    kStrobe, kDown, kLock, kNote]

        var leftButtons: [NSButton] = []
        var rightButtons: [NSButton] = []
        for (index, label) in labels.enumerated()
        {
            let button = NSButton()
            button.title = label
            button.setButtonType(.switch)
            button.tag = tags[index]
            button.state = values[index] ? .on: .off
            button.target = self
            button.action = #selector(buttonClicked)

            switch tags[index]
            {
            case kZoom:
                zoomBox = button

            case kFilt:
                filtBox = button

            case kMult:
                multBox = button

            case kFund:
                fundBox = button

            case kStrobe:
                strbBox = button

            case kDown:
                downBox = button

            case kLock:
                lockBox = button

            case kNote:
                noteBox = button

            default:
                break
            }

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
        let expandLabel = NSTextField()
        expandLabel.stringValue = "Spectrum display expand:"
        expandLabel.isEditable = false
        expandLabel.isBordered = false
        expandLabel.drawsBackground = false
        expandPopUp = NSPopUpButton()
        expandPopUp.pullsDown = false
        expandPopUp.addItems(withTitles:
                               ["x 1", "x 2", "x 4", "x 8", "x 16"])
        expandPopUp.selectItem(at: Int(log2(Float(spectrumData.expand))))
        expandPopUp.tag = kExpand
        expandPopUp.target = self
        expandPopUp.action = #selector(popUpChanged)

        let expandRow = NSStackView(views: [expandLabel])
	expandRow.addView(expandPopUp, in: .trailing)
        expandRow.spacing = 8

        let colourLabel = NSTextField()
        colourLabel.stringValue = "Strobe colours:"
        colourLabel.isEditable = false
        colourLabel.isBordered = false
        colourLabel.drawsBackground = false
        colourPopUp = NSPopUpButton()
        colourPopUp.pullsDown = false
        colourPopUp.addItems(withTitles:
                         ["Blue/Cyan", "Olive/Aquamarine", "Magenta/Yellow"])
        colourPopUp.selectItem(at: Int(strobeData.colours))
        colourPopUp.tag = kColour
        colourPopUp.target = self
        colourPopUp.action = #selector(popUpChanged)

        let colourRow = NSStackView(views: [colourLabel])
	colourRow.addView(colourPopUp, in: .trailing)
        colourRow.spacing = 8

        let refLabel = NSTextField()
        refLabel.stringValue = "Reference frequency:"
        refLabel.isEditable = false
        refLabel.isBordered = false
        refLabel.drawsBackground = false
        refText = NSTextField()
        refText.tag = kRefText
        refText.doubleValue = audioData.reference
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

        let refRow = NSStackView(views: [refLabel, refText, refStep])
        refRow.spacing = 8

        let transLabel = NSTextField()
        transLabel.stringValue = "Transpose display:"
        transLabel.isEditable = false
        transLabel.isBordered = false
        transLabel.drawsBackground = false
        transPopUp = NSPopUpButton()
        transPopUp.pullsDown = false
        transPopUp.addItems(withTitles:
                              ["+6[Key:F\u{266F}]", "+5[Key:F]", "+4[Key:E]",
                               "+3[Key:E\u{266D}]", "+2[Key:D]",
			       "+1[Key:C\u{266F}]", "+0[Key:C]", "-1[Key:B]",
			       "-2[Key:B\u{266D}]", "-3[Key:A]",
			       "-4[Key:A\u{266D}]", "-5[Key:G]",
                               "-6[Key:F\u{266F}]"])
        transPopUp.selectItem(at: Int(displayData.trans))
        transPopUp.tag = kTrans
        transPopUp.target = self
        transPopUp.action = #selector(popUpChanged)

        let transRow = NSStackView(views: [transLabel])
	transRow.addView(transPopUp, in: .trailing)
        transRow.spacing = 8

        let tempLabel = NSTextField()
        tempLabel.stringValue = "Temperament:"
        tempLabel.isEditable = false
        tempLabel.isBordered = false
        tempLabel.drawsBackground = false
        tempPopUp = NSPopUpButton()
        tempPopUp.pullsDown = false
        tempPopUp.addItems(withTitles:
                             ["Kirnberger II", "Kirnberger III",
                              "Werckmeister III", "Werckmeister IV",
                              "Werckmeister V", "Werckmeister VI",
                              "Bach (Klais)", "Just (Barbour)",
                              "Equal Temperament", "Pythagorean",
                              "Van Zwolle", "Meantone (-1/4)",
                              "Silbermann (-1/6)", "Salinas (-1/3)",
                              "Zarlino (-2/7)", "Rossi (-1/5)",
                              "Rossi (-2/9)", "Rameau (-1/4)",
                              "Kellner", "Vallotti",
                              "Young II", "Bendeler III",
                              "Neidhardt I", "Neidhardt II",
                              "Neidhardt III", "Bruder 1829",
                              "Barnes 1977", "Lambert 1774",
                              "Schlick (H. Vogel)", "Meantone # (-1/4)",
                              "Meantone b (-1/4)", "Lehman-Bach"])
        tempPopUp.selectItem(at: Int(audioData.temper))
        tempPopUp.tag = kTemp
        tempPopUp.target = self
        tempPopUp.action = #selector(popUpChanged)

        let tempRow = NSStackView(views: [tempLabel])
	tempRow.addView(tempPopUp, in: .trailing)
        tempRow.spacing = 8

        let keyLabel = NSTextField()
        keyLabel.stringValue = "Temperament key:"
        keyLabel.isEditable = false
        keyLabel.isBordered = false
        keyLabel.drawsBackground = false
        keyPopUp = NSPopUpButton()
        keyPopUp.pullsDown = false
        keyPopUp.addItems(withTitles:
                            ["C", "C\u{266F}", "D", "E\u{266D}",
                             "E", "F", "F\u{266F}", "G",
                             "A\u{266D}", "A", "B\u{266D}", "B"])
        keyPopUp.selectItem(at: Int(audioData.key))
        keyPopUp.tag = kKey
        keyPopUp.target = self
        keyPopUp.action = #selector(popUpChanged)
        let button = NSButton()
        button.title = "Filters…"
        button.setButtonType(.momentaryPushIn)
        button.bezelStyle = .rounded
        button.target = self
        button.action = #selector(showNotes)

        let keyRow = NSStackView(views: [keyLabel, keyPopUp])
	keyRow.addView(button, in: .trailing)
        keyRow.spacing = 8

        stack = NSStackView(views: [hStack, expandRow, colourRow, refRow,
                                    transRow, tempRow, keyRow])
        stack.orientation = .vertical
        stack.alignment = .left
        stack.spacing = 8
        stack.edgeInsets = NSEdgeInsets(top: 40, left: 40,
                                        bottom: 40, right: 40)

        prefWindow = NSWindow(contentRect: .zero,
                              styleMask: [.titled, .closable],
                              backing: .buffered,
                              defer: true)
        prefWindow.title = "Tuner Preferences"

        prefWindow.contentView = stack
        prefWindow.isReleasedWhenClosed = false
        prefWindow.cascadeTopLeft(from: window.cascadeTopLeft(from: .zero))
        prefWindow.makeKeyAndOrderFront(self)
    }

    @objc func showNotes(sender: NSButton)
    {
        if (noteWindow != nil)
        {
            noteWindow.makeKeyAndOrderFront(self)
            return
        }

        let labels = ["C", "C#", "D", "Eb", "E", "F",
                      "F#", "G", "Ab", "A", "Bb", "B"]

        var leftNotes: [NSButton] = []
        var rightNotes: [NSButton] = []
        for (index, label) in labels.enumerated()
        {
            let button = NSButton()
            button.title = label
            button.tag = index
            button.setButtonType(.switch)
            button.state = getNote(Int32(index)) ? .on: .off
            button.target = self
            button.action = #selector(noteClicked)

            if (index < labels.count / 2)
            {
                leftNotes.append(button)
            }

            else
            {
                rightNotes.append(button)
            }
        }

        let lStack = NSStackView(views: leftNotes)
        lStack.orientation = .vertical
        lStack.spacing = 8
        lStack.alignment = .left
        lStack.edgeInsets = NSEdgeInsets(top: 40, left: 0,
                                         bottom: 40, right: 0)

        let lmStack = NSStackView(views: rightNotes)
        lmStack.orientation = .vertical
        lmStack.spacing = 8
        lmStack.alignment = .left
        lmStack.edgeInsets = NSEdgeInsets(top: 40, left: 0,
                                          bottom: 40, right: 0)
        var leftOctaves: [NSButton] = []
        var rightOctaves: [NSButton] = []
        for index in 0 ... 8
        {
            let button = NSButton()
            button.title = String(format: "Octave %d", index)
            button.tag = index
            button.setButtonType(.switch)
            button.state = getOctave(Int32(index)) ? .on: .off
            button.target = self
            button.action = #selector(octaveClicked)

            if (index < 5)
            {
                leftOctaves.append(button)
            }

            else
            {
                rightOctaves.append(button)
            }
        }

        let rmStack = NSStackView(views: leftOctaves)
        rmStack.orientation = .vertical
        rmStack.spacing = 8
        rmStack.alignment = .left
        rmStack.edgeInsets = NSEdgeInsets(top: 40, left: 0,
                                          bottom:40, right: 0)

        let rStack = NSStackView(views: rightOctaves)
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
        noteWindow.title = "Tuner Note Filters"
        noteWindow.contentView = hStack
        noteWindow.isReleasedWhenClosed = false
        noteWindow.cascadeTopLeft(from:
                                     prefWindow!.cascadeTopLeft(from: .zero))
        noteWindow.makeKeyAndOrderFront(self)
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

    // buttonClicked
    @objc func buttonClicked(sender: NSButton)
    {
        switch sender.tag
        {
        case kZoom:
            spectrumData.zoom = (sender.state == .on) ? true: false

        case kFilt:
            audioData.filt = (sender.state == .on) ? true: false

        case kMult:
            displayData.mult = (sender.state == .on) ? true: false
            displayView.needsDisplay = true

        case kFund:
            audioData.fund = (sender.state == .on) ? true: false

        case kStrobe:
            strobeData.enable = (sender.state == .on) ? true: false
            staffData.enable = !strobeData.enable

            strobeView.isHidden = !strobeData.enable
            staffView.isHidden = !staffData.enable

        case kDown:
            audioData.down = (sender.state == .on) ? true: false

        case kLock:
            displayData.lock = (sender.state == .on) ? true: false
            displayView.needsDisplay = true

        case kNote:
            audioData.note = (sender.state == .on) ? true: false

        default:
            break
        }
    }

    @objc func popUpChanged(sender: NSPopUpButton)
    {
        let index = sender.indexOfSelectedItem

        switch sender.tag
        {
        case kColour:
            strobeData.colours = Int32(index)
            strobeData.changed = true

        case kTrans:
            displayData.trans = Int32(index)

        case kTemp:
            audioData.temper = Int32(index)

        case kKey:
            audioData.key = Int32(index)

        default:
            break
        }
    }

    // refChanged
    @objc func refChanged(sender: NSControl)
    {
        let value = sender.doubleValue
        switch sender.tag
        {
        case kRefStep:
            refText.doubleValue = value

        case kRefText:
            refStep.doubleValue = value

        default:
            break
        }

        audioData.reference = value
        displayView.needsDisplay = true
    }

    // noteClicked
    @objc func noteClicked(sender: NSButton)
    {
        setNote((sender.state == .on) ? true: false, Int32(sender.tag));
    }

    // octaveClicked
    @objc func octaveClicked(sender: NSButton)
    {
        setOctave((sender.state == .on) ? true: false, Int32(sender.tag))
    }

    @objc func print(sender: Any)
    {
        window.printWindow(sender)
    }

    // getPreferences
    func getPreferences()
    {
        let keys = ["Zoom", "Filter", "Strobe", "Down"]

        // Check defaults
        let defaults = UserDefaults.standard
        let ref = defaults.double(forKey: "Ref")
        if (ref == 0)
        {
            audioData.reference = Double(kA5Reference)
            audioData.temper = 8
            displayData.trans = 6
            spectrumData.zoom = true
            spectrumData.expand = 1
            strobeData.colours = 1
            return
        }

        audioData.reference = ref
        audioData.temper = Int32(defaults.integer(forKey: "Temper"))
        audioData.key = Int32(defaults.integer(forKey: "Key"))
        displayData.trans = Int32(defaults.integer(forKey: "Trans"))
        strobeData.colours = Int32(defaults.integer(forKey: "Colours"))
        for (index, key) in keys.enumerated()
        {
            switch index
            {
            case 0:
                spectrumData.zoom = defaults.bool(forKey: key)

            case 1:
                audioData.filt = defaults.bool(forKey: key)

            case 2:
                strobeData.enable = defaults.bool(forKey: key)

            case 3:
                audioData.down = defaults.bool(forKey: key)

            default:
                break
            }
        }

        strobeView.isHidden = !strobeData.enable
        staffData.enable = !strobeData.enable
        staffView.isHidden = !staffData.enable
    }

    // savePreferences
    func savePreferences()
    {
        let keys = ["Zoom", "Filter", "Strobe", "Down"]
        let values = [spectrumData.zoom, audioData.filt,
                      strobeData.enable, audioData.down]

        let defaults = UserDefaults.standard
        defaults.set(audioData.reference, forKey: "Ref")
        defaults.set(audioData.temper, forKey: "Temper")
        defaults.set(audioData.key, forKey: "Key")
        defaults.set(displayData.trans, forKey: "Trans")
        defaults.set(strobeData.colours, forKey: "Colours")
        for (index, key) in keys.enumerated()
        {
            defaults.set(values[index], forKey: key)
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
          .takeRetainedValue() as String:
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
