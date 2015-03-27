# Introduction #

A Mac version has been released.

![http://ctuner.googlecode.com/svn/wiki/images/TunerMac.png](http://ctuner.googlecode.com/svn/wiki/images/TunerMac.png)  ![http://ctuner.googlecode.com/svn/wiki/images/Preferences.png](http://ctuner.googlecode.com/svn/wiki/images/Preferences.png)

# Progress #

  * The basic layout and graphics are done.
  * The audio capture and processing system is working.
  * Updated version released.

## Differences ##

There are a number of differences from the Windows version:

  * The Mac audio system calculates the actual sample rate, so correction is redundant.
  * The volume control has been removed as it does not work in version 10.7 (Lion).
  * The strobe pane animation is jerkier, due to the type of timer available on the Mac.
  * Clicking on the window upsets the audio system momentarily.
  * Use the zoom button to resize the window.
  * Uses the mac vDSP library for audio analysis.
  * Options may be changed by pressing keys, they are:
    * C, Ctrl-C - Copy values to clipboard
    * D - Switch downsampling on and off
    * F - Switch filter on and off
    * K - Change strobe colours
    * L - Lock display
    * M - Display multiple sets of values
    * S - Switch strobe display off and on
    * T - Display multiple sets of values
    * Z - Zoom spectrum display
    * + - Expand spectrum display
    * - - Contract spectrum display

## Downloads ##

<wiki:gadget url="http://google-code-project-hosting-gadgets.googlecode.com/svn/build/prod/downloads/gcDownloads.xml" up\_projectName="ctuner" border="0"/>