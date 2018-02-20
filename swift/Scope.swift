//
//  Scope.swift
//  Tuner
//
//  Created by Bill Farmer on 08/10/2017.
//  Copyright © 2017 Bill Farmer. All rights reserved.
//

import Cocoa

class Scope: TunerView
{
    var buffer: Array<Float32>?

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.

        __NSRectFill(rect)

        let darkGreen = NSColor(red: 0, green: 0.5, blue: 0, alpha: 1.0)
        darkGreen.set()

        let transform = NSAffineTransform()
        transform.translateX(by: 0, yBy: NSMidY(rect))
        transform.concat()

        let path = NSBezierPath()

        for x in stride(from: 0, to: NSWidth(rect), by: 6)
        {
            path.move(to: NSPoint(x: x, y: NSHeight(rect) / 2))
            path.line(to: NSPoint(x: x, y: -NSHeight(rect) / 2))
        }

        for y in stride(from: 0, to: NSHeight(rect) / 2, by: 6)
        {
            path.move(to: NSPoint(x: NSMinX(rect), y: y))
            path.line(to: NSPoint(x: NSMaxX(rect), y: y))
            path.move(to: NSPoint(x: NSMinX(rect), y: -y))
            path.line(to: NSPoint(x: NSMaxX(rect), y: -y))
        }

        path.stroke()

        if (buffer == nil)
        {
            return
        }
    }
}
