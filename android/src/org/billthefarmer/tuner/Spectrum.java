////////////////////////////////////////////////////////////////////////////////
//
//  Tuner - An Android Tuner written in Java.
//
//  Copyright (C) 2013  Bill Farmer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with this program; if not, write to the Free Software Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//  Bill Farmer  william j farmer [at] yahoo [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

package org.billthefarmer.tuner;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;

public class Spectrum extends Graticule
{
    protected MainActivity.Audio audio;

    static float max;

    public Spectrum(Context context, AttributeSet attrs)
    {
	super(context, attrs);
    }

    protected void onDraw(Canvas canvas)
    {
	super.onDraw(canvas);

	// Check for data

	if (audio.xa == null)
	    return;

	// Color green

	paint.setStrokeWidth(1);
	paint.setColor(0xff00ff00);

	// Translate camvas

	canvas.translate(0, height);

	if (max < 1.0f)
	    max = 1.0f;

	float yscale = (float)(max / height);

	max = 0.0f;

	float oldx = 0;
	float oldy = 0;

	for (int i = 0; i <= Math.min(width, audio.data.length); i++)
	{
	    if (max < audio.xa[i])
		max = (float)audio.xa[i];

	    float y = -(float)(audio.xa[i] / yscale);
	    canvas.drawLine(oldx, oldy, i, y, paint);

	    oldx = i;
	    oldy = y;
	}
    }
}
