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

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;
import android.os.Bundle;
import android.preference.PreferenceManager;

import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

// Main Activity

public class MainActivity extends Activity
{
    Spectrum spectrum;
    Display display;
    Strobe strobe;
    Status status;
    Meter meter;
    Scope scope;

    Audio audio;

    // On Create

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

	// Find the views, not all may be present

	spectrum = (Spectrum)findViewById(R.id.spectrum);
	display = (Display)findViewById(R.id.display);
	strobe = (Strobe)findViewById(R.id.strobe);
	status = (Status)findViewById(R.id.status);
	meter = (Meter)findViewById(R.id.meter);
	scope = (Scope)findViewById(R.id.scope);

	// Load preferences

	PreferenceManager.setDefaultValues(this, R.xml.preferences, false);

	// Create audio

	audio = new Audio();

	// Connect views to audio

	if (spectrum != null)
	    spectrum.audio = audio;

	if (display != null)
	    display.audio = audio;

	if (strobe != null)
	    strobe.audio = audio;

	if (status != null)
	    status.audio = audio;

	if (scope != null)
	    scope.audio = audio;
    }

    // No menu yet

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
     	// Inflate the menu; this adds items to the action bar if it is present.

     	MenuInflater inflater = getMenuInflater();
     	inflater.inflate(R.menu.activity_main, menu);
     	return true;
    }

    // On start

    @Override
    protected void onStart()
    {
    	super.onStart();

    	// Start the audio thread

	audio.start();
    }

    @Override
    protected void onStop()
    {
    	super.onStop();

    	audio.stop();
    }

    @Override
    protected void onDestroy()
    {
    	super.onDestroy();

    	// thread = null;
    }

    public void onSettingsClick(MenuItem item)
    {
    	Intent intent = new Intent(this, SettingsActivity.class);
    	startActivity(intent);
    }

    void showAlert(String title, String message)
    {
	// Create an alert dialog builder

	AlertDialog.Builder builder =
	    new AlertDialog.Builder(this);

	// Set the title, message and button

	builder.setTitle(title);
	builder.setMessage(message);
	builder
	    .setNeutralButton(android.R.string.ok,
			      new DialogInterface.OnClickListener()
			      {				
				  @Override
				  public void onClick(DialogInterface dialog,
						      int which)
				  {
				      // Dismiss dialog

				      dialog.dismiss();	
				  }
			      });
	// Create the dialog

	AlertDialog dialog = builder.create();

	// Show it

	dialog.show();
    }

    protected class Audio
    {
	protected boolean filter;
	protected boolean downsample;
	protected Thread thread;
	protected double reference;
	protected double buffer[];
	protected short data[];

	protected double frequency;
	protected double nearest;
	protected double difference;
	protected double cents;
	protected int n;

	private AudioRecord audioRecord;

	final int MAXIMA = 8;
	final int OVERSAMPLE = 16;
	final int SAMPLES = 16384;
	final int RANGE = SAMPLES * 3 / 8;
	final int STEP = SAMPLES / OVERSAMPLE;

	final int C5_OFFSET = 57;

	private final double G = 3.023332184e+01;
	private final double K = 0.9338478249;

	final double fps = (double)11025 / (double)SAMPLES;
	final double expect = 2.0 * Math.PI * (double)STEP / (double)SAMPLES;

	private double xv[];
	private double yv[];

	private double dmax;

	private double xr[];
	private double xi[];

	protected double xa[];
	private double xp[];
	private double xf[];
	private double dx[];

	// Constructor

	Audio()
	{
	    buffer = new double[SAMPLES];
	    data = new short[STEP];
	    
	    xv = new double[2];
	    yv = new double[2];

	    xr = new double[SAMPLES];
	    xi = new double[SAMPLES];

	    xa = new double[RANGE];
	    xp = new double[RANGE];
	    xf = new double[RANGE];
	    dx = new double[RANGE];
	}

	// Start audio

	void start()
	{
	    // Start the thread

	    new Thread(new Runnable()
		{
		    public void run()
		    {
			processAudio();
		    }
		}).start();
	}

	// Process Audio

	void processAudio()
	{
	    int size;

	    // Save the thread

	    thread = Thread.currentThread();

	    size = AudioRecord
		.getMinBufferSize(11025,
				  AudioFormat.CHANNEL_IN_MONO,
				  AudioFormat.ENCODING_PCM_16BIT);
	    if (size <= 0)
	    {
		runOnUiThread(new Runnable()
		    {
			public void run()
			{
			    showAlert("Alert!",
				      "Audio min buffer size not working!");
			}
		    });

		thread = null;
		return;
	    }

	    // Create the AudioRecord object

	    try
	    {
		audioRecord =
		    new AudioRecord(AudioSource.MIC, 11025,
				    AudioFormat.CHANNEL_IN_MONO,
				    AudioFormat.ENCODING_PCM_16BIT, 4096);
	    }

	    catch (Exception e)
	    {
		runOnUiThread(new Runnable()
		    {
			public void run()
			{
			    showAlert("Alert!",
				      "Audio record not working!");
			}
		    });
	    }

	    int state = audioRecord.getState(); 

	    if (state != AudioRecord.STATE_INITIALIZED)
	    {
		runOnUiThread(new Runnable()
		    {
			public void run()
			{
			    showAlert("Alert!",
				      "Audio record not initialised!");
			}
		    });

		thread = null;
		return;
	    }

	    // Start recording

	    audioRecord.startRecording();

	    // Continue until the thread is stopped

	    while (thread != null)
	    {
		// Read a buffer of data

		size = audioRecord.read(data, 0, data.length);

		// Stop the thread if no data

		if (size == 0)
		    thread = null;

		// Move the main data buffer up

		System.arraycopy(buffer, data.length, buffer, 0, data.length);

		// Butterworth filter, 3dB/octave

		for (int i = 0; i < data.length; i++)
		{
		    xv[0] = xv[1];
		    xv[1] = (double)data[i] / G;

		    yv[0] = yv[1];
		    yv[1] = (xv[0] + xv[1]) + (K * yv[0]);

		    // Choose filtered/unfiltered data

		    buffer[(SAMPLES - STEP) + i] =
			audio.filter? yv[1]: (double)data[i];
		}

		// Check there is a scope view

		if (scope != null)
		    scope.postInvalidate();

		// Maximum data value

		if (dmax < 4096.0)
		    dmax = 4096.0;

		// Calculate normalising value

		double norm = dmax;
		dmax = 0.0;

		// Copy data to FFT input arrays for tuner

		for (int i = 0; i < buffer.length; i++)
		{
		    // Find the magnitude

		    if (dmax < Math.abs(buffer[i]))
			dmax = Math.abs(buffer[i]);

		    // Calculate the window

		    double window =
			0.5 - 0.5 * Math.cos(2.0 * Math.PI *
					     i / SAMPLES);

		    // Normalise and window the input data

		    xr[i] = buffer[i] / norm * window;
		}

		// do FFT for tuner

		fftr(xr, xi);

		// Process FFT output for tuner

		for (int i = 1; i < xa.length; i++)
		{
		    double real = xr[i];
		    double imag = xi[i];

		    xa[i] = Math.hypot(real, imag);

		    // Do frequency calculation

		    double p = Math.atan2(imag, real);
		    double dp = xp[i] - p;

		    xp[i] = p;

		    // Calculate phase difference

		    dp -= (double)i * expect;

		    int qpd = (int)(dp / Math.PI);

		    if (qpd >= 0)
			qpd += qpd & 1;

		    else
			qpd -= qpd & 1;

		    dp -=  Math.PI * (double)qpd;

		    // Calculate frequency difference

		    double df = OVERSAMPLE * dp / (2.0 * Math.PI);

		    // Calculate actual frequency from slot frequency plus
		    // frequency difference and correction value

		    xf[i] = (i * fps + df * fps);

		    // Calculate differences for finding maxima

		    dx[i] = xa[i] - xa[i - 1];
		}

		if (spectrum != null)
		    spectrum.postInvalidate();
	    }

	    // Stop and realease the audio recorder

	    audioRecord.stop();
	    audioRecord.release();
	}

	// Real to complex FFT, ignores imaginary values in input array

	private void fftr(double ar[], double ai[])
	{
	    double norm = Math.sqrt(1.0 / ar.length);

	    for (int i = 0, j = 0; i < ar.length; i++)
	    {
		if (j >= i)
		{
		    double tr = ar[j] * norm;

		    ar[j] = ar[i] * norm;
		    ai[j] = 0.0;

		    ar[i] = tr;
		    ai[i] = 0.0;
		}

		int m = n / 2;
		while (m >= 1 && j >= m)
		{
		    j -= m;
		    m /= 2;
		}
		j += m;
	    }
    
	    for (int mmax = 1, istep = 2 * mmax; mmax < n;
		 mmax = istep, istep = 2 * mmax)
	    {
		double delta = (Math.PI / mmax);
		for (int m = 0; m < mmax; m++)
		{
		    double w = m * delta;
		    double wr = Math.cos(w);
		    double wi = Math.sin(w);

		    for (int i = m; i < ar.length; i += istep)
		    {
			int j = i + mmax;
			double tr = wr * ar[j] - wi * ai[j];
			double ti = wr * ai[j] + wi * ar[j];
			ar[j] = ar[i] - tr;
			ai[j] = ai[i] - ti;
			ar[i] += tr;
			ai[i] += ti;
		    }
		}
	    }
	}

	void stop()
	{
	    thread = null;
	}
    }
}
