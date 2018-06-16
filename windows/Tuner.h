////////////////////////////////////////////////////////////////////////////////
//
//  Tuner - A Tuner written in C++.
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

#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <windowsx.h>

// Macros
#define Length(a) (sizeof(a) / sizeof(a[0]))

#define WCLASS "MainWClass"
#define PCLASS "OptionWClass"
#define FCLASS "FilterWClass"

#define OCTAVE 12
#define MIN   0.5

#undef NOISE

// Tool ids
enum
    {TOOLBAR_ID = 100,
     SPECTRUM_ID,
     DISPLAY_ID,
     STROBE_ID,
     SCOPE_ID,
     METER_ID,
     QUIT_ID,

     // Options ids

     ZOOM_ID,
     TEXT_ID,
     DOWN_ID,
     LOCK_ID,
     MULT_ID,
     FUND_ID,
     NOTE_ID,
     FILT_ID,
     CLOSE_ID,
     FILTER_ID,
     ENABLE_ID,
     EXPAND_ID,
     UPDOWN_ID,
     COLOURS_ID,
     OPTIONS_ID,
     FILTERS_ID,
     REFERENCE_ID,

     // Filters ids

     NOTES_C,
     NOTES_Cs,
     NOTES_D,
     NOTES_Eb,
     NOTES_E,
     NOTES_F,
     NOTES_Fs,
     NOTES_G,
     NOTES_Ab,
     NOTES_A,
     NOTES_Bb,
     NOTES_B,

     OCTAVES_0,
     OCTAVES_1,
     OCTAVES_2,
     OCTAVES_3,
     OCTAVES_4,
     OCTAVES_5,
     OCTAVES_6,
     OCTAVES_7,
     OCTAVES_8};

// Bitmap ids
enum
    {OPTIONS_BM,
     QUIT_BM};

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
     RANGE = SAMPLES * 7 / 16,
     STEP = SAMPLES / OVERSAMPLE};

// Tuner reference values
enum
    {A5_REFNCE = 440,
     C5_OFFSET = 57};

// Reference values
enum
    {MAX_REF  = 4800,
     REF_REF  = 4400,
     MIN_REF  = 4200,
     STEP_REF = 10};

// Strobe colours
enum
    {BLUE,
     OLIVE,
     MAGENTA};

// Timer values
enum
    {METER_DELAY  = 10,
     STROBE_DELAY = 10};

// Window size
enum
    {WIDTH  = 400,
     HEIGHT = 480};

// View sizes
enum
    {SCOPE_HEIGHT    = 50,
     SPECTRUM_HEIGHT = 50,
     DISPLAY_HEIGHT  = 158,
     STROBE_HEIGHT   = 68,
     METER_HEIGHT    = 82,
     TOTAL_HEIGHT    = 408};

// Margins
enum
    {MARGIN  = 20,
     OFFSET = 32,
     SPACING = 8,
     TOTAL   = 72};

// Button sizes
enum
    {CHECK_HEIGHT = 24,
     CHECK_WIDTH = 124,
     NOTE_HEIGHT = 24,
     NOTE_WIDTH = 36,
     OCTAVE_HEIGHT = 24,
     OCTAVE_WIDTH = 76,
     BUTTON_HEIGHT = 26,
     BUTTON_WIDTH = 85};

// Window sizes
enum
    {GROUP_WIDTH = (CHECK_WIDTH * 2) + (MARGIN * 4),
     GROUP_HEIGHT = (CHECK_HEIGHT  * 4) + (MARGIN * 2) + (SPACING * 2),
     FILTER_WIDTH = (CHECK_WIDTH * 2) + (MARGIN * 4),
     FILTER_HEIGHT = (CHECK_HEIGHT * 6) + (MARGIN * 2) + (SPACING * 5),
     OPTIONS_WIDTH = GROUP_WIDTH + (MARGIN * 2) + 4,
     OPTIONS_HEIGHT = (GROUP_HEIGHT * 2) + (MARGIN * 2) + SPACING,
     FILTERS_WIDTH = FILTER_WIDTH + (MARGIN * 2) + 4,
     FILTERS_HEIGHT = FILTER_HEIGHT + (MARGIN * 2)};

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
    float x;
    float y;
} POINTF;

// Global data
typedef struct
{
    HWND hwnd;
    RECT wind;
    RECT rect;
} WINDOW, *WINDOWP;

typedef struct
{
    HWND hwnd;
    RECT rect;
} TOOL, *TOOLP;

typedef struct
{
    HWND hwnd;
    TOOLINFO info;
} TOOLTIP, *TOOLTIPP;

typedef struct
{
    HWND hwnd;
    RECT rect;
    UINT length;
    short *data;
} SCOPE, *SCOPEP;

