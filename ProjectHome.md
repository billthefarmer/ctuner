# Musical instrument strobe tuner #

A musical instrument strobe tuner with cents and frequency display. With additional features useful for tuning accordions or melodeons. The strobe display may be disabled, if required. Windows and [Mac](http://code.google.com/p/ctuner/wiki/Mac) versions have been released. An [Android](http://code.google.com/p/ctuner/wiki/Android) version has been released. The android version may also be found on [Github](https://github.com/billthefarmer/tuner).

![http://ctuner.googlecode.com/svn/wiki/images/Tuner.png](http://ctuner.googlecode.com/svn/wiki/images/Tuner.png)  ![http://ctuner.googlecode.com/svn/wiki/images/Options.png](http://ctuner.googlecode.com/svn/wiki/images/Options.png)


## Using ##

Play your instrument into your microphone and observe the display. The slider at the top left adjusts the microphone input level, the oscilloscope display at the top shows the input waveform. The display below shows the spectrum of the input. It may be zoomed in and out by clicking on the pane. When zoomed in the display shows about ±50 cents around the current note and includes all concurrent notes within that range.

The numeric display shows the note, deviation in cents, correct frequency, actual frequency, reference and frequency deviation. The display may be locked by clicking on the pane. The displayed values may be copied to the clipboard for pasting into another application by typing Ctrl-C. The strobe display below shifts left or right according to whether the input note is flat or sharp. It may be turned off by clicking on the pane. The meter below shows the deviation in cents. The Options button pops up the Options window, the Quit button quits the application. The status bar shows the sample rate and the correction.

## Options ##

The zoom and strobe tick boxes may be used to turn the zoom and strobe display off and on. The Filter tick box controls a 3dB/octave audio filter on the input. This is intended to make it easier to tune low notes by reducing the level of harmonics. The downsample tick box controls a downsampling algorithm that enhanced the fundamental frequency in the input. It also generates spurious subharmonics that should be ignored. The display lock tick box may be used to lock the display. The resize tickbox may be used to resize the display. The multiple tickbox may be used to display multiple sets of values. The slider below and the + and - buttons allow the reference to be adjusted.

Some audio input devices appear to have inaccurate clocks. Out of three laptops I have tested, two were up to 0.7% out. The correction facility allows a correction factor to be entered. The value will not be remembered until the save button is clicked. A known standard may be used to set this value. For example, my netbook has a correction value of 0.99322. The done button dismisses the window.

## Enhancements ##

  * Added the facility to lock the display and to copy the displayed values to the clipboard. The values are delimited by tab characters so that they paste into adjacent cells in Open Office Calc. I assume this also works in Excel, not tested.

  * Added display resize function. This makes everything twice the size, except for the slider which isn't resizeable. The buttons look a bit odd too, but all the rest of the display resizes ok.

  * Added the options above to the pop-up context menu.

  * Added multiple option for displaying multiple sets of values.

  * All the options may be changed by pressing keys, they are:
    * C, Ctrl-C - Copy values to clipboard
    * D - Switch downsampling on and off
    * F - Switch filter off and on
    * K - Change strobe colours
    * L - Lock display
    * M - Display multiple sets of values
    * O - Pop up Options dialog
    * R - Resize display
    * S - Switch strobe display off and on
    * T - Display multiple sets of values
    * Z - Zoom spectrum display
    * + - Expand spectrum display
    * - - Contract spectrum display

  * Removed reference slider from main window to options window and replaced with a text box with up/down arrows.
  * Spectrum now shows cents.
  * Added downsampling algorithm and spectrum expansion.
  * Added coloured spectrum and real sharp and flat signs.

## How it works ##

I have included a How It Works section because none of the other open source tuners that I have looked at give any explanation of the algorithms used. This tuner uses an overlapped Fast Fourier Transform together with the phase difference between sequential runs of the FFT to measure the actual frequency accurately. It appears to be about as accurate as the clock in the sound card. The filter is a single pole Butterworth filter with a corner frequency of 120Hz. The output of the FFT is scanned by a peak detector which detects up to eight peaks in the spectrum. The highest peak is taken as the measured note, other peaks are shown on the zoomed in spectrum if within range, all of them are displayed if the multiple option is on.

## Bugs ##

  * The microphone input slider does not work correctly on a machine with more than one audio device under Windows XP. It adjusts something, I haven't found what yet. It works fine under Windows 7 and probably Vista. Use the recording control to adjust the level on XP if necessary.

  * Even with the filter, it can be difficult to get the correct display tuning accordion bass reeds due to the high level of harmonics. Using a microphone with a good bass response and pointing it right at the bass end helps. The downsampling algorithm is intended to address this problem.

  * I am a bit suspicious of the way the actual frequency always seems to appear on the low side of the spectrum peak at lower frequencies. The actual frequency checks out ok despite this. Fixed - added an offset to correct for this. Fixed again, did it right this time.
