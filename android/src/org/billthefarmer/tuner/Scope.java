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

public class Scope extends Graticule
{
    protected MainActivity.Audio audio;

    static int max;

    public Scope(Context context, AttributeSet attrs)
    {
	super(context, attrs);
    }

    @Override
    protected void onDraw(Canvas canvas)
    {
	super.onDraw(canvas);

	// Check for data

	if (audio.data == null)
	    return;

	// Initialise sync

	int maxdx = 0;
	int dx = 0;
	int n = 0;

	for (int i = 1; i < audio.data.length; i++)
	{
	    dx = audio.data[i] - audio.data[i - 1];
	    if (maxdx < dx)
	    {
		maxdx = dx;
		n = i;
	    }

	    if (maxdx > 0 && dx < 0)
		break;
	}

	// Color green

	paint.setStrokeWidth(1);
	paint.setColor(0xff00ff00);

	// Translate camvas

	canvas.translate(0, height / 2);

	if (max < 4096)
	    max = 4096;

	float yscale = max / (height / 2);

	max = 0;

	float oldx = 0;
	float oldy = 0;

	for (int i = 0; i <= Math.min(width, audio.data.length - n); i++)
	{
	    if (max < Math.abs(audio.data[n + i]))
		max = Math.abs(audio.data[n + i]);

	    float y = -audio.data[n + i] / yscale;
	    canvas.drawLine(oldx, oldy, i, y, paint);

	    oldx = i;
	    oldy = y;
	}

    }
}
