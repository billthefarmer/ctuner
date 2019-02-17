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

let kMaxExpand: Int32 = 16
let kMinExpand: Int32 = 1

class SpectrumView: TunerView
{
    let kTextSize: CGFloat = 10
    var max: Double = 0

    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            spectrumData.zoom = !spectrumData.zoom
            if (zoomBox != nil)
            {
                zoomBox.state = spectrumData.zoom ? .on: .off
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
	    for i in Int(floor(spectrumData.l)) ... Int(ceil(spectrumData.h))
	    {
	        if (i > 0 && i < spectrumData.length)
	        {
		    let value = spectrumData.data[i]

		    if (max < value)
                    {
		        max = value
                    }

		    let y = rect.minY + CGFloat(value * yscale)
		    let x = rect.minX + CGFloat((Double(i) -
                                                      spectrumData.l) * xscale)

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
	    for i in 0 ..< Int(spectrumData.count)
	    {
	        // Draw line for values that are in range
	        if (spectrumData.values[i] > spectrumData.l &&
		      spectrumData.values[i] < spectrumData.h)
	        {
		    let x = rect.minX +
                      CGFloat((spectrumData.values[i] -
                                 spectrumData.l) * xscale)
		    NSBezierPath.strokeLine(from: NSMakePoint(x, rect.minY),
		                            to: NSMakePoint(x, rect.maxY))
	        }
	    }

	    // Select font
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            let attribs: [NSAttributedString.Key: Any] =
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

		    let x = rect.minX +
                      CGFloat((spectrumData.values[i] -
                                 spectrumData.l) * xscale)

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
	    let xscale = log(Double(spectrumData.length) /
			       Double(spectrumData.expand)) / Double(width)

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
	                if (i > 0 && i < spectrumData.length)
	                {
		            if (value < spectrumData.data[i])
                            {
			        value = spectrumData.data[i]
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

	    for i in 0 ..< Int(spectrumData.count)
	    {
	        // Draw line for values

	        let x = spectrumData.values[i] / xscale
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
	        s.draw(at: NSMakePoint(rect.minX + CGFloat(x),
                                       rect.minY - 2),
                       withAttributes: attribs)
	    }

	    if (spectrumData.expand > 1)
	    {
	        let s = String(format: "x %d", spectrumData.expand)
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

        if (audioData.down)
        {
	    "D".draw(at: NSMakePoint(rect.minX + 2,
                                     rect.maxY - kTextSize - 3),
                     withAttributes: attribs)
        }

        if (audioData.note)
        {
	    "NF".draw(at: NSMakePoint(rect.minX + 2, rect.minY),
                     withAttributes: attribs)
        }
    }    
}
