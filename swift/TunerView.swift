//
//  TunerView.swift
//  Tuner
//
//  Created by Bill Farmer on 08/10/2017.
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

class TunerView: NSView
{
    let kOctave: Int32 = 12

    let trans: [Int32] =
      [6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6]

    let notes =
      ["C", "C", "D", "E", "E", "F",
       "F", "G", "A", "A", "B", "B"]

    let sharps =
      ["", "\u{0266F}", "", "\u{0266D}", "", "",
       "\u{0266F}", "", "\u{0266D}", "", "\u{0266D}", ""]

    var rect: NSRect = .zero
    var width: CGFloat = 0
    var height: CGFloat = 0

    // draw
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        rect = DrawEdge(dirtyRect)
        width = rect.width
        height = rect.height
    }

    // DrawEdge
    func DrawEdge(_ rect: NSRect) -> NSRect
    {
        // Save context
        NSGraphicsContext.saveGraphicsState()

        // Set colour
        NSColor.lightGray.set()

        // Draw edge
        var path = NSBezierPath(roundedRect: rect, xRadius: 8, yRadius: 8)
        path.fill()

        // Restore context before clip
        NSGraphicsContext.restoreGraphicsState()

        // Create inset
        let inset = NSInsetRect(rect, 2, 2)
        path = NSBezierPath(roundedRect: inset, xRadius: 8, yRadius: 8)
        path.setClip()

        return inset
    }

    // This, IMHO is a kludge because you ought to be able to do this
    // in AppDelegate rather than one of the views
    // keyDown
    override func keyDown(with event: NSEvent)
    {
        let key = event.characters!

        switch key.lowercased()
        {
        case "c":
            // Create string
            let string =
              String(format:
                       "%@%@%d %+4.2lf\u{00A2} %4.2lfHz %4.2lfHz %4.2lfHz\n",
                     notes[Int(disp.n - trans[Int(disp.trans)] +
                                 kOctave) % notes.endIndex],
		     sharps[Int(disp.n - trans[Int(disp.trans)] +
                                  kOctave) % sharps.endIndex],
                     disp.n / kOctave, disp.c * 100.0,
                     disp.fr, disp.f,
                     disp.f - disp.fr)

            // Put it on the pasteboard
            let pboard = NSPasteboard.general
            pboard.clearContents()
            pboard.setString(string, forType: .string)

        case "k":
            // Change colour
            strobeView.colour += 1
            if (strobeView.colour >= strobeView.kMaxColours)
            {
                strobeView.colour = 0
            }

            // Update popup
            if (colourPopUp != nil)
            {
                colourPopUp.selectItem(at: strobeView.colour)
            }

            // Update strobe view
            strobe.changed = true
            strobeView.needsDisplay = true

        case "d":
            // Update downsampling
            audio.down = !audio.down
            if (downBox != nil)
            {
                downBox.state = audio.down ? .on: .off
            }
            // Update spectrum view
            spectrumView.needsDisplay = true

        case "f":
            // Update filter
            audio.filt = !audio.filt
            if (filtBox != nil)
            {
                filtBox.state = audio.filt ? .on: .off
            }
            // Update scope view
            scopeView.needsDisplay = true

        case "l":
            // Update display lock
            displayView.lock = !displayView.lock
            if (lockBox != nil)
            {
                lockBox.state = displayView.lock ? .on: .off
            }
            // Update display
            displayView.needsDisplay = true

        case "m":
            // Update multiple notes
            displayView.mult = !displayView.mult
            if (multBox != nil)
            {
                multBox.state = displayView.mult ? .on: .off
            }
            // Update display
            displayView.needsDisplay = true

        case "s":
            // Switch views
            strobeView.enable = !strobeView.enable
            staffView.enable = !strobeView.enable
            // Hide views, animation here
            if (strbBox != nil)
            {
                strbBox.state = strobeView.enable ? .on: .off
            }

        case "z":
            // Update zoom
            spectrum.zoom = !spectrum.zoom
            if (zoomBox != nil)
            {
                zoomBox.state = spectrum.zoom ? .on: .off
            }
            // Update spectrum view
            spectrumView.needsDisplay = true

        case "+":
            // Expand spectrum
            spectrum.expand *= 2
            if (spectrum.expand > kMaxExpand)
            {
                spectrum.expand = kMinExpand
            }
            if (expandPopUp != nil)
            {
                expandPopUp
                  .selectItem(at: Int(log2(Float(spectrum.expand))))
            }
            // Update spectrum view
            spectrumView.needsDisplay = true

        case "-":
            // Contract spectrum
            spectrum.expand /= 2
            if (spectrum.expand < kMinExpand)
            {
                spectrum.expand = kMaxExpand
            }
            if (expandPopUp != nil)
            {
                expandPopUp.selectItem(at:
                                         Int(log2(Float(spectrum.expand))))
            }
            // Update spectrum view
            spectrumView.needsDisplay = true

        default:
            NSLog("Key %@ %@", key, event.charactersIgnoringModifiers ?? "")
            break
        }
    }
}
