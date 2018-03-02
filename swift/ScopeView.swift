//
//  Scope.swift
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

class ScopeView: TunerView
{
    let kTextSize: CGFloat = 10
    var max: Float = 0

    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            audioData.filter = !audioData.filter
        }
    }

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        __NSRectFill(rect)

        // Dark green graticule
        let darkGreen = NSColor(red: 0, green: 0.5, blue: 0, alpha: 1.0)
        darkGreen.set()

        // Move the origin
        let transform = AffineTransform(translationByX: 0, byY: NSMidY(rect))
        (transform as NSAffineTransform).concat()

        // Draw graticule
        let path = NSBezierPath()
        for x in stride(from: NSMinX(rect), to: NSMaxX(rect), by: 6)
        {
            path.move(to: NSPoint(x: x, y: NSMaxY(rect) / 2))
            path.line(to: NSPoint(x: x, y: -NSMaxY(rect) / 2))
        }

        for y in stride(from: 0, to: NSHeight(rect) / 2, by: 6)
        {
            path.move(to: NSPoint(x: NSMinX(rect), y: y))
            path.line(to: NSPoint(x: NSMaxX(rect), y: y))
            path.move(to: NSPoint(x: NSMinX(rect), y: -y))
            path.line(to: NSPoint(x: NSMaxX(rect), y: -y))
        }

        path.stroke()

        if (scopeData.data == nil)
        {
            return
        }

        // Initialise sync
        var maxdx: Float = 0
        var dx: Float = 0
        var n = 0

        if (width < 1)
        {
            return
        }

        for i in 1 ..< Int(width)
        {
	    dx = scopeData.data[i] - scopeData.data[i - 1]
	    if (maxdx < dx)
	    {
	        maxdx = dx
	        n = i
	    }

	    if (maxdx < 0.0 && dx > 0.0)
            {
	        break
            }
        }

        // Calculate scale
        if (max < 0.125)
        {
	    max = 0.125
        }

        let yscale = max / (height / 2)
        max = 0.0

        // Green trace
        NSColor.green.set()

        // Draw the trace
        path.removeAllPoints()
        path.move(to: NSZeroPoint)

        for i in 0 ..< Int(width)
        {
	    if (max < abs(scopeData.data[n + i]))
            {
	        max = abs(scopeData.data[n + i])
            }

	    let y = scopeData.data[n + i] / yscale
	    path.line(to: NSPoint(x: NSMinX(rect) + CGFloat(i), y: CGFloat(y)))
        }

            path.stroke()

        // Show F if filtered
        if (audioData.filter == true)
        {
	    // Yellow text
            NSColor.yellow.set()

	    // Select font
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            font.set()
            "F".draw(at: NSPoint(x: 0, y: Int(height) / 2 - 2))
        }

        // NSLog("Scope displayed");
    }
}
