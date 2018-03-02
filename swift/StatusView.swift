//
//  Status.swift
//  Tuner
//
//  Created by Bill Farmer on 08/10/2017.
//  Copyright © 2017 Bill Farmer. All rights reserved.
//

import Cocoa

class StatusView: NSView
{
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.

        let path = NSBezierPath()
        path.move(to: NSPoint(x: 0,
                              y: NSHeight(dirtyRect) - 1))
        path.line(to: NSPoint(x: NSWidth(dirtyRect),
                              y: NSHeight(dirtyRect) - 1))
        path.stroke()
    }
}
