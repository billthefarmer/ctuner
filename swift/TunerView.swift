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
    var rect: NSRect = .zero
    var width: CGFloat = 0
    var height: CGFloat = 0

    // draw
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        rect = DrawEdge(dirtyRect)
        width = NSWidth(rect)
        height = NSHeight(rect)
    }

    // DrawEdge
    func DrawEdge(_ rect: NSRect) -> NSRect
    {
        // Save context
        NSGraphicsContext.saveGraphicsState()

        // Set colour
        NSColor.gray.set()

        // Draw edge
        let path = NSBezierPath(roundedRect: rect, xRadius: 8, yRadius: 8)
        path.lineWidth = 2
        path.stroke()

        // Restore context before clip
        NSGraphicsContext.restoreGraphicsState()

        // Create inset
        let inset = NSInsetRect(rect, 2, 2)
        __NSRectClip(inset)

        return inset
    }

    // This IMHO is a kludge because you ought to be able to do this
    // in AppDelegate rather than one of the views
    // keyDown
    override func keyDown(with event: NSEvent)
    {
        let key = event.characters!

        switch key.lowercased()
        {
        case "c", "k":
            strobeData.colours += 1
            if (strobeData.colours >= strobeView.kMaxColours)
            {
                strobeData.colours = 0
            }

            if (strobePopUp != nil)
            {
                strobePopUp.selectItem(at: Int(strobeData.colours))
            }
            strobeData.changed = true
            strobeView.needsDisplay = true

        case "d":
            audioData.down = !audioData.down
            if (downBox != nil)
            {
                downBox.state = audioData.down ? .on: .off
            }
            spectrumView.needsDisplay = true

        case "f":
            audioData.filt = !audioData.filt
            if (filtBox != nil)
            {
                filtBox.state = audioData.filt ? .on: .off
            }
            scopeView.needsDisplay = true

        case "l":
            displayData.lock = !displayData.lock
            if (lockBox != nil)
            {
                lockBox.state = displayData.lock ? .on: .off
            }
            displayView.needsDisplay = true

        case "m":
            displayData.mult = !displayData.mult
            if (multBox != nil)
            {
                multBox.state = displayData.mult ? .on: .off
            }
            displayView.needsDisplay = true

        case "s":
            strobeData.enable = !strobeData.enable
            if (strbBox != nil)
            {
                strbBox.state = strobeData.enable ? .on: .off
            }
            strobeView.needsDisplay = true

        case "z":
            spectrumData.zoom = !spectrumData.zoom
            if (zoomBox != nil)
            {
                zoomBox.state = spectrumData.zoom ? .on: .off
            }
            spectrumView.needsDisplay = true

        default:
            // NSLog("Key %@", key)
            break
        }
    }
}
