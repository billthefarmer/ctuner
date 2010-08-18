				Tuner
				=====

Musical instrument tuner using microphone input. Displays the input
waveform, a zoomable spectrum, details of the current note being
played, an optional strobe and a tuning meter. There are sliders to
adjust the input level and the reference frequency.

Using
=====

Play your instrument into your microphone and observe the display. The
slider at the top left adjusts the microphone input level, the
oscilloscope display at the top shows the input waveform. The display
below shows the spectrum of the input. It may be zoomed in and out by
clicking on the pane, or by pressing the 'Z' key. When zoomed in the
display shows about ±50 cents around the current note.

The numeric display shows the note, deviation in cents, correct
frequency, actual frequency, reference and frequency deviation. The
display may be locked by clicking on the pane, or by pressing the 'L'
key. The display values may be copied to the clipboard for pasting
into another application by typing Ctrl-C. The slider below and the +
and - buttons allow the reference to be adjusted. The strobe display
below shifts left or right according to whether the input note is flat
or sharp. It may be turned off by clicking on the pane, or by pressing
the 'S' key. The meter below shows the deviation in cents. The whole
display may be resized by pressing the 'R' key. Multiple tuning values
may be displayed in the strobe pane by pressing the 'T' key. The
Options button pops up the Options window, the Quit button quits the
application. The status bar shows the sample rate and the correction.

Clicking the right mouse button in a blank area of the tuner window
pops up a context menu which may be used to change all the options
above and to pop up the Options window and quit the application.

Options
=======

The zoom and strobe tick boxes may be used to turn the zoom and strobe
display off and on. The audio filter tick box controls a 3dB/octave
audio filter on the input. This is intended to make it easier to tune
low notes by reducing the level of harmonics. The display lock tick
box may be used to lock the display. The resize tick box may be used
to resize the display. The tremolo tick box may be used to display
multiple tuning values.

Some audio input devices appear to have inaccurate clocks. Out of
three laptops I have tested, two were up to 0.7% out. The correction
facility allows a correction factor to be entered. The value will not
be remembered until the save button is clicked. A known standard may
be used to set this value. For example, my netbook has a correction
value of 0.99322. The done button dismisses the window.

How it works
============

I have included a How It Works section because none of the other open
source tuners that I have looked at give any explanation of the
algorithms used. This tuner uses an overlapped Fast Fourier Transform
together with the phase difference between sequential runs of the FFT
to measure the actual frequency accurately. It appears to be about as
accurate as the clock in the sound card.

Files
=====

Changes			- Change log
COPYING			- Licence file
Makefile		- Build file
Readme.txt		- This file
Tuner.c			- Source file
Tuner.exe		- Application
Tuner.manifest		- Manifest file
Tuner.nsi		- NSIS installer script
Tuner.rc		- Resource file

Building
========

Use the makefile with Mingw or Cygwin GCC, or use a Microsoft C
compiler to build.




