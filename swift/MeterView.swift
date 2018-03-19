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

class MeterView: TunerView
{

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        let textSizeMedium: CGFloat = CGFloat(height / 5)
        let font = NSFont.systemFont(ofSize: textSizeMedium)
        let attribs: [NSAttributedStringKey: Any] = [.font: font]

        // Drawing code here.
        NSEraseRect(rect)

        // Move the origin
        let transform = AffineTransform(translationByX: NSMidX(rect), byY: 0)
        (transform as NSAffineTransform).concat()

        // Draw the meter scale
        for i in 0 ..< 6
        {
	    if (i == 0)
            {
                let offset = "0".size(withAttributes: attribs).width / 2
	        "0".draw(at: NSPoint(x: -offset,
                                     y: NSMaxY(rect) - textSizeMedium))
            }

	    else
	    {
	        let x = width / 11 * i;
	        let s = String(format:"%d", i * 10)
                let offset = s.size(withAttributes: attribs).width / 2

	        s.draw(at: NSPoint(x: x - offset,
                                   y: NSMaxY(rect) - textSizeMedium))
	        s.draw(at: NSPoint(x: -x - offset,
                                   y: NSMaxY(rect) - textSizeMedium))
	    }
        }

        CGContextSetShouldAntialias(context, false);
        CGContextBeginPath(context);

        for (int i = 0; i < 6; i++)
        {
	    int x = width / 11 * i;

	    CGContextMoveToPoint(context, x, 18);
	    CGContextAddLineToPoint(context, x, 24);

	    CGContextMoveToPoint(context, -x, 18);
	    CGContextAddLineToPoint(context, -x, 24);

	    for (int j = 1; j < 5; j++)
	    {
	        if (i < 5)
	        {
		    CGContextMoveToPoint(context, x + j * width / 55, 20);
		    CGContextAddLineToPoint(context, x + j * width / 55, 24);
	        }

	        CGContextMoveToPoint(context, -x + j * width / 55, 20);
	        CGContextAddLineToPoint(context, -x + j * width / 55, 24);
	    }
        }

        CGContextStrokePath(context);
    }
    
}