typedef struct
{
    HWND hwnd;
    RECT rect;
    UINT length;
    UINT expand;
    BOOL zoom;
    float f;
    float r;
    float l;
    float h;
    int count;
    double *data;
    double *values;
} SPECTRUM, *SPECTRUMP;

typedef struct
{
    HWND hwnd;
    RECT rect;
    BOOL mult;
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
    TOOL filter;
    TOOL close;
    TOOL quit;
} BUTTON, *BUTTONP;

typedef struct
{
    TOOL notes[12];
    TOOL octaves[12];
} BOXES, *BOXESP;

typedef struct
{
    HWND hwnd;
    RECT rect;
    double c;
    HANDLE timer;
} METER, *METERP;

typedef struct
{
    HWND hwnd;
    RECT rect;
    double c;
    int colours;
    BOOL enable;
    BOOL changed;
    HANDLE timer;
} STROBE, *STROBEP;

typedef struct
{
    DWORD id;
    BOOL down;
    BOOL fund;
    BOOL note;
    BOOL filter;
    HWAVEIN hwi;
    HANDLE thread;
    double reference;
} AUDIO, *AUDIOP;

typedef struct
{
    BOOL note[12];
    BOOL octave[9];
} FILTER;

// Global handle
HINSTANCE hInst;

// Gdiplus token
ULONG_PTR token;

// Gdiplus input
Gdiplus::GdiplusStartupInput input;

// Global data
WINDOW window;
WINDOW options;
WINDOW filters;

TOOL toolbar;
TOOLTIP tooltip;
SCOPE scope;
SPECTRUM spectrum;
DISPLAY display;
STROBE strobe;
METER meter;

BUTTON button;

TOOL group;
TOOL zoom;
TOOL text;
TOOL lock;
TOOL down;
TOOL mult;
TOOL fund;
TOOL note;
TOOL filt;
TOOL enable;
TOOL expand;
TOOL updown;
TOOL colours;
TOOL reference;

BOXES boxes;
AUDIO audio;
FILTER filter;

// Function prototypes
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK OptionWProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK FilterWProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK EnumChildProc(HWND, LPARAM);
BOOL RegisterMainClass(HINSTANCE);
VOID GetSavedStatus(VOID);
BOOL DrawItem(WPARAM, LPARAM);
BOOL DrawStrobe(HDC, RECT);
BOOL DrawScope(HDC, RECT);
BOOL DrawSpectrum(HDC, RECT);
BOOL DrawDisplay(HDC, RECT);
BOOL DrawLock(HDC, int, int);
BOOL DrawMeter(HDC, RECT);
BOOL DisplayContextMenu(HWND, POINTS);
BOOL DisplayOptions(WPARAM, LPARAM);
BOOL DisplayFilters(WPARAM, LPARAM);
BOOL DisplayOptionsMenu(HWND, POINTS);
BOOL DisplayClicked(WPARAM, LPARAM);
BOOL SpectrumClicked(WPARAM, LPARAM);
BOOL StrobeClicked(WPARAM, LPARAM);
BOOL MeterClicked(WPARAM, LPARAM);
BOOL FilterClicked(WPARAM, LPARAM);
BOOL FundamentalClicked(WPARAM, LPARAM);
BOOL NoteFilterClicked(WPARAM, LPARAM);
BOOL ScopeClicked(WPARAM, LPARAM);
BOOL LockClicked(WPARAM, LPARAM);
BOOL ZoomClicked(WPARAM, LPARAM);
BOOL ExpandClicked(WPARAM, LPARAM);
BOOL ContractClicked(WPARAM, LPARAM);
BOOL MultipleClicked(WPARAM, LPARAM);
BOOL EnableClicked(WPARAM, LPARAM);
BOOL EditReference(WPARAM, LPARAM);
BOOL CharPressed(WPARAM, LPARAM);
BOOL CopyDisplay(WPARAM, LPARAM);
BOOL DownClicked(WPARAM, LPARAM);
BOOL BoxClicked(WPARAM, LPARAM);
BOOL ChangeReference(WPARAM, LPARAM);
BOOL WindowResize(HWND, WPARAM, LPARAM);
BOOL WindowResizing(HWND, WPARAM, LPARAM);
BOOL AddToolbarBitmap(HWND, LPCTSTR);
BOOL AddToolbarButtons(HWND);
VOID CALLBACK MeterCallback(PVOID, BOOL);
VOID CALLBACK StrobeCallback(PVOID, BOOL);
DWORD WINAPI AudioThread(LPVOID);
VOID WaveInData(WPARAM, LPARAM);
VOID fftr(complex[], int);

#endif
