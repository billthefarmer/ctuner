////////////////////////////////////////////////////////////////////////////////
//
//  Tuner - A Tuner written in C.
//
//  Copyright (C) 2009  Bill Farmer
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

#ifndef TUNER_H
#define TUNER_H

#define _WIN32_IE    0x0501
#define _WIN32_WINNT 0x0500

#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>

// Macros

#define Length(a) (sizeof(a) / sizeof(a[0]))

#define WCLASS "MainWClass"
#define PCLASS "PopupClass"

#define OCTAVE 12
#define MIN   0.5

#undef NOISE

// Tool ids

enum
    {SCOPE_ID = 101,
     SPECTRUM_ID,
     DISPLAY_ID,
     STROBE_ID,
     VOLUME_ID,
     STATUS_ID,
     SLIDER_ID,
     METER_ID,
     QUIT_ID,
     ZOOM_ID,
     TEXT_ID,
     SAVE_ID,
     LOCK_ID,
     CLOSE_ID,
     RESIZE_ID,
     FILTER_ID,
     ENABLE_ID,
     OPTIONS_ID,
     MULTIPLE_ID,
     REFERENCE_ID,
     CORRECTION_ID};

// Wave in values

enum
    {SAMPLE_RATE = 11025L,
     BITS_PER_SAMPLE = 16,
     BLOCK_ALIGN = 2,
     CHANNELS = 1};

// Audio processing values

enum
    {MAXIMA = 8,
     OVERSAMPLE = 16,
     SAMPLES = 16384,
     RANGE = SAMPLES * 3 / 8,
     STEP = SAMPLES / OVERSAMPLE,
     LORANGE = STEP * 3 / 8};

// Tuner reference values

enum
    {A5_REFNCE = 440,
     C5_OFFSET = 57};

// Slider values

enum
    {MAX_VOL  = 100,
     MIN_VOL  = 0,
     STEP_VOL = 10,

     MAX_REF  = 4500,
     REF_REF  = 4400,
     MIN_REF  = 4300,
     STEP_REF = 10,

     MAX_METER = 200,
     REF_METER = 100,
     MIN_METER = 0,

     MAX_CORRECTION = 101000,
     REF_CORRECTION = 100000,
     MIN_CORRECTION =  99000};

// Timer values

enum
    {METER_DELAY  = 100,
     STROBE_DELAY = 100};

// Window size

enum
    {WIDTH  = 320,
     HEIGHT = 396};

// Memory size

enum
    {MEM_SIZE = 1024};

// Structs

typedef struct
{
    double r;
    double i;
} complex;

typedef struct
{
    double f;
    double fr;
    int n;
} maximum;

typedef struct
{
    float f;
    float r;
    float l;
    float h;
} value;

// Global data

typedef struct
{
    HWND hwnd;
    BOOL zoom;
    RECT rwnd;
    RECT rclt;
} WINDOW, *WINDOWP;

typedef struct
{
    HWND hwnd;
} TOOL, *TOOLP;

typedef struct
{
    HWND hwnd;
    TOOLINFO info;
} TOOLTIP, *TOOLTIPP;

typedef struct
{
    HWND hwnd;
    UINT length;
    short *data;
} SCOPE, *SCOPEP;

typedef struct
{
    HWND hwnd;
    UINT length;
    BOOL zoom;
    float f;
    float r;
    float l;
    float h;
    int count;
    double *data;
    value *values;
} SPECTRUM, *SPECTRUMP;

typedef struct
{
    HWND hwnd;
    BOOL multiple;
    BOOL lock;
    double f;
    double fr;
    double c;
    int n;
    int count;
    maximum *maxima;
} DISPLAY, *DISPLAYP;

typedef struct
{
    TOOL options;
    TOOL save;
    TOOL close;
    TOOL quit;
} BUTTON, *BUTTONP;

typedef struct
{
    TOOL sample;
    TOOL reference;
    TOOL correction;
} LEGEND, *LEGENDP;

typedef struct
{
    HWND hwnd;
    double c;
    TOOL slider;
    HANDLE timer;
} METER, *METERP;

typedef struct
{
    HWND hwnd;
    double c;
    BOOL enable;
    HANDLE timer;
} STROBE, *STROBEP;

typedef struct
{
    DWORD id;
    BOOL filter;
    HWAVEIN hwi;
    HANDLE thread;
    double correction;
    double reference;
} AUDIO, *AUDIOP;

typedef struct
{
    HMIXER hmx;
    MIXERLINE *pmxl;
    MIXERCONTROL *pmxc;
    MIXERLINECONTROLS *pmxlc;
    MIXERCONTROLDETAILS *pmxcd;
    MIXERCONTROLDETAILS_UNSIGNED *pmxcdu;
} MIXER, *MIXERP;

// Function prototypes.

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PopupProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ResizeProc(HWND, LPARAM);
BOOL RegisterMainClass(HINSTANCE);
VOID GetSavedStatus(VOID);
BOOL ResizeWindow(WPARAM, LPARAM);
BOOL DrawItem(WPARAM, LPARAM);
BOOL DrawStrobe(HDC, RECT);
BOOL DrawScope(HDC, RECT);
BOOL DrawSpectrum(HDC, RECT);
BOOL DrawDisplay(HDC, RECT);
BOOL DrawLock(HDC, int, int);
BOOL DrawMeter(HDC, RECT);
BOOL DisplayContextMenu(HWND, POINTS);
BOOL DisplayOptions(WPARAM, LPARAM);
BOOL DisplayOptionsMenu(HWND, POINTS);
BOOL DisplayClicked(WPARAM, LPARAM);
BOOL SpectrumClicked(WPARAM, LPARAM);
BOOL StrobeClicked(WPARAM, LPARAM);
BOOL MeterClicked(WPARAM, LPARAM);
BOOL FilterClicked(WPARAM, LPARAM);
BOOL ResizeClicked(WPARAM, LPARAM);
BOOL ScopeClicked(WPARAM, LPARAM);
BOOL LockClicked(WPARAM, LPARAM);
BOOL ZoomClicked(WPARAM, LPARAM);
BOOL MultipleClicked(WPARAM, LPARAM);
BOOL EnableClicked(WPARAM, LPARAM);
BOOL EditReference(WPARAM, LPARAM);
BOOL EditCorrection(WPARAM, LPARAM);
BOOL ChangeVolume(WPARAM, LPARAM);
BOOL VolumeChange(WPARAM, LPARAM);
BOOL CharPressed(WPARAM, LPARAM);
BOOL CopyDisplay(WPARAM, LPARAM);
BOOL ChangeCorrection(WPARAM, LPARAM);
BOOL ChangeReference(WPARAM, LPARAM);
BOOL SaveCorrection(WPARAM, LPARAM);
VOID UpdateStatusBar(VOID);
VOID CALLBACK MeterCallback(PVOID, BOOL);
VOID CALLBACK StrobeCallback(PVOID, BOOL);
VOID TooltipShow(WPARAM, LPARAM);
VOID TooltipPop(WPARAM, LPARAM);
DWORD WINAPI AudioThread(LPVOID);
VOID WaveInData(WPARAM, LPARAM);
VOID fftr(complex[], int);

#endif
