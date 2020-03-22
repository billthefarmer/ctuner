//
//  Scope.swift
//  Tuner
//
//  Created by Bill Farmer on 08/10/2017.
//  Copyright Â© 2017 Bill Farmer. All rights reserved.
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
    var max: Double = 0

    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            audioData.filt = !audioData.filt
            if (filtBox != nil)
            {
                filtBox.state = audioData.filt ? .on: .off
            }
            needsDisplay = true;
        }
    }

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        NSBezierPath.fill(rect)

        // Dark green graticule
        let darkGreen = NSColor(red: 0, green: 0.25, blue: 0, alpha: 1.0)
        darkGreen.set()

        // Move the origin
        let context = NSGraphicsContext.current!
        context.cgContext.translateBy(x: 0, y: rect.midY)
        // let transform = AffineTransform(translationByX: 0, byY: rect.midY)
        // (transform as NSAffineTransform).concat()
        context.shouldAntialias = false;

        // Draw graticule
        for x in stride(from: rect.minX, to: rect.maxX, by: 6)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(x, rect.maxY / 2),
                                    to: NSMakePoint(x, -rect.maxY / 2))
        }

        for y in stride(from: 0, to: height / 2, by: 6)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(rect.minX, y),
                                    to: NSMakePoint(rect.maxX, y))
            NSBezierPath.strokeLine(from: NSMakePoint(rect.minX, -y),
                                    to: NSMakePoint(rect.maxX, -y))
        }

        if (scopeData.data == nil)
        {
            return
        }

        // Initialise sync
        var maxdx: Double = 0
        var dx: Double = 0
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

        let yscale = max / Double(height / 2)
        max = 0.0

        // Green trace
        NSColor.green.set()
        context.shouldAntialias = true;

        // Draw the trace
        let path = NSBezierPath()
        path.move(to: .zero)

        for i in 0 ..< Int(width)
        {
	    if (max < abs(scopeData.data[n + i]))
            {
	        max = abs(scopeData.data[n + i])
            }

	    let y = scopeData.data[n + i] / yscale
	    path.line(to: NSMakePoint(rect.minX + CGFloat(i), CGFloat(y)))
        }

        path.stroke()

	// Select font
        let font = NSFont.systemFont(ofSize: kTextSize)
        let attribs: [NSAttributedString.Key: Any] =
          [.foregroundColor: NSColor.yellow,
           .font: font]

        // Show F if filtered
        if (audioData.filt)
        {
            "F".draw(at: NSMakePoint(rect.minX + 2,
                                     rect.midY - kTextSize - 5),
                     withAttributes: attribs)
        }

        // Show FF if fundamental filter
        if (audioData.fund)
        {
            "FF".draw(at: NSMakePoint(rect.minX + 2, -rect.midY + 2),
                     withAttributes: attribs)
        }
    }
}
