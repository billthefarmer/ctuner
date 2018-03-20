//
//  Meter.swift
//  Tuner
//
//  Created by Bill Farmer on 08/10/2017.
//  Copyright © 2017 Bill Farmer. All rights reserved.
//
//  Created by Bill Farmer on 18/02/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
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

// TunerView
class MeterView: TunerView
{
    // draw
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Text and tick sizes
        let textSize: CGFloat = CGFloat(height / 3)
        let tickSize: CGFloat = CGFloat(height / 6)
        let tickSize2: CGFloat = tickSize / 2

        // Font
        let font = NSFont.systemFont(ofSize: textSize)
        var attribs: [NSAttributedStringKey: Any] = [.font: font]

        // Scale text if necessary
        let dx = "50".size(withAttributes: attribs).width
        if (dx >= width / 11)
        {
            expansion = log((width / 12) / dx)
            attribs = [.font: font, .expansion: expansion]
        }

        // Gradient
        let gradient = NSGradient(colors: [NSColor.textColor,
                                           NSColor.windowBackgroundColor,
                                           NSColor.textColor])
        // Drawing code here.
        NSEraseRect(rect)

        // Move the origin
        var transform = AffineTransform(translationByX: NSMidX(rect),
                                        byY: NSMaxY(rect) - textSize - 4)
        (transform as NSAffineTransform).concat()

        // Draw the meter scale
        for i in 0 ..< 6
        {
	    if (i == 0)
            {
                let offset = "0".size(withAttributes: attribs).width / 2
	        "0".draw(at: NSPoint(x: -offset, y: 0),
                       withAttributes: attribs)
            }

	    else
	    {
	        let x = width / 11 * Float(i)
	        let s = String(format:"%d", i * 10)
                let offset = s.size(withAttributes: attribs).width / 2

	        s.draw(at: NSPoint(x: CGFloat(x) - offset, y: 0),
                       withAttributes: attribs)
	        s.draw(at: NSPoint(x: -CGFloat(x) - offset, y: 0),
                       withAttributes: attribs)
	    }
        }

        // Move the origin
        transform = AffineTransform(translationByX: 0, byY: -tickSize)
        (transform as NSAffineTransform).concat()

        NSGraphicsContext.current!.shouldAntialias = false;
        let path = NSBezierPath()
        path.lineWidth = 2

        for i in 0 ..< 6
        {
	    let x = width / 11 * Float(i)

	    path.move(to: NSPoint(x: CGFloat(x), y: 0))
	    path.line(to: NSPoint(x: CGFloat(x), y: tickSize))

	    path.move(to: NSPoint(x: -CGFloat(x), y: 0))
	    path.line(to: NSPoint(x: -CGFloat(x), y: tickSize))

	    for j in 1 ..< 5
	    {
                let jx = Float(j) * width / 55

	        if (i < 5)
	        {
		    path.move(to: NSPoint(x: CGFloat(x + jx), y: 0))
		    path.line(to: NSPoint(x: CGFloat(x + jx), y: tickSize2))
	        }

	        path.move(to: NSPoint(x: CGFloat(-x + jx), y: 0))
	        path.line(to: NSPoint(x: CGFloat(-x + jx), y: tickSize2))
	    }
        }

        path.stroke()

        // Move the origin
        transform = AffineTransform(translationByX: 0, byY: -tickSize)
        (transform as NSAffineTransform).concat()

        NScolor.gray.set()

        // Draw bar
        NSFrameRect(NSMakeRect(width / 36 - width / 2, height / 128,
                               width - width / 18, -height / 128))
    }
}
