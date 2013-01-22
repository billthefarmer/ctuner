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
import android.graphics.Paint.Style;
import android.graphics.Typeface;
import android.util.AttributeSet;

public class Display extends TunerView
{
    int n;
    float c;
    float df;
    float f;
    float fr;
    float r;

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

    public void setValues(int note, float cents, float diff,
			  float freq, float ref, float reff)
    {
	n = note;
	c = cents;
	df = diff;
	f = freq;
	fr = ref;
	r = reff;

	invalidate();
    }

    public void onSizeChanged(int w, int h, int oldw, int oldh)
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

    public void onDraw(Canvas canvas)
    {
    	super.onDraw(canvas);

    	paint.setColor(0xff000000);
    	paint.setStrokeWidth(1);
    	paint.setStyle(Style.FILL_AND_STROKE);
    	paint.setTypeface(Typeface.DEFAULT_BOLD);
    	paint.setTextSize(large);

    	canvas.translate(0, large);
    	canvas.drawText("C", margin, 0, paint);
    	
    	float dx = paint.measureText("C");
    	
    	paint.setTextSize(medium);
    	canvas.drawText("0", margin + dx, 0, paint);

    	paint.setTextSize(large);
    	dx = paint.measureText("+0.00¢");
    	canvas.drawText("+0.00¢", width - dx - margin, 0, paint);
    	
    	canvas.translate(0, medium);
    	paint.setTextSize(medium);
    	paint.setTypeface(Typeface.DEFAULT);
    	canvas.drawText("0000.00Hz", margin, 0, paint);
    	
    	dx = paint.measureText("0000.00Hz");
    	canvas.drawText("0000.00Hz", width - dx - margin, 0, paint);

    	canvas.translate(0, medium);
    	paint.setTextSize(medium);
    	canvas.drawText("440.00Hz", margin, 0, paint);

    	dx = paint.measureText("+0.00Hz");
    	canvas.drawText("+0.00Hz", width - dx - margin, 0, paint);

    }
}
