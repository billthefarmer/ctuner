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

// Macros
#define Length(a) (sizeof(a) / sizeof(a[0]))

#define WCLASS "MainWClass"
#define PCLASS "PopupClass"

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
     REFERENCE_ID};

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
     SPACING = 8,
     TOTAL   = 72};

// Button sizes
enum
    {CHECK_HEIGHT = 24,
     CHECK_WIDTH = 124,
     BUTTON_HEIGHT = 26,
     BUTTON_WIDTH = 85};

// Popup sizes
enum
    {GROUP_WIDTH = (CHECK_WIDTH * 2) + (MARGIN * 4),
     GROUP_HEIGHT = (CHECK_HEIGHT  * 4) + (MARGIN * 2) + (SPACING * 2),
     POPUP_WIDTH = GROUP_WIDTH + (MARGIN * 2) + 4,
     POPUP_HEIGHT = (GROUP_HEIGHT * 2) + (MARGIN * 4)};

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
    value *values;
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

// Global handle
HINSTANCE hInst;

// Gdiplus token
ULONG_PTR token;

// Gdiplus input
Gdiplus::GdiplusStartupInput input;

// Global data
WINDOW window;
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
TOOL filter;
TOOL enable;
TOOL expand;
TOOL updown;
TOOL colours;
TOOL options;
TOOL reference;

AUDIO audio;

// Function prototypes.
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PopupProc(HWND, UINT, WPARAM, LPARAM);
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
