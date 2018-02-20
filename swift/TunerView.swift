//
//  TunerView.swift
//  Tuner
//
//  Created by Bill Farmer on 08/10/2017.
//  Copyright © 2017 Bill Farmer. All rights reserved.
//

import Cocoa

class TunerView: NSView
{
    var rect: NSRect = NSRect()

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        rect = DrawEdge(dirtyRect)
    }

    func DrawEdge(_ rect: NSRect) -> NSRect
    {
        // Save context
        NSGraphicsContext.current?.saveGraphicsState()

        // Set colour
        let grey = NSColor(white: 0.5, alpha: 1.0)
        grey.set()

        // Draw edge
        let path = NSBezierPath(roundedRect: rect, xRadius: 8, yRadius: 8)
        path.lineWidth = 2
        path.stroke()

        // Restore context before clip
        NSGraphicsContext.current?.restoreGraphicsState()

        // Create inset
        let inset = NSInsetRect(rect, 2, 2)
        __NSRectClip(inset)

        return inset
    }
}
