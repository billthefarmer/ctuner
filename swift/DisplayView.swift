//
//  Display.swift
//  Tuner
//
//  Created by Bill Farmer on 08/10/2017.
//  Copyright © 2017 Bill Farmer. All rights reserved.
//

import Cocoa

class DisplayView: TunerView
{
    let kOctave: Int32 = 12
    let kTextSize: CGFloat = 12

    let notes =
      ["C", "C", "D", "E", "E", "F",
       "F", "G", "A", "A", "B", "B"]

    let sharps =
      ["", "\u{0266F}", "", "\u{0266D}", "", "",
       "\u{0266F}", "", "\u{0266D}", "", "\u{0266D}", ""]

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
            let attribs: [NSAttributedStringKey: Any] = [.font: font]

            var s = ""
            var x = NSMinX(rect) + 2
            var y = NSMaxY(rect) - textSizeSmall - 2

            // No data, probably
	    if (displayData.count == 0)
	    {
	        // Display note
                s = String(format: "%@%@%d",
                           notes[Int(displayData.n) % notes.endIndex],
		           sharps[Int(displayData.n) % sharps.endIndex],
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
	        s = String(format: "%@%@%d", notes[Int(n) % notes.endIndex],
		       sharps[Int(n) % sharps.endIndex], n / kOctave)
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
            var x = NSMinX(rect) + 2
            var y = NSMaxY(rect) - textSizeLarger - 2

	    // Select font
            var font = NSFont.boldSystemFont(ofSize: textSizeLarger)
            var attribs: [NSAttributedStringKey: Any] = [.font: font]

	    s = String(format: "%@", notes[Int(displayData.n) % notes.endIndex])
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
                       sharps[Int(displayData.n) % sharps.endIndex])
            s.draw(at: NSMakePoint(x, y + textSizeLarger / 2),
                   withAttributes: attribs)

	    // Select font
            font = NSFont.boldSystemFont(ofSize: textSizeLarge)
            attribs = [.font: font]

            // Right justify
	    s = String(format: "%+4.2f\u{00A2}", displayData.c * 100.0)
            x = NSMaxX(rect) - s.size(withAttributes: attribs).width - 2
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

	    // Select font
            font = NSFont.boldSystemFont(ofSize: textSizeMedium)
            attribs = [.font: font]

	    y -= textSizeMedium + 4
            x = NSMinX(rect) + 2

	    s = String(format: "%4.2lfHz", displayData.fr)
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

            // Right justify
	    s = String(format: "%4.2lfHz", displayData.f)
            x = NSMaxX(rect) - s.size(withAttributes: attribs).width - 2
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

	    y -= textSizeMedium
            x = NSMinX(rect) + 2

	    s = String(format: "%4.2lfHz", (audioData.reference == 0) ?
		    kA5Reference : audioData.reference)
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)

            // Right justify
	    s = String(format: "%+4.2lfHz", displayData.f - displayData.fr)
            x = NSMaxX(rect) - s.size(withAttributes: attribs).width - 2
            s.draw(at: NSMakePoint(x, y), withAttributes: attribs)
        }

        // Display L if locked
        if (displayData.lock == true)
        {
            let font = NSFont.boldSystemFont(ofSize: kTextSize)
            let attribs: [NSAttributedStringKey: Any] = [.font: font]

            "L".draw(at: NSMakePoint(NSMinX(rect) + 2, NSMinY(rect)),
                     withAttributes: attribs)
        }
    }
}
