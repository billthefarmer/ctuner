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
    @objc var lock = false
    @objc var mult = false
    @objc var tran = 0
    @objc var n = 0
    @objc var c = 0.0
    @objc var f = 0.0
    @objc var fr = 0.0

    let kTextSize: CGFloat = 12

    // mouseDown
    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            lock = !lock
            if (lockBox != nil)
            {
                lockBox.state = lock ? .on: .off
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

        if (mult)
        {
	    // Select font
            let font = NSFont.systemFont(ofSize: textSizeSmall)
            let attribs: [NSAttributedString.Key: Any] = [.font: font]

            var s = ""
            var x = rect.minX + 2
            var y = rect.maxY - textSizeSmall - 2

            // No data, probably
	    if (disp.count == 0)
	    {
	        // Display note
                s = String(format: "%@%@%d",
                           notes[Int(disp.n -
                                       trans[Int(disp.trans)] +
                                       kOctave) % notes.endIndex],
		           sharps[Int(disp.n -
                                        trans[Int(disp.trans)] +
                                       kOctave) % sharps.endIndex],
                                  disp.n / kOctave)
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                x += s.size(withAttributes: attribs).width +
                  "  ".size(withAttributes: attribs).width

	        // Display cents
                s = String(format: "%+4.2lf\u{00A2}", disp.c * 100.0)
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                x += s.size(withAttributes: attribs).width +
                  "  ".size(withAttributes: attribs).width

	        // Display reference
	        s = String(format: "%4.2lfHz", disp.fr);
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                x += s.size(withAttributes: attribs).width +
                  "  ".size(withAttributes: attribs).width

	        // Display frequency
	        s = String(format: "%4.2lfHz", disp.f);
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
                x += s.size(withAttributes: attribs).width +
                  "  ".size(withAttributes: attribs).width

	        // Display difference
	        s = String(format: "%+4.2lfHz", disp.f - disp.fr);
                s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
            }

            for i in 0 ..< Int(disp.count)
            {
                // Actual freq
	        let f = disp.maxima[i].f

	        // Reference freq
	        let fr = disp.maxima[i].fr

                // Note number
	        let n = disp.maxima[i].n

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
                           notes[Int(n - trans[Int(disp.trans)] +
                                       kOctave) % notes.endIndex],
		           sharps[Int(n - trans[Int(disp.trans)] +
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
                       notes[Int(disp.n -
                                   trans[Int(disp.trans)] +
                                   kOctave) % notes.endIndex])
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
            x += s.size(withAttributes: attribs).width

	    // Select font
            font = NSFont.boldSystemFont(ofSize: textSizeLarger / 2)
            attribs = [.font: font]

	    s = String(format: "%d", disp.n / kOctave) 
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

	    // Select font
            font = NSFont.boldSystemFont(ofSize: textSizeLarger / 2)
            attribs = [.font: font]

	    s = String(format: "%@",
                       sharps[Int(disp.n -
                                    trans[Int(disp.trans)] +
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
	    s = String(format: "%+4.2f\u{00A2}", disp.c * 100.0)
            x = rect.maxX - s.size(withAttributes: attribs).width - 2
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

	    // Select font
            font = NSFont.boldSystemFont(ofSize: textSizeMedium)
            attribs = [.font: font]

	    y -= textSizeMedium + 4
            x = rect.minX + 2

	    s = String(format: "%4.2lfHz", disp.fr)
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

            // Right justify
	    s = String(format: "%4.2lfHz", disp.f)
            x = rect.maxX - s.size(withAttributes: attribs).width - 2
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

	    y -= textSizeMedium
            x = rect.minX + 2

	    s = String(format: "%4.2lfHz", (audio.reference == 0) ?
		    kA5Reference : audio.reference)
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

            // Right justify
	    s = String(format: "%+4.2lfHz", disp.f - disp.fr)
            x = rect.maxX - s.size(withAttributes: attribs).width - 2
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
        }

        // Display L if locked
        if (lock == true)
        {
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            let attribs: [NSAttributedString.Key: Any] = [.font: font]

            "L".draw(at: NSMakePoint(rect.minX + 2, rect.minY),
                     withAttributes: attribs)
        }
    }
}
