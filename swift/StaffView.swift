//
//  StaffView.swift
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

class StaffView: TunerView
{

    // Treble clef
    let tc: [[CGFloat]] =
    [
      [-6, 16], [-8, 13],
      [-14, 19], [-10, 35], [2, 35],
      [8, 37], [21, 30], [21, 17],
      [21, 5], [10, -1], [0, -1],
      [-12, -1], [-23, 5], [-23, 22],
      [-23, 29], [-22, 37], [-7, 49],
      [10, 61], [10, 68], [10, 73],
      [10, 78], [9, 82], [7, 82],
      [2, 78], [-2, 68], [-2, 62],
      [-2, 25], [10, 18], [11, -8],
      [11, -18], [5, -23], [-4, -23],
      [-10, -23], [-15, -18], [-15, -13],
      [-15, -8], [-12, -4], [-7, -4],
      [3, -4], [3, -20], [-6, -17],
      [-5, -23], [9, -20], [9, -9],
      [7, 24], [-5, 30], [-5, 67],
      [-5, 78], [-2, 87], [7, 91],
      [13, 87], [18, 80], [17, 73],
      [17, 62], [10, 54], [1, 45],
      [-5, 38], [-15, 33], [-15, 19],
      [-15, 7], [-8, 1], [0, 1],
      [8, 1], [15, 6], [15, 14],
      [15, 23], [7, 26], [2, 26],
      [-5, 26], [-9, 21], [-6, 16]
    ]

    // Bass clef
    let bc: [[CGFloat]] =
      [
        [-2.3,3],
        [6,7], [10.5,12], [10.5,16],
        [10.5,20.5], [8.5,23.5], [6.2,23.3],
        [5.2,23.5], [2,23.5], [0.5,19.5],
        [2,20], [4,19.5], [4,18],
        [4,17], [3.5,16], [2,16],
        [1,16], [0,16.9], [0,18.5],
        [0,21], [2.1,24], [6,24],
        [10,24], [13.5,21.5], [13.5,16.5],
        [13.5,11], [7,5.5], [-2.0,2.8],
        [14.9,21],
        [14.9,22.5], [16.9,22.5], [16.9,21],
        [16.9,19.5], [14.9,19.5], [14.9,21],
        [14.9,15],
        [14.9,16.5], [16.9,16.5], [16.9,15],
        [16.9,13.5], [14.9,13.5], [14.9,15]
      ]

    // Note head
    let hd: [[CGFloat]] =
      [
        [8.0, 0.0],
        [8.0, 8.0], [-8.0, 8.0], [-8.0, 0.0],
        [-8.0, -8.0], [8.0, -8.0], [8.0, 0.0]
      ]

    // Scale offsets
    let offset:  [Int] =
      [
        0, 0, 1, 2, 2, 3,
        3, 4, 5, 5, 6, 6
      ]

    let treble = NSBezierPath()
    let bass = NSBezierPath()
    let head = NSBezierPath()

    // mouseDown
    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            staffData.enable = !staffData.enable
            strobeData.enable = !staffData.enable
            if (strbBox != nil)
            {
                strbBox.state = strobeData.enable ? .on: .off
            }

            isHidden = !staffData.enable
            strobeView.isHidden = !strobeData.enable
            needsDisplay = true
        }
    }

    // draw
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        NSEraseRect(rect)
        if treble.isEmpty
        {
            // Treble clef
            treble.move(to: NSMakePoint(tc[0][0], tc[0][1]))
            treble.line(to: NSMakePoint(tc[1][0], tc[1][1]))
            for i in stride(from: 2, to: tc.count, by: 3)
            {
                treble.curve(to: NSMakePoint(tc[i + 2][0], tc[i + 2][1]),
                             controlPoint1: NSMakePoint(tc[i][0], tc[i][1]),
                             controlPoint2: NSMakePoint(tc[i + 1][0],
                                                        tc[i + 1][1]));
            }

            // Bass clef
            bass.move(to: NSMakePoint(bc[0][0], bc[0][1]))
            for i in stride(from: 1, to: 27, by: 3)
            {
                bass.curve(to: NSMakePoint(bc[i + 2][0], bc[i + 2][1]),
                           controlPoint1: NSMakePoint(bc[i][0], bc[i][1]),
                           controlPoint2: NSMakePoint(bc[i + 1][0],
                                                      bc[i + 1][1]));
            }

            // Two dots
            bass.move(to: NSMakePoint(bc[28][0], bc[25][1]))
            for i in stride(from: 29, to: 34, by: 3)
            {
                bass.curve(to: NSMakePoint(bc[i + 2][0], bc[i + 2][1]),
                           controlPoint1: NSMakePoint(bc[i][0], bc[i][1]),
                           controlPoint2: NSMakePoint(bc[i + 1][0],
                                                      bc[i + 1][1]));
            }

            bass.move(to: NSMakePoint(bc[35][0], bc[25][1]))
            for i in stride(from: 36, to: bc.count, by: 3)
            {
                bass.curve(to: NSMakePoint(bc[i + 2][0], bc[i + 2][1]),
                           controlPoint1: NSMakePoint(bc[i][0], bc[i][1]),
                           controlPoint2: NSMakePoint(bc[i + 1][0],
                                                      bc[i + 1][1]));
            }

            // Note head
            head.move(to: NSMakePoint(hd[0][0], hd[0][1]))
            for i in stride(from: 1, to: hd.count, by: 3)
            {
                head.curve(to: NSMakePoint(hd[i + 2][0], hd[i + 2][1]),
                           controlPoint1: NSMakePoint(hd[i][0], hd[i][1]),
                           controlPoint2: NSMakePoint(hd[i + 1][0],
                                                      hd[i + 1][1]));
            }
        }
    }
}
