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
clicking on the pane. When zoomed in the display shows about ±50 cents
around the current note.

The numeric display shows the note, deviation in cents, correct
frequency, actual frequency, reference and frequency deviation. The
slider below and the + and - buttons allow the reference to be
adjusted. The strobe display below shifts left or right according to
whether the input note is flat or sharp. It may be turned off by
clicking on the pane. The meter below shows the deviation in
cents. The Options button pops up the Options window, the Quit button
quits the application. The status bar shows the sample rate and the
correction.

Options
=======

The zoom and strobe tick boxes may be used to turn the zoom and strobe
display off and on. The Filter tick box controls a 3dB/octave audio
filter on the input. This is intended to make it easier to tune low
notes by reducing the level of harmonics.

Some audio input devices appear to have inaccurate clocks. Out of
three laptops I have tested, two were up to 7% out. The correction
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




