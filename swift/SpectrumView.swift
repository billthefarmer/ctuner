//
//  Spectrum.swift
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

let kMaxExpand = 16
let kMinExpand = 1

class SpectrumView: TunerView
{
    @objc var zoom = true
    @objc var expand = 1
    @objc var f = 0.0
    @objc var l = 0.0
    @objc var h = 0.0
    @objc var r = 0.0

    let kTextSize = CGFloat(10)
    var max = 0.0

    // mouseDown
    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            zoom = !zoom
            if (zoomBox != nil)
            {
                zoomBox.state = zoom ? .on: .off
            }
            needsDisplay = true
        }
    }

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        let context = NSGraphicsContext.current!

        // Drawing code here.
        NSBezierPath.fill(rect)

        let darkGreen = NSColor(red: 0, green: 0.25, blue: 0, alpha: 1.0)
        darkGreen.set()
        context.shouldAntialias = false;

        for x in stride(from: rect.minX, to: rect.maxX, by: 6)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(x, rect.minY),
                                    to: NSMakePoint(x, rect.maxY))
        }

        for y in stride(from: rect.minY, to: rect.maxY, by: 6)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(rect.minX, y),
                                    to: NSMakePoint(rect.maxX, y))
        }

        if (spectrum.data == nil)
        {
            return
        }

        if (width < 1)
        {
            return
        }

        if (max < 1.0)
        {
	    max = 1.0
        }

        // Calculate the scaling
        let yscale = Double(height) / max
        max = 0.0

        // Green trace
        NSColor.green.set()

        if (zoom)
        {
	    // Calculate scale
	    let xscale = (Double(width) / (r - l)) / 2.0

            // Draw vertical centre line
            NSBezierPath.strokeLine(from: NSMakePoint(rect.midX,
                                                      rect.minY),
                                    to: NSMakePoint(rect.midX,
                                                    rect.maxY))
            // Draw the spectrum
            let path = NSBezierPath()
            path.move(to: .zero)

            // Antialias
            context.shouldAntialias = true;

	    // Draw trace
	    for i in Int(floor(l)) ... Int(ceil(h))
	    {
	        if (i > 0 && i < spectrum.length)
	        {
		    let value = spectrum.data[i]

		    if (max < value)
                    {
		        max = value
                    }

		    let y = rect.minY + CGFloat(value * yscale)
		    let x = rect.minX + CGFloat((Double(i) - l) * xscale)

		    path.line(to: NSMakePoint(x, y))
	        }
	    }

            // Complete path
            path.line(to: NSMakePoint(rect.maxX, 0))
            path.close()

            // Fill colour
            let lightGreen = NSColor(red: 0, green: 1, blue: 0, alpha: 0.25)
            lightGreen.set()
            path.fill()

            // Stroke colour
            NSColor.green.set()
	    path.stroke()

	    // Yellow pen for frequency trace
            NSColor.yellow.set()
            context.shouldAntialias = false;

	    // Draw line for nearest frequency
	    for i in 0 ..< Int(spectrum.count)
	    {
	        // Draw line for values that are in range
	        if (spectrum.values[i] > l && spectrum.values[i] < h)
	        {
		    let x = rect.minX +
                      CGFloat((spectrum.values[i] - l) * xscale)
		    NSBezierPath.strokeLine(from: NSMakePoint(x, rect.minY),
		                            to: NSMakePoint(x, rect.maxY))
	        }
	    }

	    // Select font
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            let attribs: [NSAttributedString.Key: Any] =
              [.foregroundColor: NSColor.yellow, .font: font]
            context.shouldAntialias = true;

	    for i in 0 ..< Int(spectrum.count)
	    {
	        // Show value for values that are in range

	        if (spectrum.values[i] > l && spectrum.values[i] < h)
	        {
		    let f = disp.maxima[i].f

		    // Reference freq
		    let fr = disp.maxima[i].fr

		    let c = -12.0 * log2(fr / f)

		    // Ignore silly values
		    if (!c.isFinite)
                    {
		        continue
                    }

		    let x = rect.minX +
                      CGFloat((spectrum.values[i] - l) * xscale)

		    let s = String(format: "%+0.0f", c * 100.0)
		    s.draw(at: NSMakePoint(x, rect.minY - 2),
                           withAttributes: attribs)
	        }
	    }
        }

        else
        {
            // Antialias
            context.shouldAntialias = true;

            // Scale
	    let xscale = log(Double(spectrum.length) /
			       Double(expand)) / Double(width)

            // Draw the spectrum
            let path = NSBezierPath()
            path.move(to: .zero)

            var last = 1
	    for x in 0 ... Int(width)
	    {
	        var value = 0.0

                let index = Int(round(pow(M_E, Double(x) * xscale)))
                if (index > last)
                {
                    for i in last ..< index
                    {
	                // Don't show DC component
	                if (i > 0 && i < spectrum.length)
	                {
		            if (value < spectrum.data[i])
                            {
			        value = spectrum.data[i]
                            }
		        }
	            }
                }

	        // Update last index
	        last = index;

	        if (max < value)
                {
		    max = value
                }

	        let y = value * yscale
                path.line(to: NSMakePoint(rect.minX + CGFloat(x),
                                          rect.minY + CGFloat(y)))
	    }

            // Complete path
            path.line(to: NSMakePoint(rect.maxX, 0))
            path.close()

            // Fill colour
            let lightGreen = NSColor(red: 0, green: 1, blue: 0, alpha: 0.25)
            lightGreen.set()
            path.fill()

            // Stroke colour
            NSColor.green.set()
	    path.stroke()

	    // Yellow pen for frequency trace
	    NSColor.yellow.set()
            path.removeAllPoints()

	    for i in 0 ..< Int(spectrum.count)
	    {
	        // Draw line for values

	        let x = spectrum.values[i] / xscale
	        path.move(to: NSMakePoint(rect.minX + CGFloat(x),
                                          rect.minY))
	        path.line(to: NSMakePoint(rect.minX + CGFloat(x),
                                          rect.maxY))
	    }

	    path.stroke()

	    // Select font
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            let attribs: [NSAttributedString.Key: Any] =
              [.foregroundColor: NSColor.yellow,
               .font: font]
            context.shouldAntialias = true;

	    for i in 0 ..< Int(spectrum.count)
	    {
	        // Show value for values

	        let f = disp.maxima[i].f

	        // Reference freq

	        let fr = disp.maxima[i].fr

	        let c = -12.0 * log2(fr / f)

	        // Ignore silly values

	        if (!c.isFinite)
                {
		    continue
                }

	        let x = spectrum.values[i] / xscale
	        let s = String(format: "%+0.0f", c * 100.0)
	        s.draw(at: NSMakePoint(rect.minX + CGFloat(x),
                                       rect.minY - 2),
                       withAttributes: attribs)
	    }

	    if (expand > 1)
	    {
	        let s = String(format: "x %d", expand)
                let dx = s.size(withAttributes: attribs).width
	        s.draw(at: NSMakePoint(rect.maxX - dx - 2,
                                       rect.minY),
                       withAttributes: attribs)
	    }
        }

        let font = NSFont.systemFont(ofSize: kTextSize)
        let attribs: [NSAttributedString.Key: Any] =
          [.foregroundColor: NSColor.yellow, .font: font]
        context.shouldAntialias = true;

        if (audio.down)
        {
	    "D".draw(at: NSMakePoint(rect.minX + 2,
                                     rect.maxY - kTextSize - 3),
                     withAttributes: attribs)
        }

        if (audio.note)
        {
	    "NF".draw(at: NSMakePoint(rect.minX + 2, rect.minY),
                     withAttributes: attribs)
        }
    }
}
