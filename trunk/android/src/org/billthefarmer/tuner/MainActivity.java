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

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
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

    Thread thread;

    AudioRecord audio;

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

	// Start the audio thread

	startAudio();
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

    public void settings(MenuItem item)
    {
    	Intent intent = new Intent(this, SettingsActivity.class);
    	startActivity(intent);
    }

    // Start Audio

    void startAudio()
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
        final int size;
        short buffer[];

        // Save the thread

	thread = Thread.currentThread();
			
	size = AudioRecord.getMinBufferSize(11025, AudioFormat.CHANNEL_IN_MONO,
					    AudioFormat.ENCODING_PCM_16BIT);

	if (size < 0)
	    return;

	// Create the AudioRecord object

	try
	{
	    audio =
		new AudioRecord(AudioSource.MIC, 11025,
				AudioFormat.CHANNEL_IN_MONO,
				AudioFormat.ENCODING_PCM_16BIT, 4096);
	}

	catch (Exception e)
	{}

	AlertDialog.Builder builder =
	    new AlertDialog.Builder(this);

	builder.setMessage(R.string.alert_message);
	builder.setTitle(R.string.alert_title);

	AlertDialog dialog = builder.create();
	dialog.show();

	//	    return;

	int state = audio.getState(); 

	if (state == AudioRecord.STATE_INITIALIZED)
	{
	    buffer = new short[1024];

	    audio.startRecording();
	
	    audio.read(buffer, 0, buffer.length);

	    audio.stop();

	    audio.release();
	}

	runOnUiThread(new Runnable()
	    {
		public void run()
		{
		    status.update(size, audio);
		}
	    });

	thread = null;

	while (thread != null)
	{
	}
    }
}
