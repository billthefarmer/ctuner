//
//  Strobe.swift
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

class StrobeView: TunerView
{
    // Colours
    let colours: CGFloat[][][] =
      [[[0.25, 0.25, 1, 1], [0.25, 1, 1, 1]],
       [[0.5, 0.5, 0, 1], [0.5, 1, 0.85, 1]],
       [[1, 0.25, 1, 1], [1, 1, 0.25, 1]]];

    var foreground: NSColor!
    var background: NSColor!

    var gradient: NSGradient!

    var mc = 0.0
    var mx = 0.0

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        NSEraseRect(rect)

        if (!strobeData.enable)
        {
            return
        }

        // Create patterns
        if (gradient == nil || strobe.changed)
        {
	    // Create colours
	    foreground = NSColor(red: colours[strobeData.colours][0][0],
                                 green: colours[strobeData.colours][0][1],
                                 blue: colours[strobeData.colours][0][2],
                                 alpha: colours[strobeData.colours][0][3])
	    background = NSColor(red: colours[strobeData.colours][1][0],
                                 green: colours[strobeData.colours][1][1],
                                 blue: colours[strobeData.colours][1][2],
                                 alpha: colours[strobeData.colours][1][3])
	    // Create gradient
	    gradient = NSGradient(colors: [foreground, background, foreground])
	    strobe.changed = false;
        }

	mc = ((7.0 * mc) + strobeData.c) / 8.0;
	mx += mc * 50.0;

	if (mx > 160.0)
        {
	    mx = 0.0;
        }

	if (mx < 0.0)
        {
	    mx = 160.0;
        }

	let rx = mx - 160

        let context = NSGraphicsContext.current!

	if (abs(mc) > 0.4)
	{
	    background.set()
	    NSBezierPath.fill(NSMakeRect(0, 0, width, 10))
	}

	else if (abs(mc) > 0.2)
	{
	    for x in stride(from: mod(rx, 20), to: width, by: 20)
            {
	        gradient.draw(in: NSMakeRect(x, 0, 10, 10), angle: 0)
	    }
	}

	else
	{
	    for x in stride(from: mod(rx, 20), to: width, by: 20)
	    {
		foreground.set()
		NSBezierPath.fill(NSMakeRect(x, 0, 10, 10))

		background.set()
		NSBezierPath.fill(NSMakeRect(x + 10, 0, 10, 10))
	    }
	}

	if (fabsf(mc) > 0.4)
	{
	    for x in stride(from: mod(rx, 20), to: width, by: 40)
            {
	        gradient.draw(in: NSMakeRect(x, 10, 40, 10), angle: 0)
	    }
        }

	else
	{
	    for x in stride(from: mod(rx, 40), to: width, by: 40)
	    {
		foreground.set()
		NSBezierPath.fill(NSMakeRect(x, 10, 20, 10))

		background.set()
		NSBezierPath.fill(NSMakeRect(x + 20, 10, 20, 10))
	    }
	}

	for (float x = fmodf(rx, 80); x <= width; x += 80)
	{
	    foreground.set()
	    NSBezierPath.fill(NSMakeRect(x, 20, 40, 10))

	    background.set()
	    NSBezierPath.fill(NSMakeRect(x + 40, 20, 40, 10))
	}

	for (float x = fmodf(rx, 160); x <= width; x += 160)
	{
	    foreground.set()
	    NSBezierPath.fill(NSMakeRect(x, 30, 80, 10))

	    background.set()
	    NSBezierPath.fill(NSMakeRect(x + 80, 30, 80, 10))
	}
    }
}
