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

import Cocoa

// Apparently these aren't declared anywhere
@objc protocol EditMenuActions
{
    func redo(_ sender: AnyObject)
    func undo(_ sender: AnyObject)
}

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

// AppDelegate
class AppDelegate: NSObject, NSApplicationDelegate
{
    let kWidth  = CGFloat(400)
    let kHeight = CGFloat(480)

    var window: NSWindow!

    var prefWindow: NSWindow!
    var noteWindow: NSWindow!

    var stack: NSStackView!
    var timer: Timer!

    lazy var applicationName: String =
    {
	if let bundleName = Bundle.main.object(forInfoDictionaryKey: "CFBundleName")
        {
            if let bundleNameAsString = bundleName as? String
            {
                return bundleNameAsString
	    }

	    else
            {
                Swift.print("CFBundleName not a String!")
	    }
	}

	else
        {
	    Swift.print("CFBundleName nil!")
	}

	return NSLocalizedString("Tuner", comment: "The name of this application")
    }()

    // applicationWillFinishLaunching
    func applicationWillFinishLaunching(_ notification: Notification)
    {
        populateMainMenu()
    }

    // populateMainMenu
    func populateMainMenu()
    {
        let mainMenu = NSMenu(title: "MainMenu")

	// The titles of the menu items are for identification
	// purposes only and shouldn't be localized.  The strings in
	// the menu bar come from the submenu titles, except for the
	// application menu, whose title is ignored at runtime.
	var menuItem = mainMenu.addItem(withTitle: "Application",
                                        action: nil,
                                        keyEquivalent: "")
	var submenu = NSMenu(title: "Application")
	populateApplicationMenu(submenu)
	mainMenu.setSubmenu(submenu, for: menuItem)

	menuItem = mainMenu.addItem(withTitle: "File", action: nil, keyEquivalent: "")
	submenu = NSMenu(title: NSLocalizedString("File", comment: "File menu"))
	populateFileMenu(submenu)
	mainMenu.setSubmenu(submenu, for: menuItem)

	menuItem = mainMenu.addItem(withTitle: "Edit", action: nil, keyEquivalent: "")
	submenu = NSMenu(title: NSLocalizedString("Edit", comment: "Edit menu"))
	populateEditMenu(submenu)
	mainMenu.setSubmenu(submenu, for: menuItem)

	menuItem = mainMenu.addItem(withTitle: "View", action: nil, keyEquivalent: "")
	submenu = NSMenu(title: NSLocalizedString("View", comment: "View menu"))
	populateViewMenu(submenu)
	mainMenu.setSubmenu(submenu, for: menuItem)

	menuItem = mainMenu.addItem(withTitle: "Window", action: nil, keyEquivalent: "")
	submenu = NSMenu(title: NSLocalizedString("Window", comment: "Window menu"))
	populateWindowMenu(submenu)
	mainMenu.setSubmenu(submenu, for: menuItem)
	NSApp.windowsMenu = submenu

	menuItem = mainMenu.addItem(withTitle: "Help", action: nil, keyEquivalent: "")
	submenu = NSMenu(title: NSLocalizedString("Help", comment: "View menu"))
	populateHelpMenu(submenu)
	mainMenu.setSubmenu(submenu, for: menuItem)

	NSApp.mainMenu = mainMenu
    }

