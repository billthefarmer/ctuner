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
import android.graphics.LinearGradient;
import android.graphics.Matrix;
import android.graphics.Paint.Cap;
import android.graphics.Paint.Join;
import android.graphics.Paint.Style;
import android.graphics.Path;
import android.graphics.RectF;
import android.graphics.Shader.TileMode;
import android.util.AttributeSet;

// Meter

public class Meter extends TunerView
{
    MainActivity.Audio audio;

    LinearGradient gradient;
    Matrix matrix;
    RectF barRect;
    Path path;

    float cents;
    float medium;
    int margin;

    // Constructor

    public Meter(Context context, AttributeSet attrs)
    {
	super(context, attrs);
    }

    // OnSizeChanged

    protected void onSizeChanged(int w, int h, int oldw, int oldh)
    {
    	super.onSizeChanged(w, h, oldw, oldh);

	// Recalculate text size

	medium = height / 2.5f;
	paint.setTextSize(medium);

	// Scale text if necessary to fit it in

	float dx = paint.measureText("50");
	if (dx >= width / 11)
	    paint.setTextScaleX((width / 12) / dx);

	// Create a rect for the horizoltal bar

	barRect = new RectF(width / 36 - width / 2, -height / 32,
			    width / 2 - width / 36, height / 32);

	// Create a gradient

	gradient =
	    new LinearGradient(0, -height / 16, 0, height / 16,
			       0xff5f5f5f, 0xff9f9f9f, TileMode.CLAMP);

	// Create a path for the thumb

	path = new Path();

	path.moveTo(0, -1);
	path.lineTo(1, 0);
	path.lineTo(1, 1);
	path.lineTo(-1, 1);
	path.lineTo(-1, 0);
	path.close();

	// Create a matrix to scale the path,
	// a bit narrower than the height

	matrix = new Matrix();
	matrix.setScale(height / 10, height / 8);

	// Scale the path

	path.transform(matrix);
    }

    // OnDraw

    @SuppressLint("DefaultLocale")
    protected void onDraw(Canvas canvas)
    {
    	super.onDraw(canvas);

	// Reset the paint to black

    	paint.setColor(0xff000000);
    	paint.setStrokeWidth(1);
    	paint.setStyle(Style.FILL_AND_STROKE);
    	paint.setShader(null);

	// Translate the canvas down
	// and to the centre

   	canvas.translate(width / 2, medium);

	// Calculate x scale

	float xscale = width / 11;

	// Draw the scale legend

	for (int i = 0; i <= 5; i++)
	{
	    String s = String.format("%d", i * 10);
	    float dx = paint.measureText(s) / 2;
	    float x = i * xscale;

	    canvas.drawText(s, x - dx, 0, paint);
	    canvas.drawText(s, -x - dx, 0, paint);
	}

	// Wider lines for the scale

    	paint.setStrokeWidth(3);
    	paint.setStyle(Style.STROKE);
   	canvas.translate(0, medium / 2);

	// Draw the scale

	for (int i = 1; i <= 5; i++)
	{
	    float x = i * xscale;

	    canvas.drawLine(x, 0, x, -medium / 2, paint);
	    canvas.drawLine(-x, 0, -x, -medium / 2, paint);
	}

	// Draw the centre line a bit shorter
	// so it doesn't touch the zero

	canvas.drawLine(0, 0, 0, -medium / 2.5f, paint);

	// Draw the fine scale

	for (int i = 0; i <= 25; i++)
	{
	    float x = i * xscale / 5;

	    canvas.drawLine(x, 0, x, -medium / 4, paint);
	    canvas.drawLine(-x, 0, -x, -medium / 4, paint);
	}

	// Transform the canvas down
	// for the meter pointer

   	canvas.translate(0, medium / 2.5f);

	// Set the paint colour to
	// a paler shade of grey
	// and fill the bar

	paint.setColor(0xffdfdfdf);
	paint.setStyle(Style.FILL);
	canvas.drawRoundRect(barRect, 10, 10, paint);

	// Set the gradient and
	// draw the thumb outline

	paint.setShader(gradient);
	paint.setStyle(Style.STROKE);
	paint.setStrokeWidth(5);
	canvas.drawRoundRect(barRect, 10, 10, paint);

	// Translate the canvas to
	// the scaled cents value

	canvas.translate(cents * (xscale / 10), 0);

	// Set up the paint for
	// rounded corners

	paint.setStrokeCap(Cap.ROUND);
	paint.setStrokeJoin(Join.ROUND);
	
	// Remove the gradient, and
	// set fill style and fill
	// the thumb

	paint.setShader(null);
	paint.setStyle(Style.FILL);
	canvas.drawPath(path, paint);

	// Reset the matrix to scale the gradient
	// and draw the thumb outline

	matrix.reset();
	matrix.setScale(1, -1);
	gradient.setLocalMatrix(matrix);

	paint.setShader(gradient);
	paint.setStyle(Style.STROKE);

	canvas.drawPath(path, paint);
    }
}
