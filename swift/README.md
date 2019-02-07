# Tuner

A musical instrument tuner with extra facilities for tuning free reed
instruments

![Tuner](https://github.com/billthefarmer/billthefarmer.github.io/blob/master/images/ctuner/Tuner-swift.png)
![Preferences](https://github.com/billthefarmer/billthefarmer.github.io/blob/master/images/ctuner/Tuner-preferences.png)
![Note Filters](https://github.com/billthefarmer/billthefarmer.github.io/blob/master/images/ctuner/Note-filter.png)

 * Oscilloscope
 * Spectrum with zoom, shows individual reeds
 * Display frequency, cents, frequency deviation for single reed or up to eight
 * Strobe with choice of colours
 * Staff displays current note
 * Meter shows cents deviation
 * Display lock
 * Audio lowpass filter for bass reeds
 * Experimental downsampling for bass reeds
 * Fundamental filter
 * Note filter - allows display of selected notes and octaves only

This is a port to Swift of the old Carbon based app
[here](../mac). Responds to keyboard to change some preferences.

### Keys
 * **c** - Change strobe colours
 * **d** - Toggle downsampling
 * **f** - Toggle audio filter
 * **k** - Change strobe colours
 * **l** - Toggle display lock
 * **m** - Toggle multiple reeds
 * **s** - Toggle strobe/staff display
 * **z** - Toggle Spectrum zoom

### Actions
 * Click on scope to toggle audio filter
 * Click on spectrum to toggle zoom
 * Click on display to lock
 * click on meter to lock
 * Click on strobe to toggle
 * Click on staff to toggle
