# Tuner

A musical instrument tuner with extra facilities for tuning free reed
instruments

![Tuner]()
![Preferences]()
![Note Filters]()

 * Oscilloscope
 * Spectrum with zoom, shows individual reeds
 * Display frequency, cents, frequency deviation for single reed or up to eight
 * Strobe with choice of colours
 * Meter shows cents deviation
 * Display lock
 * Audio lowpass filter for bass reeds
 * Experimental downsampling for bass reeds
 * Fundamental filter
 * Note filter - allows display of selected notes and octaves only

This is a port to Swift of the old Carbon based app
[here](../mac). Responds to keyboard to change some preferences.

 * **c/k** - Change strobe colours
 * **d** - Toggle downsampling
 * **f** - Toggle audio filter
 * **l** - Toggle display lock
 * **m** - Toggle multiple reeds
 * **s** - Toggle strobe display
 * **z** - Toggle Spectrum zoom

 * Click on Scope to toggle audio filter
 * Click on Spectrum to toggle zoom
 * Click on display to lock
 * Click on strobe to enable
 