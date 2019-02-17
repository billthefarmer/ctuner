//
//  Display.swift
//  Tuner
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

class DisplayView: TunerView
{
    let kTextSize: CGFloat = 12

    // mouseDown
    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            displayData.lock = !displayData.lock
            if (lockBox != nil)
            {
                lockBox.state = displayData.lock ? .on: .off
            }
            needsDisplay = true
        }
    }

    // draw
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Text sizes
        let textSizeLarger = height / 2
        let textSizeLarge = height / 3
        let textSizeMedium = height / 5
        let textSizeSmall = height / 9

        // Drawing code here.
        NSEraseRect(rect)

        if (displayData.mult)
        {
	    // Select font
            let font = NSFont.systemFont(ofSize: textSizeSmall)
            let attribs: [NSAttributedString.Key: Any] = [.font: font]

            var s = ""
            var x = rect.minX + 2
            var y = rect.maxY - textSizeSmall - 2

            // No data, probably
	    if (displayData.count == 0)
	    {
	        // Display note
                s = String(format: "%@%@%d",
                           notes[Int(displayData.n -
                                       trans[Int(displayData.trans)] +
                                       kOctave) % notes.endIndex],
		           sharps[Int(displayData.n -
                                        trans[Int(displayData.trans)] +
                                       kOctave) % sharps.endIndex],
                                  displayData.n / kOctave)
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                x += s.size(withAttributes: attribs).width +
                  "  ".size(withAttributes: attribs).width

	        // Display cents
                s = String(format: "%+4.2lf\u{00A2}", displayData.c * 100.0)
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                x += s.size(withAttributes: attribs).width +
                  "  ".size(withAttributes: attribs).width

	        // Display reference
	        s = String(format: "%4.2lfHz", displayData.fr);
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                x += s.size(withAttributes: attribs).width +
                  "  ".size(withAttributes: attribs).width

	        // Display frequency
	        s = String(format: "%4.2lfHz", displayData.f);
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                x += s.size(withAttributes: attribs).width +
                  "  ".size(withAttributes: attribs).width

	        // Display difference
	        s = String(format: "%+4.2lfHz", displayData.f - displayData.fr);
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
            }

            for i in 0 ..< Int(displayData.count)
            {
                // Actual freq
	        let f = displayData.maxima[i].f

	        // Reference freq
	        let fr = displayData.maxima[i].fr

                // Note number
	        let n = displayData.maxima[i].n

                // Ignore negative
	        if (n < 0)
                {
		    continue
                }

                // Cents
	        let c = -12.0 * log2(fr / f)

	        // Ignore silly values
	        if (!c.isFinite)
                {
		    continue
                }

	        // Display note
	        s = String(format: "%@%@%d",
                           notes[Int(n - trans[Int(displayData.trans)] +
                                       kOctave) % notes.endIndex],
		           sharps[Int(n - trans[Int(displayData.trans)] +
                                        kOctave) % sharps.endIndex],
                           n / kOctave)
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                x += s.size(withAttributes: attribs).width +
                  "  ".size(withAttributes: attribs).width
	        // Display cents

	        s = String(format: "%+4.2lf\u{00A2}", c * 100.0)
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                x += s.size(withAttributes: attribs).width +
                  "  ".size(withAttributes: attribs).width
	        // Display reference

	        s = String(format: "%4.2lfHz", fr)
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                x += s.size(withAttributes: attribs).width +
                  "  ".size(withAttributes: attribs).width
	        // Display frequency

	        s = String(format: "%4.2lfHz", f)
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                x += s.size(withAttributes: attribs).width +
                  "  ".size(withAttributes: attribs).width
	        // Display difference

	        s = String(format: "%+4.2lfHz", f - fr)
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                y += textSizeSmall
            }
        }

        else
        {
            var s = ""
            var x = rect.minX + 2
            var y = rect.maxY - textSizeLarger - 2

	    // Select font
            var font = NSFont.boldSystemFont(ofSize: textSizeLarger)
            var attribs: [NSAttributedString.Key: Any] = [.font: font]

	    s = String(format: "%@",
                       notes[Int(displayData.n -
                                   trans[Int(displayData.trans)] +
                                   kOctave) % notes.endIndex])
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
            x += s.size(withAttributes: attribs).width

	    // Select font
            font = NSFont.boldSystemFont(ofSize: textSizeLarger / 2)
            attribs = [.font: font]

	    s = String(format: "%d", displayData.n / kOctave) 
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

	    // Select font
            font = NSFont.boldSystemFont(ofSize: textSizeLarger / 2)
            attribs = [.font: font]

	    s = String(format: "%@",
                       sharps[Int(displayData.n -
                                    trans[Int(displayData.trans)] +
                                    kOctave) % sharps.endIndex])
            s.draw(at: NSMakePoint(x, y + textSizeLarger / 2),
                   withAttributes: attribs)

	    // Select font
            font = NSFont.boldSystemFont(ofSize: textSizeLarge)
            attribs = [.font: font]
            let dx = "-50.00\u{00A2}".size(withAttributes: attribs).width
            if (dx >= width / 2)
            {
                let expansion = log((width / 2) / dx)
                attribs = [.font: font, .expansion: expansion]
            }

            // Right justify
	    s = String(format: "%+4.2f\u{00A2}", displayData.c * 100.0)
            x = rect.maxX - s.size(withAttributes: attribs).width - 2
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

	    // Select font
            font = NSFont.boldSystemFont(ofSize: textSizeMedium)
            attribs = [.font: font]

	    y -= textSizeMedium + 4
            x = rect.minX + 2

	    s = String(format: "%4.2lfHz", displayData.fr)
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

            // Right justify
	    s = String(format: "%4.2lfHz", displayData.f)
            x = rect.maxX - s.size(withAttributes: attribs).width - 2
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

	    y -= textSizeMedium
            x = rect.minX + 2

	    s = String(format: "%4.2lfHz", (audioData.reference == 0) ?
		    kA5Reference : audioData.reference)
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

            // Right justify
	    s = String(format: "%+4.2lfHz", displayData.f - displayData.fr)
            x = rect.maxX - s.size(withAttributes: attribs).width - 2
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
        }

        // Display L if locked
        if (displayData.lock == true)
        {
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            let attribs: [NSAttributedString.Key: Any] = [.font: font]

            "L".draw(at: NSMakePoint(rect.minX + 2, rect.minY),
                     withAttributes: attribs)
        }
    }
}
