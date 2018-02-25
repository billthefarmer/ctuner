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

class Spectrum: TunerView
{
    let kTextSize: CGFloat = 10
    var max: Float = 0

    override func resize(withOldSuperviewSize oldSize: NSSize)
    {
        super.resize(withOldSuperviewSize: oldSize)

        spectrumData.expand = 1
        spectrumData.zoom = false;
    }

    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            spectrumData.zoom = !spectrumData.zoom
        }
    }

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        __NSRectFill(rect)

        let darkGreen = NSColor(red: 0, green: 0.5, blue: 0, alpha: 1.0)
        darkGreen.set()

        let path = NSBezierPath()

        for x in stride(from: NSMinX(rect), to: NSMaxX(rect), by: 6)
        {
            path.move(to: NSPoint(x: x, y: NSMinY(rect)))
            path.line(to: NSPoint(x: x, y: NSMaxY(rect)))
        }

        for y in stride(from: NSMinY(rect), to: NSMaxY(rect), by: 6)
        {
            path.move(to: NSPoint(x: NSMinX(rect), y: y))
            path.line(to: NSPoint(x: NSMaxX(rect), y: y))
        }

        path.stroke()

        if (spectrumData.data == nil)
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
        let yscale = height / max
        max = 0.0

        // Green trace
        NSColor.green.set()

        // Draw the spectrum
        path.removeAllPoints()
        path.move(to: NSZeroPoint)

        if (spectrumData.zoom)
        {
	    // Calculate scale
	    let xscale = (width / (spectrumData.r - spectrumData.l)) / 2.0

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

		    let y = value * yscale
		    let x = (Float(i) - spectrumData.l) * xscale 

		    path.line(to: NSPoint(x: Int(x), y: Int(y)))
	        }
	    }

            // Draw vertical centre line
	    path.move(to: NSPoint(x: Int(width / 2), y: 0))
	    path.line(to: NSPoint(x: Int(width / 2), y: Int(-height)))
	    path.stroke()

	    // Yellow pen for frequency trace
            NSColor.yellow.set()
            path.removeAllPoints()

	    // Draw line for nearest frequency
	    for i in 0 ..< spectrumData.count
	    {
	        // Draw line for values that are in range
	        if (spectrumData.values[Int(i)] > spectrumData.l &&
		      spectrumData.values[Int(i)] < spectrumData.h)
	        {
		    let x = (spectrumData.values[Int(i)] -
                               spectrumData.l) * xscale
		    path.move(to: NSPoint(x: Int(x), y: 0))
		    path.line(to: NSPoint(x: Int(x), y: Int(height)))
	        }
	    }

	    path.stroke()

	    // Select font
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            font.set()

	    for i in 0 ..< spectrumData.count
	    {
	        // Show value for values that are in range

	        if (spectrumData.values[Int(i)] > spectrumData.l &&
		      spectrumData.values[Int(i)] < spectrumData.h)
	        {
		    let f = displayData.maxima[Int(i)].f

		    // Reference freq
		    let fr = displayData.maxima[Int(i)].fr

		    let c = -12.0 * log2f(fr / f)

		    // Ignore silly values
		    if (!c.isFinite)
                    {
		        continue
                    }

		    let x = (spectrumData.values[Int(i)] -
                               spectrumData.l) * xscale

		    let s = String(format: "%+0.0f", c * 100.0)
		    s.draw(at: NSPoint(x: Int(x), y: 1))
	        }
	    }
        }

        else
        {
	    let xscale = Float(spectrumData.length /
			    spectrumData.expand) / width

	    for x in 0 ..< Int(width)
	    {
	        var value: Float = 0

	        // Don't show DC component
	        if (x > 0)
	        {
		    for j in 0 ..< Int(xscale)
		    {
		        let n = x * Int(xscale) + j

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
                path.line(to: NSPoint(x: NSMinX(rect) + CGFloat(x),
                                      y: NSMinY(rect) + CGFloat(y)))
	    }

	    path.stroke()

	    // Yellow pen for frequency trace
	    NSColor.yellow.set()
            path.removeAllPoints()

	    for i in 0 ..< spectrumData.count
	    {
	        // Draw line for values

	        let x = spectrumData.values[Int(i)] / xscale
	        path.move(to: NSPoint(x: NSMinX(rect) + CGFloat(x),
                                      y: NSMinY(rect)))
	        path.line(to: NSPoint(x: NSMinX(rect) + CGFloat(x),
                                      y: NSMaxY(rect)))
	    }

	    path.stroke()

	    // Select font
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            font.set()

	    for i in 0 ..< spectrumData.count
	    {
	        // Show value for values

	        let f = displayData.maxima[Int(i)].f

	        // Reference freq

	        let fr = displayData.maxima[Int(i)].fr

	        let c = -12.0 * log2f(fr / f)

	        // Ignore silly values

	        if (!c.isFinite)
                {
		    continue
                }

	        let x = spectrumData.values[Int(i)] / xscale
	        let s = String(format: "%+0.0f", c * 100.0)
	        s.draw(at: NSPoint(x: Int(x), y: 1))
	    }

	    if (spectrumData.expand > 1)
	    {
	        let s = String(format: "x%d", spectrumData.expand)
	        s.draw(at: NSPoint(x: 0, y: 1))
	    }
        }

        if (audio.downsample == false)
        {
	    NSColor.yellow.set()
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            font.set()

	    "D".draw(at: NSPoint(x: 2, y: 2))
        }

        // NSLog("Spectrum displayed")
    }    
}
