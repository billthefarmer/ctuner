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

class SpectrumView: TunerView
{
    let kTextSize: CGFloat = 10
    var max: Double = 0

    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            spectrumData.zoom = !spectrumData.zoom
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

        for x in stride(from: NSMinX(rect), to: NSMaxX(rect), by: 6)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(x, NSMinY(rect)),
                                    to: NSMakePoint(x, NSMaxY(rect)))
        }

        for y in stride(from: NSMinY(rect), to: NSMaxY(rect), by: 6)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(NSMinX(rect), y),
                                    to: NSMakePoint(NSMaxX(rect), y))
        }

        if (spectrumData.data == nil)
        {
            return
        }

        if (spectrumData.expand == 0)
        {
            spectrumData.expand = 1
            spectrumData.zoom = true
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

        if (spectrumData.zoom)
        {
	    // Calculate scale
	    let xscale = (Double(width) / (spectrumData.r -
                                             spectrumData.l)) / 2.0

            // Draw vertical centre line
            NSBezierPath.strokeLine(from: NSMakePoint(NSMidX(rect),
                                                      NSMinY(rect)),
                                    to: NSMakePoint(NSMidX(rect),
                                                    NSMaxY(rect)))
            // Draw the spectrum
            let path = NSBezierPath()
            path.move(to: .zero)

            // Antialias
            context.shouldAntialias = true;

	    // Draw trace
	    for i in Int(floor(spectrumData.l)) ..< Int(ceil(spectrumData.h))
	    {
	        if (i > 0 && i < spectrumData.length)
	        {
		    let value = spectrumData.data[i]

		    if (max < value)
                    {
		        max = value
                    }

		    let y = NSMinY(rect) + CGFloat(value * yscale)
		    let x = NSMinX(rect) + CGFloat((Double(i) -
                                                      spectrumData.l) * xscale)

		    path.line(to: NSMakePoint(x, y))
	        }
	    }

	    path.stroke()

	    // Yellow pen for frequency trace
            NSColor.yellow.set()
            context.shouldAntialias = false;

	    // Draw line for nearest frequency
	    for i in 0 ..< Int(spectrumData.count)
	    {
	        // Draw line for values that are in range
	        if (spectrumData.values[i] > spectrumData.l &&
		      spectrumData.values[i] < spectrumData.h)
	        {
		    let x = NSMinX(rect) +
                      CGFloat((spectrumData.values[i] -
                                 spectrumData.l) * xscale)
		    NSBezierPath.strokeLine(from: NSMakePoint(x, NSMinY(rect)),
		                            to: NSMakePoint(x, NSMaxY(rect)))
	        }
	    }

	    // Select font
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            let attribs: [NSAttributedStringKey: Any] =
              [.foregroundColor: NSColor.yellow, .font: font]
            context.shouldAntialias = true;

	    for i in 0 ..< Int(spectrumData.count)
	    {
	        // Show value for values that are in range

	        if (spectrumData.values[i] > spectrumData.l &&
		      spectrumData.values[i] < spectrumData.h)
	        {
		    let f = displayData.maxima[i].f

		    // Reference freq
		    let fr = displayData.maxima[i].fr

		    let c = -12.0 * log2(fr / f)

		    // Ignore silly values
		    if (!c.isFinite)
                    {
		        continue
                    }

		    let x = NSMinX(rect) +
                      CGFloat((spectrumData.values[i] -
                                 spectrumData.l) * xscale)

		    let s = String(format: "%+0.0f", c * 100.0)
		    s.draw(at: NSMakePoint(x, NSMinY(rect) - 2),
                           withAttributes: attribs)
	        }
	    }
        }

        else
        {
	    let xscale = Double(spectrumData.length /
			          spectrumData.expand) / Double(width)

            // Draw the spectrum
            let path = NSBezierPath()
            path.move(to: .zero)

            // Antialias
            NSGraphicsContext.current!.shouldAntialias = true;

	    for x in 0 ..< Int(width)
	    {
	        var value: Double = 0

	        // Don't show DC component
	        if (x > 0)
	        {
		    for j in 0 ..< Int(xscale)
		    {
		        let n = Int(Double(x) * xscale) + j

		        if (value < spectrumData.data[n])
                        {
			    value = spectrumData.data[n]
                        }
		    }
	        }

	        if (max < value)
                {
		    max = value
                }

	        let y = value * yscale
                path.line(to: NSMakePoint(NSMinX(rect) + CGFloat(x),
                                          NSMinY(rect) + CGFloat(y)))
	    }

	    path.stroke()

	    // Yellow pen for frequency trace
	    NSColor.yellow.set()
            path.removeAllPoints()

	    for i in 0 ..< Int(spectrumData.count)
	    {
	        // Draw line for values

	        let x = spectrumData.values[i] / xscale
	        path.move(to: NSMakePoint(NSMinX(rect) + CGFloat(x),
                                          NSMinY(rect)))
	        path.line(to: NSMakePoint(NSMinX(rect) + CGFloat(x),
                                          NSMaxY(rect)))
	    }

	    path.stroke()

	    // Select font
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            let attribs: [NSAttributedStringKey: Any] =
              [.foregroundColor: NSColor.yellow,
               .font: font]
            context.shouldAntialias = true;

	    for i in 0 ..< Int(spectrumData.count)
	    {
	        // Show value for values

	        let f = displayData.maxima[i].f

	        // Reference freq

	        let fr = displayData.maxima[i].fr

	        let c = -12.0 * log2(fr / f)

	        // Ignore silly values

	        if (!c.isFinite)
                {
		    continue
                }

	        let x = spectrumData.values[i] / xscale
	        let s = String(format: "%+0.0f", c * 100.0)
	        s.draw(at: NSMakePoint(NSMinX(rect) + CGFloat(x),
                                       NSMinY(rect) - 2),
                       withAttributes: attribs)
	    }

	    if (spectrumData.expand > 1)
	    {
	        let s = String(format: "x%d", spectrumData.expand)
	        s.draw(at: NSMakePoint(NSMinX(rect), NSMinY(rect) - 2),
                       withAttributes: attribs)
	    }
        }

        let font = NSFont.systemFont(ofSize: kTextSize)
        let attribs: [NSAttributedStringKey: Any] =
          [.foregroundColor: NSColor.yellow, .font: font]
        context.shouldAntialias = true;

        if (audioData.down)
        {
	    "D".draw(at: NSMakePoint(NSMinX(rect),
                                     NSMaxY(rect) - kTextSize - 1),
                     withAttributes: attribs)
        }

        if (audioData.note)
        {
	    "NF".draw(at: NSMakePoint(NSMinX(rect), NSMinY(rect) - 2),
                     withAttributes: attribs)
        }
    }    
}
