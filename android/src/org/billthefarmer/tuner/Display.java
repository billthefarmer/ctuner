////////////////////////////////////////////////////////////////////////////////
//
//  Tuner - An Android Tuner written in Java.
//
//  Copyright (C) 2013	Bill Farmer
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
//  Bill Farmer	 william j farmer [at] yahoo [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

package org.billthefarmer.tuner;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint.Style;
import android.graphics.Typeface;
import android.util.AttributeSet;

// Class Display

public class Display extends TunerView
{
    protected MainActivity.Audio audio;

    int large;
    int medium;
    int small;
    
    int margin;

    final String notes[] =
    {"C", "C#", "D", "Eb", "E", "F",
     "F#", "G", "Ab", "A", "Bb", "B"};

    public Display(Context context, AttributeSet attrs)
    {
	super(context, attrs);
    }

    protected void onSizeChanged(int w, int h, int oldw, int oldh)
    {
	super.onSizeChanged(w, h, oldw, oldh);
	
	large = height / 3;
	medium = height / 4;
	small = height / 6;
	margin = width / 32;

	paint.setTextSize(medium);
	float dx = paint.measureText("0000.00Hz");
	
	if (dx + (margin * 2) >= width / 2)
	{
	    float xscale = (width / 2) / (dx + (margin * 2));
	    paint.setTextScaleX(xscale);
	}
    }

    @SuppressLint("DefaultLocale")
    protected void onDraw(Canvas canvas)
    {
	super.onDraw(canvas);

	paint.setColor(Color.BLACK);
	paint.setStrokeWidth(1);
	paint.setStyle(Style.FILL_AND_STROKE);
	paint.setTypeface(Typeface.DEFAULT_BOLD);

	if (audio == null)
	    return;

	canvas.translate(0, large);
	paint.setTextSize(large);
	canvas.drawText(notes[audio.n % 12], margin, 0, paint);
	
	float dx = paint.measureText(notes[audio.n % 12]);
	
	paint.setTextSize(medium);
	String s = Integer.toString(audio.n / 12);
	canvas.drawText(s, margin + dx, 0, paint);

	paint.setTextSize(large);
	s = String.format("%+5.2f¢", audio.cents);
	dx = paint.measureText(s);
	canvas.drawText(s, width - dx - margin, 0, paint);
	
	canvas.translate(0, medium);
	paint.setTextSize(medium);
	paint.setTypeface(Typeface.DEFAULT);
	s = String.format("%4.2fHz", audio.nearest);
	canvas.drawText(s, margin, 0, paint);
	
	s = String.format("%4.2fHz", audio.frequency);
	dx = paint.measureText(s);
	canvas.drawText(s, width - dx - margin, 0, paint);

	canvas.translate(0, medium);
	paint.setTextSize(medium);
	s = String.format("%4.2fHz", audio.reference);
	canvas.drawText(s, margin, 0, paint);

	s = String.format("%+5.2fHz", audio.difference);
	dx = paint.measureText(s);
	canvas.drawText(s, width - dx - margin, 0, paint);

    }
}
