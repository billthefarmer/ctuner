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

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.View;

public class Status extends View
{
    MainActivity.Audio audio;

    int size;
    int width;
    int height;
    Paint paint;
    RectF rect;

    Object object;
    
    public Status(Context context, AttributeSet attrs)
    {
	super(context, attrs);
    }

    protected void onSizeChanged(int w, int h, int oldw, int oldh)
    {
	width = w;
	height = h;

	if (paint == null)
	    paint = new Paint();

	rect = new RectF(0, 0, width, height);
    }

    @SuppressLint("DefaultLocale")
    protected void onDraw(Canvas canvas)
    {
	rect.inset(1, 1);

	paint.setColor(0xff7f7f7f);
	paint.setStrokeWidth(3);
	paint.setFlags(Paint.ANTI_ALIAS_FLAG);
	paint.setStyle(Paint.Style.STROKE);
	canvas.drawLine(0, 0, width, 0, paint);

	paint.setStrokeWidth(0);
	paint.setColor(0xff000000);
	paint.setTextSize(height / 2);

	boolean bool;

	bool = (object != null);
	
	canvas.translate(width / 32, height * 2 / 3);
	String s = String.format("Min buffer size: %d, record = %b", size, bool);
	canvas.drawText(s, 0, 0, paint);
    }
}
