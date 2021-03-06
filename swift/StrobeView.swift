//
//  StrobeView.swift
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
    let colours: [[[CGFloat]]] =
      [[[0.25, 0.25, 1, 1], [0.25, 1, 1, 1]],
       [[0.5, 0.5, 0, 1], [0.5, 1, 0.85, 1]],
       [[1, 0.25, 1, 1], [1, 1, 0.25, 1]]]

    let kMaxColours = 3

    @objc var c = 0.0
    @objc var colour = 1
    @objc var enable = false
    {
        didSet
        {
            isHidden = !enable
            needsDisplay = true
        }
    }
    @objc var changed = false

    var foreground: NSColor!
    var background: NSColor!

    var gradient: NSGradient!

    var mc = 0.0
    var mx = CGFloat(0)

    // mouseDown
    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            enable = !enable
            if (strbBox != nil)
            {
                strbBox.state = enable ? .on: .off
            }
            staffView.enable = !enable
        }
    }

    // draw
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.

        // Create patterns
        if (gradient == nil || changed)
        {
	    // Create colours
	    foreground = NSColor(red: colours[colour][0][0],
                                 green: colours[colour][0][1],
                                 blue: colours[colour][0][2],
                                 alpha: colours[colour][0][3])
	    background = NSColor(red: colours[colour][1][0],
                                 green: colours[colour][1][1],
                                 blue: colours[colour][1][2],
                                 alpha: colours[colour][1][3])
	    // Create gradient
	    gradient = NSGradient(colors: [foreground, background, foreground])
	    changed = false;
        }

	mc = ((7 * mc) + c) / 8
	mx += CGFloat(mc * 50)

        let size = height / 4

	if (mx > size * 16)
        {
	    mx = 0.0;
        }

	if (mx < 0.0)
        {
	    mx = size * 16;
        }

	let rx = mx - size * 16

	if (abs(mc) > 0.4)
	{
	    background.set()
	    NSBezierPath.fill(NSMakeRect(rect.minX, rect.minY,
                                         width, size))
	}

	else if (abs(mc) > 0.2)
	{
	    for x in stride(from: CGFloat(rx).remainder(dividingBy: size * 2),
                            to: width + size * 2, by: size * 2)
            {
	        gradient.draw(in: NSMakeRect(x - size, rect.minY,
                                             size * 2, size), angle: 0)
	    }
	}

	else
	{
	    for x in stride(from: CGFloat(rx).remainder(dividingBy: size * 2),
                            to: width + size * 2, by: size * 2)
	    {
		foreground.set()
		NSBezierPath.fill(NSMakeRect(x, rect.minY, size, size))

		background.set()
		NSBezierPath.fill(NSMakeRect(x - size, rect.minY,
                                             size, size))
	    }
	}

	if (abs(mc) > 0.4)
	{
	    for x in stride(from: CGFloat(rx).remainder(dividingBy: size * 2),
                            to: width + size * 4, by: size * 4)
            {
	        gradient.draw(in: NSMakeRect(x, rect.minY + size, size * 4,
                                             size), angle: 0)
	    }
        }

	else
	{
	    for x in stride(from: CGFloat(rx).remainder(dividingBy: size * 4),
                            to: width + size * 4, by: size * 4)
	    {
		foreground.set()
		NSBezierPath.fill(NSMakeRect(x, rect.minY + size,
                                             size * 2, size))

		background.set()
		NSBezierPath.fill(NSMakeRect(x - size * 2, rect.minY + size,
                                             size * 2, size))
	    }
	}

	for x in stride(from: CGFloat(rx).remainder(dividingBy: size * 8),
                        to: width + size * 8, by: size * 8)
	{
	    foreground.set()
	    NSBezierPath.fill(NSMakeRect(x, rect.minY + size * 2, size * 4,
                                         size))

	    background.set()
	    NSBezierPath.fill(NSMakeRect(x - size * 4, rect.minY + size * 2,
                                         size * 4, size))
	}

	for x in stride(from: CGFloat(rx).remainder(dividingBy: size * 16),
                        to: width + size * 16, by: size * 16)
	{
	    foreground.set()
	    NSBezierPath.fill(NSMakeRect(x, rect.minY + size * 3, size * 8,
                                         size))

	    background.set()
	    NSBezierPath.fill(NSMakeRect(x - size * 8, rect.minY + size * 3,
                                         size * 8, size))
	}
    }
}