    // populateApplicationMenu
    func populateApplicationMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("About", comment: "About menu item") + " " + applicationName
	var menuItem = menu.addItem(withTitle: title,
                                    action: #selector(NSApplication.orderFrontStandardAboutPanel(_: )),
                                    keyEquivalent: "")
	menuItem.target = NSApp

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Preferences…", comment: "Preferences menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(showPreferences),
                                keyEquivalent: ",")

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Services", comment: "Services menu item")
	menuItem = menu.addItem(withTitle: title, action: nil, keyEquivalent: "")
	let servicesMenu = NSMenu(title: "Services")
	menu.setSubmenu(servicesMenu, for: menuItem)
	NSApp.servicesMenu = servicesMenu

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Hide", comment: "Hide menu item") + " " + applicationName
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSApplication.hide(_: )),
                                keyEquivalent: "h")
	menuItem.target = NSApp

	title = NSLocalizedString("Hide Others", comment: "Hide Others menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSApplication.hideOtherApplications(_: )),
                                keyEquivalent: "h")
	menuItem.keyEquivalentModifierMask = [.command, .option]
	menuItem.target = NSApp

	title = NSLocalizedString("Show All", comment: "Show All menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSApplication.unhideAllApplications(_: )),
                                keyEquivalent: "")
	menuItem.target = NSApp

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Quit", comment: "Quit menu item") + " " + applicationName
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSApplication.terminate(_: )),
                                keyEquivalent: "q")
	menuItem.target = NSApp
    }

    // populateFileMenu
    func populateFileMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("Print…", comment: "Print menu item")
	menu.addItem(withTitle: title,
                     action: #selector(print),
                     keyEquivalent: "p")

	menu.addItem(NSMenuItem.separator())

        title = NSLocalizedString("Close Window", comment: "Close Window menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSWindow.performClose(_: )),
                     keyEquivalent: "w")
    }

    // populateEditMenu
    func populateEditMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("Undo", comment: "Undo menu item")
	menu.addItem(withTitle: title,
                     action: #selector(EditMenuActions.undo(_: )),
                     keyEquivalent: "z")

	title = NSLocalizedString("Redo", comment: "Redo menu item")
	menu.addItem(withTitle: title,
                     action: #selector(EditMenuActions.redo(_: )),
                     keyEquivalent: "Z")

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Cut", comment: "Cut menu item")
	menu.addItem(withTitle: title, action: #selector(NSText.cut(_: )), keyEquivalent: "x")

	title = NSLocalizedString("Copy", comment: "Copy menu item")
	menu.addItem(withTitle: title, action: #selector(NSText.copy(_: )), keyEquivalent: "c")

	title = NSLocalizedString("Paste", comment: "Paste menu item")
	menu.addItem(withTitle: title, action: #selector(NSText.paste(_: )), keyEquivalent: "v")

	title = NSLocalizedString("Paste and Match Style",
                                  comment: "Paste and Match Style menu item")
	var menuItem = menu.addItem(withTitle: title,
                                    action: #selector(NSTextView.pasteAsPlainText(_: )),
                                    keyEquivalent: "V")
	menuItem.keyEquivalentModifierMask = [.command, .option]

	title = NSLocalizedString("Delete", comment: "Delete menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSText.delete(_: )),
                     keyEquivalent: "\u{8}") // backspace

	title = NSLocalizedString("Select All", comment: "Select All menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSText.selectAll(_: )),
                     keyEquivalent: "a")

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Find", comment: "Find menu item")
	menuItem = menu.addItem(withTitle: title, action: nil, keyEquivalent: "")
	let findMenu = NSMenu(title: "Find")
	populateFindMenu(findMenu)
	menu.setSubmenu(findMenu, for: menuItem)

	title = NSLocalizedString("Spelling", comment: "Spelling menu item")
	menuItem = menu.addItem(withTitle: title, action: nil, keyEquivalent: "")
	let spellingMenu = NSMenu(title: "Spelling")
	populateSpellingMenu(spellingMenu)
	menu.setSubmenu(spellingMenu, for: menuItem)
    }

    // populateFindMenu
    func populateFindMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("Find…", comment: "Find… menu item")
	var menuItem = menu.addItem(withTitle: title,
                                    action: #selector(NSResponder.performTextFinderAction(_: )),
                                    keyEquivalent: "f")
	menuItem.tag = NSTextFinder.Action.showFindInterface.rawValue

	title = NSLocalizedString("Find Next", comment: "Find Next menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSResponder.performTextFinderAction(_: )),
                                keyEquivalent: "g")
	menuItem.tag = NSTextFinder.Action.nextMatch.rawValue

	title = NSLocalizedString("Find Previous", comment: "Find Previous menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSResponder.performTextFinderAction(_: )),
                                keyEquivalent: "G")
	menuItem.tag = NSTextFinder.Action.previousMatch.rawValue

	title = NSLocalizedString("Use Selection for Find",
                                  comment: "Use Selection for Find menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSResponder.performTextFinderAction(_: )),
                                keyEquivalent: "e")
	menuItem.tag = NSTextFinder.Action.setSearchString.rawValue

	title = NSLocalizedString("Jump to Selection", comment: "Jump to Selection menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSResponder.centerSelectionInVisibleArea(_: )),
                     keyEquivalent: "j")
    }

    // populateSpellingMenu
    func populateSpellingMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("Spelling…", comment: "Spelling… menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSText.showGuessPanel(_: )),
                     keyEquivalent: ": ")

	title = NSLocalizedString("Check Spelling", comment: "Check Spelling menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSText.checkSpelling(_: )),
                     keyEquivalent: ";")

	title = NSLocalizedString("Check Spelling as You Type",
                                  comment: "Check Spelling as You Type menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSTextView.toggleContinuousSpellChecking(_: )),
                     keyEquivalent: "")
    }

    // populateViewMenu
    func populateViewMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("Show Toolbar", comment: "Show Toolbar menu item")
	var menuItem = menu.addItem(withTitle: title,
                                    action: #selector(NSWindow.toggleToolbarShown(_: )),
                                    keyEquivalent: "t")
	menuItem.keyEquivalentModifierMask = [.command, .option]

	title = NSLocalizedString("Customize Toolbar…", comment: "Customize Toolbar… menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSWindow.runToolbarCustomizationPalette(_: )),
                     keyEquivalent: "")

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Enter Full Screen", comment: "Enter Full Screen menu item")
	menuItem = menu.addItem(withTitle: title,
                                action: #selector(NSWindow.toggleFullScreen(_: )),
                                keyEquivalent: "f")
	menuItem.keyEquivalentModifierMask = [.command, .control]
    }

    // populateWindowMenu
    func populateWindowMenu(_ menu: NSMenu)
    {
        var title = NSLocalizedString("Minimize", comment: "Minimize menu item")
	menu.addItem(withTitle: title,
                     action: #selector(NSWindow.performMiniaturize(_: )),
                     keyEquivalent: "m")

	title = NSLocalizedString("Zoom", comment: "Zoom menu item")
	menu.addItem(withTitle: title, action: #selector(NSWindow.performZoom(_: )),
                     keyEquivalent: "")

	menu.addItem(NSMenuItem.separator())

	title = NSLocalizedString("Bring All to Front", comment: "Bring All to Front menu item")
	let menuItem = menu.addItem(withTitle: title,
                                    action: #selector(NSApplication.arrangeInFront(_: )),
                                    keyEquivalent: "")
	menuItem.target = NSApp
    }

    // populateHelpMenu
    func populateHelpMenu(_ menu: NSMenu)
    {
        let title = applicationName + " " + NSLocalizedString("Help", comment: "Help menu item") 
	let menuItem = menu.addItem(withTitle: title,
                                    action: #selector(NSApplication.showHelp(_: )),
                                    keyEquivalent: "?")
	menuItem.target = NSApp
    }

    // applicationDidFinishLaunching
    func applicationDidFinishLaunching(_ aNotification: Notification)
    {
        // Insert code here to initialize your application

        // Set up window
        window = NSWindow(contentRect: NSMakeRect(0, 0, kWidth, kHeight),
                          styleMask: [.titled, .closable, .resizable, .miniaturizable],
                          backing: .buffered,
                          defer: true)
        window.contentMinSize = NSMakeSize(kWidth, kHeight)
        window.contentAspectRatio = NSMakeSize(kWidth, kHeight)
        window.showsResizeIndicator = true
        window.collectionBehavior.insert(.fullScreenNone)
        window.title = applicationName
        window.center()

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
        let spectrumHeight = NSLayoutConstraint(item: spectrumView as Any,
                                                attribute: .height,
                                                relatedBy: .equal,
                                                toItem: scopeView as Any,
                                                attribute: .height,
                                                multiplier: 1,
                                                constant: 0)

        let displayHeight = NSLayoutConstraint(item: displayView as Any,
                                               attribute: .height,
                                               relatedBy: .equal,
                                               toItem: spectrumView as Any,
                                               attribute: .height,
                                               multiplier: 3.25,
                                               constant: 0)

        let strobeHeight = NSLayoutConstraint(item: strobeView as Any,
                                              attribute: .height,
                                              relatedBy: .equal,
                                              toItem: spectrumView as Any,
                                              attribute: .height,
                                              multiplier: 1.375,
                                              constant: 0)

        let staffHeight = NSLayoutConstraint(item: staffView as Any,
                                             attribute: .height,
                                             relatedBy: .equal,
                                             toItem: spectrumView as Any,
                                             attribute: .height,
                                             multiplier: 1.375,
                                             constant: 0)

        let meterHeight = NSLayoutConstraint(item: meterView as Any,
                                             attribute: .height,
                                             relatedBy: .equal,
                                             toItem: strobeView as Any,
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

        let kExpanse = ["x 1", "x 2", "x 4", "x 8", "x 16"]
        let kColours = ["Blue/Cyan", "Olive/Aquamarine", "Magenta/Yellow"]
        let kTranspose = ["+6[Key:F\u{266F}]", "+5[Key:F]", "+4[Key:E]",
                          "+3[Key:E\u{266D}]", "+2[Key:D]",
		          "+1[Key:C\u{266F}]", "+0[Key:C]", "-1[Key:B]",
		          "-2[Key:B\u{266D}]", "-3[Key:A]",
		          "-4[Key:A\u{266D}]", "-5[Key:G]",
                          "-6[Key:F\u{266F}]"]
        let kTemperaments = ["Kirnberger II", "Kirnberger III",
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
                             "Meantone b (-1/4)", "Lehman-Bach"]
        let kKeys = ["C", "C\u{266F}", "D", "E\u{266D}",
                     "E", "F", "F\u{266F}", "G",
                     "A\u{266D}", "A", "B\u{266D}", "B"]

        let labels = ["Zoom spectrum", "Filter audio",
                      "Multiple notes", "Fundamental filter",
                      "Display strobe", "Downsample",
                      "Lock display", "Note filter"]

        let values = [spectrumView.zoom, audio.filt,
                      displayView.mult, audio.fund,
                      strobeView.enable, audio.down,
                      displayView.lock, audio.note]

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
        expandPopUp.addItems(withTitles: kExpanse)
        expandPopUp.selectItem(at: Int(log2(Float(spectrumView.expand))))
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
        colourPopUp.addItems(withTitles: kColours)
        colourPopUp.selectItem(at: strobeView.colour)
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
        refText.doubleValue = audio.reference
        refText.target = self
        refText.action = #selector(refChanged)
        refStep = NSStepper()
        refStep.tag = kRefStep
        refStep.maxValue = 480.0
        refStep.minValue = 420.0
        refStep.increment = 1.0
        refStep.doubleValue = audio.reference
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
        transPopUp.addItems(withTitles: kTranspose)
        transPopUp.selectItem(at: displayView.trans)
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
        tempPopUp.addItems(withTitles: kTemperaments)
        tempPopUp.selectItem(at: Int(audio.temper))
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
        keyPopUp.addItems(withTitles:kKeys)
        keyPopUp.selectItem(at: Int(audio.key))
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

    // showNotes
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
            spectrumView.zoom = (sender.state == .on) ? true: false
            spectrumView.needsDisplay = true

        case kFilt:
            audio.filt = (sender.state == .on) ? true: false
            scopeView.needsDisplay = true

        case kMult:
            displayView.mult = (sender.state == .on) ? true: false
            displayView.needsDisplay = true

        case kFund:
            audio.fund = (sender.state == .on) ? true: false
            scopeView.needsDisplay = true

        case kStrobe:
            strobeView.enable = (sender.state == .on) ? true: false
            staffView.enable = !strobeView.enable
            // Hide views, animation here
            strobeView.isHidden = !strobeView.enable
            staffView.isHidden = !staffView.enable
            strobeView.needsDisplay = true
            staffView.needsDisplay = true

        case kDown:
            audio.down = (sender.state == .on) ? true: false
            spectrumView.needsDisplay = true

        case kLock:
            displayView.lock = (sender.state == .on) ? true: false
            displayView.needsDisplay = true

        case kNote:
            audio.note = (sender.state == .on) ? true: false
            spectrumView.needsDisplay = true

        default:
            break
        }
    }

    // popUpChanged
    @objc func popUpChanged(sender: NSPopUpButton)
    {
        let index = sender.indexOfSelectedItem

        switch sender.tag
        {
        case kColour:
            // Update strobe view
            strobeView.colour = index
            strobeView.changed = true
            strobeView.needsDisplay = true

        case kTrans:
            // Update display
            displayView.trans = index
            displayView.needsDisplay = true
            staffView.needsDisplay = true

        case kTemp:
            // Update display
            audio.temper = Int32(index)
            displayView.needsDisplay = true
            staffView.needsDisplay = true

        case kKey:
            // Update display
            audio.key = Int32(index)
            displayView.needsDisplay = true
            staffView.needsDisplay = true

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

        audio.reference = value
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

    // print
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
            audio.reference = Double(kA5Reference)
            audio.temper = 8
            displayView.trans = 6
            spectrumView.zoom = true
            spectrumView.expand = 1
            strobeView.enable = false
            strobeView.colour = 1
            return
        }

        audio.reference = ref
        audio.temper = Int32(defaults.integer(forKey: "Temper"))
        audio.key = Int32(defaults.integer(forKey: "Key"))
        displayView.trans = defaults.integer(forKey: "Trans")
        strobeView.colour = defaults.integer(forKey: "Colours")
        for (index, key) in keys.enumerated()
        {
            switch index
            {
            case 0:
                spectrumView.zoom = defaults.bool(forKey: key)
                spectrumView.needsDisplay = true

            case 1:
                audio.filt = defaults.bool(forKey: key)
                scopeView.needsDisplay = true

            case 2:
                strobeView.enable = defaults.bool(forKey: key)
                staffView.enable = !strobeView.enable
                strobeView.isHidden = !strobeView.enable
                staffView.isHidden = !staffView.enable
                strobeView.needsDisplay = true
                staffView.needsDisplay = true

            case 3:
                audio.down = defaults.bool(forKey: key)

            default:
                break
            }
        }
    }

    // savePreferences
    func savePreferences()
    {
        let keys = ["Zoom", "Filter", "Strobe", "Down"]
        let values = [spectrumView.zoom, audio.filt,
                      strobeView.enable, audio.down]

        let defaults = UserDefaults.standard
        defaults.set(audio.reference, forKey: "Ref")
        defaults.set(audio.temper, forKey: "Temper")
        defaults.set(audio.key, forKey: "Key")
        defaults.set(displayView.trans, forKey: "Trans")
        defaults.set(strobeView.colour, forKey: "Colours")
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

        let error = (status > 0) ? NSFileTypeForHFSTypeCode(OSType(status))! :
          String(utf8String: AudioUnitErrString(status))!

        alert.informativeText = String(format: "%@ %@ (%x)", informativeText,
                                       error, status)
        alert.runModal()
    }

    // applicationShouldTerminateAfterLastWindowClosed
    func applicationShouldTerminateAfterLastWindowClosed(_ sender:
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

    // applicationSupportsSecureRestorableState
    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool
    {
        return true
    }
}
