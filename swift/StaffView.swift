//
//  StaffView.swift
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

    // Sharp symbol
    let sp: [[CGFloat]] =
      [
        [35, 35], // 0
        [8, 22], // 1
        [8, 46], // 2
        [35, 59], // 3
        [35, 101], // 4
        [8, 88], // 5
        [8, 111], // 6
        [35, 125], // 7
        [35, 160], // 8
        [44, 160], // 9
        [44, 129], // 10
        [80, 147], // 11
        [80, 183], // 12
        [89, 183], // 13
        [89, 151], // 14
        [116, 165], // 15
        [116, 141], // 16
        [89, 127], // 17
        [89, 86], // 18
        [116, 100], // 19
        [116, 75], // 20
        [89, 62], // 21
        [89, 19], // 22
        [80, 19], // 23
        [80, 57], // 23
        [44, 39], // 25
        [44, -1], // 26
        [35, -1], // 27
        [35, 35], // 28
        [44, 64], // 29
        [80, 81], // 30
        [80, 123], // 31
        [44, 105], // 32
        [44, 64], // 33
      ]

    // Flat symbol
    let ft: [[CGFloat]] =
      [
        [20, 86], // 0
        [28, 102.667], [41.6667, 111], [61, 111], // 3
        [71.6667, 111], [80.3333, 107.5], [87, 100.5], // 6
        [93.6667, 93.5], [97, 83.6667], [97, 71], // 9
        [97, 53], [89, 36.6667], [73, 22], // 12
        [57, 7.33333], [35.3333, -1.33333], [8, -4], // 15
        [8, 195], // 16
        [20, 195], // 17
        [20, 86], // 18
        [20, 7], // 19
        [35.3333, 9], [47.8333, 15.6667], [57.5, 27], // 22
        [67.1667, 38.3333], [72, 51.6667], [72, 67], // 25
        [72, 75.6667], [70.1667, 82.3333], [66.5, 87], // 28
        [62.8333, 91.6667], [57.3333, 94], [50, 94], // 31
        [41.3333, 94], [34.1667, 90.3333], [28.5, 83], // 34
        [22.8333, 75.6667], [20, 64.6667], [20, 50], // 37
        [20, 7], // 38
      ]

    // Scale offsets
    let offset: [CGFloat] =
      [
        0, 0, 1, 2, 2, 3,
        3, 4, 5, 5, 6, 6
      ]

    // Accidentals
    let accidentals =
      [
        kNatural, kSharp, kNatural, kFlat, kNatural, kNatural,
        kSharp, kNatural, kFlat, kNatural, kFlat, kNatural
      ]

    @objc var note = 0
    {
        didSet
        {
            needsDisplay = true
        }
    }
    @objc var enable = true
    {
        didSet
        {
            isHidden = !enable
            needsDisplay = true
        }
    }

    // mouseDown
    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            enable = !enable
            if (strbBox != nil)
            {
                strbBox.state = strobeView.enable ? .on: .off
            }
            strobeView.enable = !enable
        }
    }

    // draw
    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        let treble = NSBezierPath()
        let bass = NSBezierPath()
        let head = NSBezierPath()
        let sharp = NSBezierPath()
        let flat = NSBezierPath()

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
        bass.move(to: NSMakePoint(bc[28][0], bc[28][1]))
        for i in stride(from: 29, to: 34, by: 3)
        {
            bass.curve(to: NSMakePoint(bc[i + 2][0], bc[i + 2][1]),
                       controlPoint1: NSMakePoint(bc[i][0], bc[i][1]),
                       controlPoint2: NSMakePoint(bc[i + 1][0],
                                                  bc[i + 1][1]));
        }

        bass.move(to: NSMakePoint(bc[35][0], bc[35][1]))
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

        // Sharp
        sharp.move(to: NSMakePoint(sp[0][0], sp[0][1]))
        for i in 1 ... 28
        {
            sharp.line(to: NSMakePoint(sp[i][0], sp[i][1]))
        }

        sharp.move(to: NSMakePoint(sp[0][0], sp[0][1]))
        for i in 29 ... 33
        {
            sharp.line(to: NSMakePoint(sp[i][0], sp[i][1]))
        }

        // Flat
        flat.move(to: NSMakePoint(ft[0][0], ft[0][1]))
        for i in stride(from: 1, to: 15, by: 3)
        {
            flat.curve(to: NSMakePoint(ft[i + 2][0], ft[i + 2][1]),
                       controlPoint1: NSMakePoint(ft[i][0], ft[i][1]),
                       controlPoint2: NSMakePoint(ft[i + 1][0],
                                                  ft[i + 1][1]));
        }

        for i in 15 ... 18
        {
            flat.line(to: NSMakePoint(ft[i][0], ft[i][1]))
        }

        flat.move(to: NSMakePoint(ft[19][0], ft[19][1]))
        for i in stride(from: 19, to: 36, by: 3)
        {
            flat.curve(to: NSMakePoint(ft[i + 2][0], ft[i + 2][1]),
                       controlPoint1: NSMakePoint(ft[i][0], ft[i][1]),
                       controlPoint2: NSMakePoint(ft[i + 1][0],
                                                  ft[i + 1][1]));
        }
        flat.line(to: NSMakePoint(ft[38][0], ft[38][1]))

        // Drawing code here.
        NSEraseRect(rect)

        // Move the origin
        let context = NSGraphicsContext.current!
        context.cgContext.translateBy(x: 0, y: rect.midY)

        let lineHeight = height / 14
        let lineWidth = width / 16
        let margin = width / 32

        // Draw staff
        for i in 1 ... 5
        {
            let y = CGFloat(i) * lineHeight
            NSBezierPath.strokeLine(from: NSMakePoint(margin, y),
                                    to: NSMakePoint(width - margin, y))
            NSBezierPath.strokeLine(from: NSMakePoint(margin, -y),
                                    to: NSMakePoint(width - margin, -y))
        }

        // Draw leger lines
        NSBezierPath
          .strokeLine(from: NSMakePoint(width / 2 - lineWidth / 2, 0),
                      to: NSMakePoint(width / 2 + lineWidth / 2, 0))
        NSBezierPath
          .strokeLine(from: NSMakePoint(width / 2 + lineWidth * 5.5,
                                        lineHeight * 6),
                      to: NSMakePoint(width / 2 + lineWidth * 6.5,
                                      lineHeight * 6))
        NSBezierPath
          .strokeLine(from: NSMakePoint(width / 2 - lineWidth * 5.5,
                                        -lineHeight * 6),
                      to: NSMakePoint(width / 2 - CGFloat(lineWidth) * 6.5,
                                      -lineHeight * 6))

        // Scale treble clef
        var bounds = treble.bounds
        var scale = (height / 2) / (bounds.height)
        transform = AffineTransform(scale: scale)
        treble.transform(using: transform)
        transform = AffineTransform(translationByX: margin + lineWidth / 2,
                                    byY: lineHeight)
        treble.transform(using: transform)
        treble.fill()

        // Scale bass clef
        bounds = bass.bounds
        scale = (lineHeight * 4) / (bounds.height)
        transform = AffineTransform(scale: scale)
        bass.transform(using: transform)
        transform = AffineTransform(translationByX: margin + lineWidth / 4,
                                    byY: -lineHeight * 5.4)
        bass.transform(using: transform)
        bass.fill()

        // Scale note head
        bounds = head.bounds
        scale = (CGFloat(lineHeight) * 1.5) / (bounds.height)
        transform = AffineTransform(scale: scale)
        head.transform(using: transform)

        // Scale sharp
        bounds = sharp.bounds
        transform = AffineTransform(translationByX:
                                      -(bounds.minX + bounds.maxX) / 2,
                                    byY: -(bounds.minY + bounds.maxY) / 2)
        sharp.transform(using: transform)
        scale = (CGFloat(lineHeight) * 3) / (bounds.height)
        transform = AffineTransform(scale: scale)
        sharp.transform(using: transform)

        // Scale flat
        bounds = flat.bounds
        transform = AffineTransform(translationByX:
                                      -(bounds.minX + bounds.maxX) / 2,
                                    byY: -(bounds.minY + bounds.maxY) / 2)
        flat.transform(using: transform)
        scale = (lineHeight * 3) / (bounds.height)
        transform = AffineTransform(scale: scale)
        flat.transform(using: transform)

        // Calculate transform for note
        let xBase = lineWidth * 14;
        let yBase = lineHeight * 14;
        let note = self.note - translate[displayView.trans];
        var octave = note / kOctave;
        let index = (note + kOctave) % kOctave;

        // Wrap top two octaves
        if octave >= 6
        {
            octave -= 2;
        }

        // Wrap C0
        else if octave == 0 && index <= 1
        {
            octave += 4;
        }

        // Wrap bottom two octaves
        else if octave <= 1 || octave == 2 && index <= 1
        {
            octave += 2;
        }

        let dx = (CGFloat(octave) * lineWidth * 3.5) +
          (offset[Int(index)] * (lineWidth / 2));
        let dy = (CGFloat(octave) * lineHeight * 3.5) +
          (offset[Int(index)] * (lineHeight / 2));

        // Draw note
        transform = AffineTransform(translationByX: width / 2 - xBase + dx,
                                    byY: -yBase + dy)
        head.transform(using: transform)
        head.fill()

        switch accidentals[Int(index)]
        {
        case kNatural:
            // Do nothing
            break

        case kSharp:
            // Draw sharp
            transform =
              AffineTransform(translationByX:
                                width / 2 - xBase + dx - CGFloat(lineWidth / 2),
                              byY: -yBase + dy)
            sharp.transform(using: transform)
            sharp.fill()

        case kFlat:
            // Draw flat
            transform =
              AffineTransform(translationByX:
                                width / 2 - xBase + dx - CGFloat(lineWidth / 2),
                              byY: -yBase + dy + CGFloat(lineHeight / 2))
            flat.transform(using: transform)
            flat.fill()

        default:
            // Do nothing
            break
        }
    }
}
