# Introduction #

An Android version is in development. It may also be found on [Github](https://github.com/billthefarmer/tuner). The apk may also be found on [F-Droid](http://f-droid.org/repository/browse/?fdcategory=Multimedia&fdid=org.billthefarmer.tuner&fdpage=1).

![http://ctuner.googlecode.com/svn/wiki/images/Tuner-portrait.png](http://ctuner.googlecode.com/svn/wiki/images/Tuner-portrait.png)
![http://ctuner.googlecode.com/svn/wiki/images/Tuner-settings.png](http://ctuner.googlecode.com/svn/wiki/images/Tuner-settings.png)
![http://ctuner.googlecode.com/svn/wiki/images/Tuner-landscape.png](http://ctuner.googlecode.com/svn/wiki/images/Tuner-landscape.png)

## Using ##

Hold the phone/tablet near the instrument, play a note and observe the display. In portrait the app shows a scope with the input signal, a spectrum showing the processed signal, the main display with the note value, cents error, frequency error etc, a strobe display and a meter display showing the cents error. In landscape the app shows just the main display and the meter. Using the touch setting changes below causes a short notification message to pop up for a few seconds.

### Scope ###

Touch the scope to add an input low pass filter to make it easier to tune bass reeds my removing harmonics. Touch again to remove it. The colours have been chosen both for clarity, and for that 'retro' look like a real piece of hardware.

### Spectrum ###

Touch the spectrum to zoom in to the current note. The display can show up to eight reeds. Touch again to zoom back out. Touch and hold to add a downsampling feature to help with bass reeds. This may produce spurious results. Touch and hold again to remove.

### Display ###

Touch the display to lock the current results. Touch again to remove the lock. Display lock is not preserved if the screen is rotated. Touch and hold the display to switch between displaying the closest result to the reference note and up to eight results. This setting is not remembered the next time you use the app.

### Strobe ###

Touch the strobe to turn it on and off. Strobe colours are configurable in the settings.

### Meter ###

The meter shows the cents error up to plus or minus 50 cents. Touch the meter to copy the results to the clipboard. Touch and hold to prevent the display turning itself off. Touch and hold again to allow the display to turn off.

### Status bar ###

The status bar shows the input sample rate and the status of the various settings.

## Settings ##

All the settings are preserved except the multiple note option and will be remembered the next time you use the app.

### Input ###

The **input source** may be changed. This doesn't make any difference on my tablet, but it may on a phone.

#### Audio filter ####

The are **audio filter** item controls a low pass filter in the audio input before processing.

#### Downsample ####

The **downsample** item controls a downsampling feature on the audio input during processing to help with bass reeds. This feature may cause spurious results

### Display ###

#### Keep the display on ####

The screen item controls the display backlight.

#### Spectrum zoom ####

The **spectrum zoom** item controls the zoom feature of the spectrum display.

#### Display multiple notes ####

The **multiple notes** item controls the display of more than one result on the display.

### Strobe ###

#### Strobe ####

The **strobe** item controls the strobe display.

#### Strobe colours ####

If the strobe is enabled, there is an option to change the **colours**. There are three predefined settings and a custom setting. If the custom setting is selected the two colours may be changed individually. touch the coloured circle to select a colour.

![http://ctuner.googlecode.com/svn/wiki/images/Tuner-colourpicker.png](http://ctuner.googlecode.com/svn/wiki/images/Tuner-colourpicker.png)
![http://ctuner.googlecode.com/svn/wiki/images/Tuner-reference.png](http://ctuner.googlecode.com/svn/wiki/images/Tuner-reference.png)

### Reference ###

The **reference** frequency for 'A' may be changed over a limited range. This will be remembered, so change it back to 440Hz before you quit the app.

### About ###

The **about** item shows the copyright and licence.