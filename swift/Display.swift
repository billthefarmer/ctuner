//
//  Display.swift
//  Tuner
//
//  Created by Bill Farmer on 08/10/2017.
//  Copyright © 2017 Bill Farmer. All rights reserved.
//

import Cocoa

class Display: TunerView
{

    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            displayData.lock = !displayData.lock
            needsDisplay = true
        }
    }

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.

        NSEraseRect(rect)

        "Test".draw(at: NSPoint(x: 2, y: 2))
    }
    
}
