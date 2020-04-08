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
    @objc var cents = Double(0)

    var mc = Double(0)

    // mouseDown
    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            displayView.lock = !displayView.lock
            if (lockBox != nil)
            {
                lockBox.state = displayView.lock ? .on: .off
            }
            displayView.needsDisplay = true
        }
    }

    // draw
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Do the inertia calculation
        mc = ((mc * 19) + meter.c) / 20

        // Context
        let context = NSGraphicsContext.current!

        // Text and tick sizes
        let textSize = CGFloat(height / 3)
        let tickSize = CGFloat(height / 6)
        let tickSize2 = tickSize / 2

        // Font
        let font = NSFont.systemFont(ofSize: textSize)
        var attribs: [NSAttributedString.Key: Any] = [.font: font]

        // Scale text if necessary
        let dx = "50".size(withAttributes: attribs).width
        if (dx >= width / 11)
        {
            let expansion = log((width / 12) / dx)
            attribs = [.font: font, .expansion: expansion]
        }
        // Drawing code here.
        NSEraseRect(rect)

        // Move the origin
        context.cgContext.translateBy(x: rect.midX, y: rect.maxY - textSize -
                                          tickSize2)
        // Draw the meter scale
        for i in 0 ..< 6
        {
	    if (i == 0)
            {
                let offset = "0".size(withAttributes: attribs).width / 2
	        "0".draw(at: NSMakePoint(-offset, 0), withAttributes: attribs)
            }

	    else
	    {
	        let x = width / 11 * CGFloat(i)
	        let s = String(format:"%d", i * 10)
                let offset = s.size(withAttributes: attribs).width / 2

	        s.draw(at: NSMakePoint(CGFloat(x) - offset, 0),
                       withAttributes: attribs)
	        s.draw(at: NSMakePoint(-CGFloat(x) - offset, 0),
                       withAttributes: attribs)
	    }
        }

        // Move the origin
        context.cgContext.translateBy(x: 0, y: -tickSize)
        context.shouldAntialias = false;

        for i in 0 ..< 6
        {
	    let x = width / 11 * CGFloat(i)

	    NSBezierPath.strokeLine(from: NSMakePoint(CGFloat(x), 0),
	                            to: NSMakePoint(CGFloat(x), tickSize))

	    NSBezierPath.strokeLine(from: NSMakePoint(-CGFloat(x), 0),
	                            to: NSMakePoint(-CGFloat(x), tickSize))

	    for j in 1 ..< 5
	    {
                let jx = CGFloat(j) * width / 55

	        if (i < 5)
	        {
		    NSBezierPath
                      .strokeLine(from: NSMakePoint(CGFloat(x + jx), 0),
		                  to: NSMakePoint(CGFloat(x + jx), tickSize2))
	        }

	        NSBezierPath
                  .strokeLine(from: NSMakePoint(CGFloat(-x + jx), 0),
	                      to: NSMakePoint(CGFloat(-x + jx), tickSize2))
	    }
        }

        // Move the origin
        context.cgContext.translateBy(x: 0, y: -tickSize)
        NSColor.gray.set()

        // Draw bar
        NSBezierPath.fill(NSMakeRect(width / 36 - width / 2, -height / 32,
                                       width - width / 18, height / 32))

        // Gradient
        let gradient = NSGradient(colors: [NSColor.gray,
                                           NSColor.white,
                                           NSColor.gray])!
        // Thumb
        let thumb = NSBezierPath()
        thumb.move(to: NSMakePoint(0, 2))
        thumb.line(to: NSMakePoint(1, 1))
        thumb.line(to: NSMakePoint(1, -2))
        thumb.line(to: NSMakePoint(-1, -2))
        thumb.line(to: NSMakePoint(-1, 1))
        thumb.close()

        // Transform
        let scale = AffineTransform(scale: height / 16)
        let translate =
          AffineTransform(translationByX: CGFloat(mc) * (width * 10 / 11),
                          byY: 0)
        thumb.transform(using: scale)
        thumb.transform(using: translate)
        context.shouldAntialias = true;
        gradient.draw(in: thumb, angle: 90)
        thumb.stroke()
    }
}
