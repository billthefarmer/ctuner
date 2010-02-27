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
//  Bill Farmer  william j farmer [at] tiscali [dot] co [dot] uk.
//
///////////////////////////////////////////////////////////////////////////////

#define _WIN32_IE 0x0501
#define _WIN32_WINNT 0x0500

#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>

// Macros

#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

#define WCLASS "MainWClass"

#define OCTAVE 12
#define MIN 0.5

// Global handle

HINSTANCE hInst;

// Tool ids

enum
    {SCOPE_ID = 101,
     SPECTRUM_ID,
     DISPLAY_ID,
     STROBE_ID,
     METER_ID,
     PLUS_ID,
     MINUS_ID,
     VOLUME_ID,
     STATUS_ID,
     SLIDER_ID,
     REFERENCE_ID};

// FFT direction

enum
    {FORWARD =  1,
     REVERSE = -1};

// Wave in values

enum
    {SAMPLE_RATE = 11025L,
     BITS_PER_SAMPLE = 16,
     BLOCK_ALIGN = 2,
     CHANNELS = 1};

// Audio processing values

enum
    {OVERSAMPLE = 4,
     SAMPLES = 4096,
     STEP = (SAMPLES / OVERSAMPLE)};

// Tuner reference values

enum
    {A5_REFNCE = 440,
     A5_OFFSET = 60,
     C8_OFFSET = 99};

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
     MIN_METER = 0};

// Timer values

enum
    {STROBE_DELAY = 100,
     TIMER_DELAY = 10 * 1000};

// Window size

enum
    {WIDTH  = 320,
     HEIGHT = 406};

// Global data

typedef struct
{
    double re;
    double im;
} COMPLEX, *COMPLEXP;

typedef struct
{
    HWND hwnd;
} TOOL, *TOOLP;

TOOL window;
TOOL status;
TOOL edit;

typedef struct
{
    HWND hwnd;
    TOOLINFO info;
} TOOLTIP, *TOOLTIPP;

TOOLTIP tooltip;

TOOL volume;

typedef struct
{
    HWND hwnd;
    UINT length;
    short *data;
} SCOPE, *SCOPEP;

SCOPE scope;

typedef struct
{
    HWND hwnd;
    UINT length;
    BOOL zoom;
    UINT k;
    double *data;
} SPECTRUM, *SPECTRUMP;

SPECTRUM spectrum;

typedef struct
{
    HWND hwnd;
    double f;
    double ff;
    double c;
    double e;
    int n;
} DISPLAY, *DISPLAYP;

DISPLAY display;

TOOL adjust;

struct
{
    TOOL plus;
    TOOL minus;
} button;

typedef struct
{
    HWND hwnd;
    double c;
    TOOL slider;
} METER, *METERP;

METER meter;

typedef struct
{
    HWND hwnd;
    double c;
    BOOL enable;
    HANDLE timer;
} STROBE, STROBEP;

STROBE strobe;

typedef struct
{
    DWORD id;
    DWORD then;
    DWORD sample;
    HWAVEIN hwi;
    HANDLE timer;
    HANDLE thread;
    BOOL trigger;
    double reference;
} AUDIO, *AUDIOP;

AUDIO audio;

typedef struct
{
    HMIXER hmx;
    MIXERLINE *pmxl;
    MIXERCONTROL *pmxc;
    MIXERLINECONTROLS *pmxlc;
    MIXERCONTROLDETAILS *pmxcd;
    MIXERCONTROLDETAILS_UNSIGNED *pmxcdu;
} MIXER, *MIXERP;

MIXER mixer;

// Function prototypes.

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL register_class(HINSTANCE);
BOOL DrawItem(WPARAM, LPARAM);
BOOL DrawStrobe(HDC, RECT);
BOOL DrawScope(HDC, RECT);
BOOL DrawSpectrum(HDC, RECT);
BOOL DrawDisplay(HDC, RECT);
BOOL DrawMeter(HDC, RECT);
BOOL DrawMinus(HDC, RECT, UINT);
BOOL DrawPlus(HDC, RECT, UINT);
BOOL PlusClicked(WPARAM, LPARAM);
BOOL DisplayClicked(WPARAM, LPARAM);
BOOL SpectrumClicked(WPARAM, LPARAM);
BOOL StrobeClicked(WPARAM, LPARAM);
BOOL MinusClicked(WPARAM, LPARAM);
BOOL ChangeVolume(WPARAM, LPARAM);
BOOL ChangeReference(WPARAM, LPARAM);
DWORD WINAPI AudioThread(LPVOID);
VOID WaveInData(WPARAM, LPARAM);
VOID UpdateMeter(METERP);
VOID CALLBACK StrobeCallback(PVOID, BOOL);
VOID CALLBACK TimerCallback(PVOID, BOOL);
VOID complexToComplex(int, int, COMPLEXP);

// Application entry point.

int WINAPI WinMain(HINSTANCE hInstance,
		   HINSTANCE hPrevInstance,
		   LPSTR lpszCmdLine,
		   int nCmdShow)
{
    // Initialize common controls to get the new style controls, also
    // dependent on manifest file

    InitCommonControls();

    // Check for a previous instance of this app

    if (!hPrevInstance)
	if (!register_class(hInstance))
	    return FALSE;

    // Save the application-instance handle.

    hInst = hInstance;

    // Create the main window.

    window.hwnd =
	CreateWindow(WCLASS, "Tuner",
		     WS_OVERLAPPED | WS_MINIMIZEBOX |
		     WS_SYSMENU,
		     CW_USEDEFAULT, CW_USEDEFAULT,
		     CW_USEDEFAULT, CW_USEDEFAULT,
		     NULL, 0, hInst, NULL);

    // If the main window cannot be created, terminate
    // the application.

    if (!window.hwnd)
	return FALSE;

    // Show the window and send a WM_PAINT message to the window
    // procedure.

    ShowWindow(window.hwnd, nCmdShow);
    UpdateWindow(window.hwnd);

    // Process messages

    MSG msg;
    BOOL flag;

    while ((flag = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0)
    {
	if (flag == -1)
	    break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// Register class

BOOL register_class(HINSTANCE hint)
{
    // Fill in the window class structure with parameters
    // that describe the main window.

    WNDCLASS wc = 
	{CS_HREDRAW | CS_VREDRAW,
	 MainWndProc,
	 0, 0, hInst,
	 LoadIcon(NULL, IDI_WINLOGO),
	 LoadCursor(NULL, IDC_ARROW),
	 GetSysColorBrush(COLOR_WINDOW),
	 NULL, WCLASS};

    // Register the window class.

    return RegisterClass(&wc);
}

// Main window procedure

LRESULT CALLBACK MainWndProc(HWND hWnd,
			     UINT uMsg,
			     WPARAM wParam,
			     LPARAM lParam)
{
    // Window dimensions

    RECT rWnd;
    RECT rClt;

    switch (uMsg)
    {
    case WM_CREATE:

	// Get the window and client dimensions

	GetWindowRect(hWnd, &rWnd);
	GetClientRect(hWnd, &rClt);

	// Calculate desired window width and height

	int border = (rWnd.right - rWnd.left) - rClt.right;
	int width  = WIDTH + border;
	int height = HEIGHT;

	// Set new dimensions

	SetWindowPos(hWnd, NULL, 0, 0,
		     width, height,
		     SWP_NOMOVE | SWP_NOZORDER);

	// Get client dimensions

	GetWindowRect(hWnd, &rWnd);
	GetClientRect(hWnd, &rClt);

	width = rClt.right - rClt.left;
	height = rClt.bottom - rClt.top;

	// Create volume slider

	volume.hwnd =
	    CreateWindow(TRACKBAR_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 TBS_VERT | TBS_NOTICKS,
			 8, 8, 24, 72, hWnd,
			 (HMENU)VOLUME_ID, hInst, NULL);

	// Set the slider range

	SendMessage(volume.hwnd, TBM_SETRANGE, TRUE,
		    MAKELONG(MIN_VOL, MAX_VOL));
	SendMessage(volume.hwnd, TBM_SETPAGESIZE, 0, STEP_VOL);

	// Create tooltip

	tooltip.hwnd =
	    CreateWindow(TOOLTIPS_CLASS, NULL,
			 WS_POPUP | TTS_ALWAYSTIP,
			 CW_USEDEFAULT, CW_USEDEFAULT,
			 CW_USEDEFAULT, CW_USEDEFAULT,
			 hWnd, NULL, hInst, NULL);

	SetWindowPos(tooltip.hwnd, HWND_TOPMOST, 0, 0, 0, 0,
		     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// Add volume to tooltip

	tooltip.info.cbSize = sizeof(tooltip.info);
	tooltip.info.hwnd = hWnd;
	tooltip.info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;

	tooltip.info.uId = (UINT_PTR)volume.hwnd;
	tooltip.info.lpszText = "Microphone volume";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create scope display

	scope.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 40, 8, width - 48, 32, hWnd,
			 (HMENU)SCOPE_ID, hInst, NULL);

	// Add scope to tooltip

	tooltip.info.uId = (UINT_PTR)scope.hwnd;
	tooltip.info.lpszText = "Scope";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create spectrum display

	spectrum.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 40, 48, width - 48, 32, hWnd,
			 (HMENU)SPECTRUM_ID, hInst, NULL);

	// Add spectrum to tooltip

	tooltip.info.uId = (UINT_PTR)spectrum.hwnd;
	tooltip.info.lpszText = "Spectrum, click to zoom";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create display

	display.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 8, 88, width - 16, 108, hWnd,
			 (HMENU)DISPLAY_ID, hInst, NULL);

	// Add display to tooltip

	tooltip.info.uId = (UINT_PTR)display.hwnd;
	tooltip.info.lpszText = "Display";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create minus button

	button.minus.hwnd =
	    CreateWindow(WC_BUTTON, NULL,
			 WS_VISIBLE | WS_CHILD |
			 BS_OWNERDRAW,
			 8, 206, 16, 16, hWnd,
			 (HMENU)MINUS_ID, hInst, NULL);

	// Add minus button to tooltip

	tooltip.info.uId = (UINT_PTR)button.minus.hwnd;
	tooltip.info.lpszText = "Reference frequency";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create reference slider

	adjust.hwnd =
	    CreateWindow(TRACKBAR_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 TBS_HORZ | TBS_NOTICKS | TBS_TOP,
			 24, 202, width - 48, 24, hWnd,
			 (HMENU)REFERENCE_ID, hInst, NULL);

	SendMessage(adjust.hwnd, TBM_SETRANGE, TRUE,
		    MAKELONG(MIN_REF, MAX_REF));
	SendMessage(adjust.hwnd, TBM_SETPAGESIZE, 0, STEP_REF);
	SendMessage(adjust.hwnd, TBM_SETPOS, TRUE, REF_REF);

	// Add adjust to tooltip

	tooltip.info.uId = (UINT_PTR)adjust.hwnd;
	tooltip.info.lpszText = "Reference frequency";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create plus button

	button.plus.hwnd =
	    CreateWindow(WC_BUTTON, NULL,
			 WS_VISIBLE | WS_CHILD |
			 BS_OWNERDRAW,
			 width - 24, 206, 16, 16, hWnd,
			 (HMENU)PLUS_ID, hInst, NULL);

	// Add plus button to tooltip

	tooltip.info.uId = (UINT_PTR)button.plus.hwnd;
	tooltip.info.lpszText = "Reference frequency";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create strobe

	strobe.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 8, 236, width - 16, 44, hWnd,
			 (HMENU)STROBE_ID, hInst, NULL);

	strobe.enable = TRUE;

	// Create tooltip for strobe

	tooltip.info.uId = (UINT_PTR)strobe.hwnd;
	tooltip.info.lpszText = "Strobe, click to disable/enable";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create meter

	meter.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 8, 288, width - 16, 56, hWnd,
			 (HMENU)METER_ID, hInst, NULL);

	// Add meter to tooltip

	tooltip.info.uId = (UINT_PTR)meter.hwnd;
	tooltip.info.lpszText = "Cents";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create meter slider

	meter.slider.hwnd =
	    CreateWindow(TRACKBAR_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 TBS_HORZ | TBS_NOTICKS | TBS_TOP,
			 12, 310, width - 23, 26, hWnd,
			 (HMENU)SLIDER_ID, hInst, NULL);

	SendMessage(meter.slider.hwnd, TBM_SETRANGE, TRUE,
		    MAKELONG(MIN_METER, MAX_METER));
	SendMessage(meter.slider.hwnd, TBM_SETPOS, TRUE, REF_METER);

	// Add slider to tooltip

	tooltip.info.uId = (UINT_PTR)meter.slider.hwnd;
	tooltip.info.lpszText = "Cents";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create status bar

	status.hwnd =
	    CreateWindow(STATUSCLASSNAME,
			 "\tCalibrating sample rate and correction factor",
			 WS_VISIBLE | WS_CHILD,
			 0, 0, 0, 0, hWnd,
			 (HMENU)STATUS_ID, hInst, NULL);

	// Create edit popup window

	edit.hwnd =
	    CreateWindow(WC_EDIT, "Tuner Log",
			 WS_VISIBLE | WS_POPUP |
			 WS_CAPTION | WS_VSCROLL |
			 ES_MULTILINE | ES_READONLY,
			 rWnd.right + 10, rWnd.top,
			 WIDTH * 3 / 2 + border, HEIGHT, hWnd,
			 (HMENU)NULL, hInst, NULL);

	SetWindowText(edit.hwnd, "");

	// Start audio thread

	audio.thread = CreateThread(NULL, 0, AudioThread, hWnd, 0, &audio.id);

	// Start strobe timer

	CreateTimerQueueTimer(&strobe.timer, NULL,
			      (WAITORTIMERCALLBACK)StrobeCallback,
			      &strobe.hwnd, STROBE_DELAY, STROBE_DELAY,
			      WT_EXECUTEDEFAULT);
	break;

    // Colour static text

    case WM_CTLCOLORSTATIC:
	return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
	break;

	// Draw item

    case WM_DRAWITEM:
	return DrawItem(wParam, lParam);
	break;

	// Disable menus by capturing this message

    case WM_INITMENU:
	break;

	// Capture system character key to stop pop up menus and other
	// nonsense

    case WM_SYSCHAR:
	break;

	// Volume control

    case WM_VSCROLL:
	ChangeVolume(wParam, lParam);

	// Set the focus back to the window

	SetFocus(hWnd);
	break;

	// Reference

    case WM_HSCROLL:
	ChangeReference(wParam, lParam);

	// Set the focus back to the window

	SetFocus(hWnd);
	break;

	// Set the focus back to the window by clicking

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
	SetFocus(hWnd);
	break;

	// Buttons

    case WM_COMMAND:
	switch (LOWORD(wParam))
	{
	case PLUS_ID:
	    PlusClicked(wParam, lParam);
	    break;

	case MINUS_ID:
	    MinusClicked(wParam, lParam);
	    break;

	case DISPLAY_ID:
	    DisplayClicked(wParam, lParam);
	    break;

	case SPECTRUM_ID:
	    SpectrumClicked(wParam, lParam);
	    break;

	case STROBE_ID:
	    StrobeClicked(wParam, lParam);
	    break;
	}

	// Set the focus back to the window

	SetFocus(hWnd);
	break;

        // Process other messages.

    case WM_DESTROY:

	PostQuitMessage(0);
	break;

	// Everything else

    default:
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

// Draw item

BOOL DrawItem(WPARAM wParam, LPARAM lParam)
{
    LPDRAWITEMSTRUCT lpdi = (LPDRAWITEMSTRUCT)lParam;
    UINT state = lpdi->itemState;
    RECT rect = lpdi->rcItem;
    HDC hdc = lpdi->hDC;

    switch (wParam)
    {
    case SCOPE_ID:
	return DrawScope(hdc, rect);
	break;

    case SPECTRUM_ID:
	return DrawSpectrum(hdc, rect);
	break;

    case STROBE_ID:
	return DrawStrobe(hdc, rect);
	break;

    case DISPLAY_ID:
	return DrawDisplay(hdc, rect);
	break;

    case METER_ID:
	return DrawMeter(hdc, rect);
	break;

    case MINUS_ID:
	return DrawMinus(hdc, rect, state);
	break;

    case PLUS_ID:
	return DrawPlus(hdc, rect, state);
	break;
    }
}

// Strobe callback

VOID CALLBACK StrobeCallback(PVOID lpParameter, BOOL TimerFired)
{
    InvalidateRgn(strobe.hwnd, NULL, TRUE);
    UpdateMeter(&meter);

    // HWND strobe = *(HWND *)lpParameter;

    // RECT rect;
    // HDC hdc;

    // GetClientRect(strobe, &rect);
    // hdc = GetDC(strobe);

    // DrawStrobe(hdc, rect);
    // ReleaseDC(strobe, hdc);
}

// Update meter

VOID UpdateMeter(METERP meter)
{
    static float mc;

    mc = ((mc * 7.0) + meter->c) / 8.0;

    int value = round(mc * MAX_METER) + REF_METER;

    SendMessage(meter->slider.hwnd, TBM_SETPOS, TRUE, value);
}

// Draw scope

BOOL DrawScope(HDC hdc, RECT rect)
{
    static HBITMAP bitmap;
    static HDC hbdc;

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate bitmap dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create bitmap

    if (bitmap == NULL)
    {
	bitmap = CreateCompatibleBitmap(hdc, width, height);

	// Create DC

	hbdc = CreateCompatibleDC(hdc);
	SelectObject(hbdc, bitmap);
	SelectObject(hbdc, GetStockObject(BLACK_BRUSH));
	SelectObject(hbdc, GetStockObject(DC_PEN));
    }

    // Erase background

    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, GetStockObject(BLACK_BRUSH));

    // Dark green graticute

    SetDCPenColor(hbdc, RGB(0, 128, 0));

    // Draw graticule

    for (int i = 4; i < width; i += 5)
    {
	MoveToEx(hbdc, i, 0, NULL);
	LineTo(hbdc, i, height);
    }

    for (int i = 4; i < height; i += 5)
    {
	MoveToEx(hbdc, 0, i, NULL);
	LineTo(hbdc, width, i);
    }

    // Don't attempt the trace until there's a buffer

    if (scope.data != NULL)
    {
	// Initialise sync

	int maxdx = 0;
	int dx = 0;
	int n = 0;

	for (int i = 1; i < width; i++)
	{
	    dx = scope.data[i] - scope.data[i - 1];
	    if (maxdx > dx)
	    {
		maxdx = dx;
		n = i;
	    }

	    if (maxdx < 0 && dx > 0)
		break;
	}

	// Green pen for scope trace

	SetDCPenColor(hbdc, RGB(0, 255, 0));

	// Move the origin

	SetViewportOrgEx(hbdc, 0, height / 2, NULL);

	static int max;

	if (max < 4096)
	    max = 4096;

	int yscale = max / (height / 2);

	max = 0;

	// Draw the trace

	MoveToEx(hbdc, 0, 0, NULL);
	for (int i = 0; i < width; i++)
	{
	    if (max < abs(scope.data[n + i]))
		max =  abs(scope.data[n+i]);

	    int y = scope.data[n + i] / yscale;
	    LineTo(hbdc, i, y);
	}
    }

    // Move the origin back

    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Copy the bitmap

    BitBlt(hdc, rect.left, rect.top, width, height,
    	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw spectrum

BOOL DrawSpectrum(HDC hdc, RECT rect)
{
    static HBITMAP bitmap;
    static HDC hbdc;

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate bitmap dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create bitmap

    if (bitmap == NULL)
    {
	bitmap = CreateCompatibleBitmap(hdc, width, height);

	// Create DC

	hbdc = CreateCompatibleDC(hdc);
	SelectObject(hbdc, bitmap);
	SelectObject(hbdc, GetStockObject(BLACK_BRUSH));
	SelectObject(hbdc, GetStockObject(DC_PEN));
    }

    // Erase background

    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, GetStockObject(BLACK_BRUSH));

    // Dark green graticule

    SetDCPenColor(hbdc, RGB(0, 128, 0));

    // Draw graticule

    for (int i = 4; i < width; i += 5)
    {
	MoveToEx(hbdc, i, 0, NULL);
	LineTo(hbdc, i, height);
    }

    for (int i = 4; i < height; i += 5)
    {
	MoveToEx(hbdc, 0, i, NULL);
	LineTo(hbdc, width, i);
    }

    // Don't attempt the trace until there's a buffer

    if (spectrum.data != NULL)
    {
	// Green pen for spectrum trace

	SetDCPenColor(hbdc, RGB(0, 255, 0));

	// Move the origin

	SetViewportOrgEx(hbdc, 0, height - 1, NULL);

	static float max;

	if (max < 1.0)
	    max = 1.0;

	// Calculate the scaling

	float xscale = (float)spectrum.length / width;
	float yscale = (float)height / max;

	max = 0.0;

	// Draw the spectrum

	for (int i = 0; i < width; i++)
	{
	    float value = 0.0;

	    if (spectrum.zoom)
	    {
	    	if (spectrum.k + i - (width / 2) >= 0 &&
	    	    spectrum.k + i - (width / 2) < spectrum.length)
	    	{
	    	    value = spectrum.data[spectrum.k + i - (width / 2)];

	    	    if (max < value)
	    		max = value;

	    	    int y = value * yscale;

	    	    MoveToEx(hbdc, i, 0, NULL);
	    	    LineTo(hbdc, i, -y);
	    	}
	    }

	    else
	    {
	    	for (int j = 0; j < xscale; j++)
	    	{
	    	    int x = i * xscale + j;

	    	    if (value < spectrum.data[x])
	    		value = spectrum.data[x];
	    	}

	    	if (max < value)
	    	    max = value;

	    	int y = value * yscale;

	    	MoveToEx(hbdc, i, 0, NULL);
	    	LineTo(hbdc, i, -y);
	    }
	}
    }

    // Move the origin back

    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Copy the bitmap

    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw display

BOOL DrawDisplay(HDC hdc, RECT rect)
{
    enum
    {LARGE_HEIGHT  = 42,
     MEDIUM_HEIGHT = 28};

    static char *notes[] =
	{"A", "Bb", "B", "C", "C#", "D",
	 "Eb", "E", "F", "F#", "G", "Ab"};

    // Bold font

    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_BOLD,
	 FALSE, FALSE, FALSE,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 DEFAULT_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 ""};

    static HFONT large;
    static HFONT medium;

    if (large == NULL)
    {
	lf.lfHeight = LARGE_HEIGHT;
	large = CreateFontIndirect(&lf);

	lf.lfHeight = MEDIUM_HEIGHT;
	medium = CreateFontIndirect(&lf);
    }

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Move the viewport origin

    SetViewportOrgEx(hdc, rect.left, rect.top, NULL);

    // Client coordinates

    int y = 0;
    int width = rect.right - rect.left;

    static char s[64];

    // Select font

    SelectObject(hdc, large);

    sprintf(s, "%4s%d  ", notes[display.n % LENGTH(notes)], display.n / 12); 
    TextOut(hdc, 8, y, s, strlen(s));

    sprintf(s, "%+6.2lf¢  ", display.c * 100.0);
    TextOut(hdc, width / 2, y, s, strlen(s));

    y += LARGE_HEIGHT;

    SelectObject(hdc, medium);

    sprintf(s, "%9.2lfHz  ", display.ff);
    TextOut(hdc, 8, y, s, strlen(s));

    sprintf(s, "%9.2lfHz  ", display.f);
    TextOut(hdc, width / 2, y, s, strlen(s));

    y += MEDIUM_HEIGHT;

    sprintf(s, "%9.2lfHz  ", (audio.reference == 0)?
	    A5_REFNCE: audio.reference);
    TextOut(hdc, 8, y, s, strlen(s));

    sprintf(s, "%+8.2lfHz  ", display.e);
    TextOut(hdc, width / 2, y, s, strlen(s));

    return TRUE;
}

// Draw meter

BOOL DrawMeter(HDC hdc, RECT rect)
{
    static HFONT font;

    // Plain vanilla font

    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_NORMAL,
	 FALSE, FALSE, FALSE,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 DEFAULT_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 ""};

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate bitmap dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Move origin

    SetViewportOrgEx(hdc, rect.left + width / 2, rect.top, NULL);

    // Select font

    if (font == NULL)
    {
	lf.lfHeight = 16;
	font = CreateFontIndirect(&lf);
    }

    SelectObject(hdc, font);
    SetTextAlign(hdc, TA_CENTER);

    // Draw the meter scale

    for (int i = 0; i < 6; i++)
    {
	int x = width / 11 * i;
	char s[16];

	sprintf(s, "%d", i * 10);
	TextOut(hdc, x + 1, 0, s, strlen(s));
	TextOut(hdc, -x + 1, 0, s, strlen(s));

	MoveToEx(hdc, x, 14, NULL);
	LineTo(hdc, x, 20);
	MoveToEx(hdc, -x, 14, NULL);
	LineTo(hdc, -x, 20);

	for (int j = 1; j < 5; j++)
	{
	    if (i < 5)
	    {
		MoveToEx(hdc, x + j * width / 55, 16, NULL);
		LineTo(hdc, x + j * width / 55, 20);
	    }

	    MoveToEx(hdc, -x + j * width / 55, 16, NULL);
	    LineTo(hdc, -x + j * width / 55, 20);
	}
    }

    return TRUE;
}

// Draw strobe

BOOL DrawStrobe(HDC hdc, RECT rect)
{
    static float mc = 0.0;
    static float mx = 0.0;

    static HDC hbdc;

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    if (hbdc == NULL)
    {
	hbdc = CreateCompatibleDC(hdc);
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);
	SelectObject(hbdc, GetStockObject(BLACK_BRUSH));
    }

    // Erase background

    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, GetStockObject(WHITE_BRUSH));

    if (strobe.enable)
    {
	mc = ((7.0 * mc) + strobe.c) / 8.0;
	mx += mc * 50.0;

	if (mx > 160.0)
	    mx = 0.0;

	if (mx < 0.0)
	    mx = 160.0;

	int rx = round(mx - 160.0);

	for (int x = rx % 20; x < width; x += 20)
	    Rectangle(hbdc, x, 0, x + 10, 10);

	for (int x = rx % 40; x < width; x += 40)
	    Rectangle(hbdc, x, 10, x + 20, 20);

	for (int x = rx % 80; x < width; x += 80)
	    Rectangle(hbdc, x, 20, x + 40, 30);

	for (int x = rx % 160; x < width; x += 160)
	    Rectangle(hbdc, x, 30, x + 80, 40);
    }

    BitBlt(hdc, rect.left, rect.top, width, height, hbdc,
	   0, 0, SRCCOPY);

    return TRUE;
}

// Draw minus

BOOL DrawMinus(HDC hdc, RECT rect, UINT state)
{
    // Draw nice etched edge

    if (state & ODS_SELECTED)
	DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    else
	DrawEdge(hdc, &rect , EDGE_RAISED, BF_ADJUST | BF_RECT);

    Rectangle(hdc, rect.left + 2, (rect.bottom - rect.top) / 2 + rect.top - 1,
	      rect.right - 2, (rect.bottom - rect.top) / 2 + rect.top + 1);
}

// Draw plus

BOOL DrawPlus(HDC hdc, RECT rect, UINT state)
{
    // Draw nice etched edge

    if (state & ODS_SELECTED)
	DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    else
	DrawEdge(hdc, &rect , EDGE_RAISED, BF_ADJUST | BF_RECT);

    Rectangle(hdc, rect.left + 2, (rect.bottom - rect.top) / 2 + rect.top - 1,
	      rect.right - 2, (rect.bottom - rect.top) / 2 + rect.top + 1);
    Rectangle(hdc, (rect.right - rect.left) / 2 + rect.left - 1, rect.top + 2,
	      (rect.right - rect.left) / 2 + rect.left + 1, rect.bottom - 2);
}

// Plus clicked

BOOL PlusClicked(WPARAM wParam, LPARAM lParam)
{
    if (audio.reference == 0)
	return FALSE;

    int value = audio.reference * 10;

    switch(HIWORD(wParam))
    {
    case BN_CLICKED:
	SendMessage(adjust.hwnd, TBM_SETPOS, TRUE, ++value);
	audio.reference = value / 10.0;
	InvalidateRgn(display.hwnd, NULL, TRUE);
	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Minus clicked

BOOL MinusClicked(WPARAM wParam, LPARAM lParam)
{
    if (audio.reference == 0)
	return FALSE;

    int value = audio.reference * 10;

    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	SendMessage(adjust.hwnd, TBM_SETPOS, TRUE, --value);
	audio.reference = value / 10.0;
	InvalidateRgn(display.hwnd, NULL, TRUE);
	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Display clicked

BOOL DisplayClicked(WPARAM wParam, LPARAM lParam)
{

    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	audio.trigger = TRUE;
	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Spectrum clicked

BOOL SpectrumClicked(WPARAM wParam, LPARAM lParam)
{

    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	spectrum.zoom = !spectrum.zoom;
	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Strobe clicked

BOOL StrobeClicked(WPARAM wParam, LPARAM lParam)
{

    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	strobe.enable = !strobe.enable;
	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Change Volume

BOOL ChangeVolume(WPARAM wParam, LPARAM lParam)
{
    if (mixer.hmx == NULL)
	return FALSE;

    switch (LOWORD(wParam))
    {
	// Adjustments

    case TB_PAGEDOWN:
    case TB_PAGEUP:
	break;

    case TB_THUMBPOSITION:
    case TB_THUMBTRACK:
	break;

    default:
	return FALSE;
    }

    int value = SendMessage(volume.hwnd, TBM_GETPOS, 0, 0);

    mixer.pmxcdu->dwValue = ((mixer.pmxc->Bounds.dwMaximum -
			      mixer.pmxc->Bounds.dwMinimum) *
			     (MAX_VOL - value)) / (MAX_VOL - MIN_VOL);

    mixerSetControlDetails((HMIXEROBJ)mixer.hmx, mixer.pmxcd,
			   MIXER_SETCONTROLDETAILSF_VALUE);

    return TRUE;
}

// Change reference

BOOL ChangeReference(WPARAM wParam, LPARAM lParam)
{
    if (audio.reference == 0)
	return FALSE;

    switch (LOWORD(wParam))
    {
	// Adjustments

    case TB_PAGEDOWN:
    case TB_PAGEUP:
	break;

    case TB_THUMBPOSITION:
    case TB_THUMBTRACK:
	break;

    default:
	return FALSE;
    }

    int value = SendMessage(adjust.hwnd, TBM_GETPOS, 0, 0);
    audio.reference = value / 10.0;

    InvalidateRgn(display.hwnd, NULL, TRUE);

    return TRUE;
}

// Audio thread

DWORD WINAPI AudioThread(LPVOID lpParameter)
{
    // Create wave format structure

    static WAVEFORMATEX wf =
	{WAVE_FORMAT_PCM,
	 CHANNELS, SAMPLE_RATE, SAMPLE_RATE * BLOCK_ALIGN,
	 BLOCK_ALIGN, BITS_PER_SAMPLE, 0};

    // wf.wFormatTag = WAVE_FORMAT_PCM;
    // wf.nChannels = CHANNELS;
    // wf.nSamplesPerSec = SAMPLE_RATE;
    // wf.nAvgBytesPerSec = SAMPLE_RATE * BLOCK_ALIGN;
    // wf.nBlockAlign = BLOCK_ALIGN;
    // wf.wBitsPerSample = BITS_PER_SAMPLE;
    // wf.cbSize = 0;

    MMRESULT mmr;

    // Open a waveform audio input device

    mmr = waveInOpen(&audio.hwi, WAVE_MAPPER | WAVE_FORMAT_DIRECT, &wf,
		     (DWORD_PTR)audio.id,  (DWORD_PTR)NULL, CALLBACK_THREAD);
    if (mmr != MMSYSERR_NOERROR)
    {
	char text[64];

	waveInGetErrorText(mmr, text, sizeof(text));
	MessageBox(window.hwnd, text, "WaveInOpen", MB_OK | MB_ICONERROR);
	return mmr;
    }

    do
    {
	// Mixer structures

	static MIXERLINE mxl =
	    {sizeof(MIXERLINE)};

	static MIXERCONTROL mxc =
	    {sizeof(MIXERCONTROL)};

	static MIXERLINECONTROLS mxlc =
	    {sizeof(MIXERLINECONTROLS)};

	static MIXERCONTROLDETAILS mxcd =
	    {sizeof(MIXERCONTROLDETAILS)};

	static MIXERCONTROLDETAILS_UNSIGNED mxcdu;

	// Open a mixer device

	mmr = mixerOpen(&mixer.hmx, (UINT)audio.hwi, 0, 0,
			MIXER_OBJECTF_HWAVEIN);

	if (mmr != MMSYSERR_NOERROR)
	{
	    EnableWindow(volume.hwnd, FALSE);
	    break;
	}

	// Get mixer line info

	mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE;

	mixerGetLineInfo((HMIXEROBJ)mixer.hmx, &mxl,
			 MIXER_GETLINEINFOF_COMPONENTTYPE);

	if (mmr != MMSYSERR_NOERROR)
	{
	    EnableWindow(volume.hwnd, FALSE);
	    break;
	}

	mixer.pmxl = &mxl;

	// Get a volume control

	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(mxc);
	mxlc.pamxctrl = &mxc;

	mmr = mixerGetLineControls((HMIXEROBJ)mixer.hmx, &mxlc,
				   MIXER_GETLINECONTROLSF_ONEBYTYPE);

	if (mmr != MMSYSERR_NOERROR)
	{
	    EnableWindow(volume.hwnd, FALSE);
	    break;
	}

	// Got a volume control

	mixer.pmxc = &mxc;

	// Get the control details

	mxcd.dwControlID = mxc.dwControlID;
	mxcd.cChannels = MIXERCONTROL_CONTROLF_UNIFORM;
	mxcd.cbDetails = sizeof(mxcdu);
	mxcd.paDetails = &mxcdu;

	mmr = mixerGetControlDetails((HMIXEROBJ)mixer.hmx, &mxcd,
				     MIXER_GETCONTROLDETAILSF_VALUE);

	if (mmr != MMSYSERR_NOERROR)
	{
	    EnableWindow(volume.hwnd, FALSE);
	    break;
	}

	mixer.pmxcd = &mxcd;
	mixer.pmxcdu = &mxcdu;

	// Set the slider

	int value = MAX_VOL - (mxcdu.dwValue * (MAX_VOL - MIN_VOL) /
			       (mxc.Bounds.dwMaximum -
				mxc.Bounds.dwMinimum));

	SendMessage(volume.hwnd, TBM_SETPOS, TRUE, value);

    } while (FALSE);

    // Create the waveform audio input buffers and structures

    static short data[4][STEP];
    static WAVEHDR hdrs[4] =
	{{(LPSTR)data[0], sizeof(data[0]), 0, 0, 0, 0},
	 {(LPSTR)data[1], sizeof(data[1]), 0, 0, 0, 0},
	 {(LPSTR)data[2], sizeof(data[2]), 0, 0, 0, 0},
	 {(LPSTR)data[3], sizeof(data[3]), 0, 0, 0, 0}};

    for (int i = 0; i < LENGTH(hdrs); i++)
    {
	// hdrs[i].lpData = data[i];
	// hdrs[i].dwBufferLength = sizeof(data[i];
	// hdrs[i].dwBytesRecorded = 0;
	// hdrs[i].dwUser = 0;
	// hdrs[i].dwFlags = 0;
	// hdrs[i].dwLoops = 0;

	// Prepare a waveform audio input header

	mmr = waveInPrepareHeader(audio.hwi, &hdrs[i], sizeof(WAVEHDR));
	if (mmr != MMSYSERR_NOERROR)
	{
	    char text[64];

	    waveInGetErrorText(mmr, text, sizeof(text));
	    MessageBox(window.hwnd, text, "WaveInPrepareHeader",
		       MB_OK | MB_ICONERROR);
	    return mmr;
	}

	// Add a waveform audio input buffer

	mmr = waveInAddBuffer(audio.hwi, &hdrs[i], sizeof(WAVEHDR));
	if (mmr != MMSYSERR_NOERROR)
	{
	    char text[64];

	    waveInGetErrorText(mmr, text, sizeof(text));
	    MessageBox(window.hwnd, text, "WaveInAddBuffer",
		       MB_OK | MB_ICONERROR);
	    return mmr;
	}
    }

    // Start the waveform audio input

    mmr = waveInStart(audio.hwi);
    if (mmr != MMSYSERR_NOERROR)
    {
	char text[64];

	waveInGetErrorText(mmr, text, sizeof(text));
	MessageBox(window.hwnd, text, "WaveInStart", MB_OK | MB_ICONERROR);
	return mmr;
    }

    // Get the time

    audio.then = timeGetTime();

    // Start the timer

    CreateTimerQueueTimer(&audio.timer, NULL,
			  (WAITORTIMERCALLBACK)TimerCallback,
			  &audio.hwi, TIMER_DELAY, TIMER_DELAY,
			  WT_EXECUTEDEFAULT);

    // Set up reference value;

    audio.reference = A5_REFNCE;

    // Create a message loop for processing thread messages

    MSG msg;
    BOOL flag;

    while ((flag = GetMessage(&msg, (HWND)-1, 0, 0)) != 0)
    {
	if (flag == -1)
	    break;

	// Process messages

	switch (msg.message)
	{
	    // Audio input opened

	case MM_WIM_OPEN:
	    break;

	    // Audio input data

	case MM_WIM_DATA:
	    WaveInData(msg.wParam, msg.lParam);
	    break;

	    // Audio input closed

	case MM_WIM_CLOSE:
	    break;
	}
    }

    return msg.wParam;
}

void WaveInData(WPARAM wParam, LPARAM lParam)
{
    // Create buffers for processing the audio data

    static short buffer[SAMPLES];

    static COMPLEX x[SAMPLES];

    static double xa[STEP];
    static double xm[STEP];
    static double xd[STEP];
    static double xp[STEP];
    static double xf[STEP];

    static short *data = buffer + SAMPLES - STEP;

    static double fps = (double)SAMPLE_RATE / (double)SAMPLES;
    static double expect = 2.0 * M_PI * (double)STEP / (double)SAMPLES;

    // initialise data structs

    if (scope.data == NULL)
    {
	scope.data = data;
	scope.length = SAMPLES;

	spectrum.data = xa;
	spectrum.length = STEP;
    }

    // Copy the input data

    memmove(buffer, buffer + STEP, (SAMPLES - STEP) * sizeof(short));
    memmove(buffer + (SAMPLES - STEP), ((WAVEHDR *)lParam)->lpData,
	    STEP * sizeof(short));

    // Give the buffer back

    waveInAddBuffer(audio.hwi, (WAVEHDR *)lParam, sizeof(WAVEHDR));

    static double dmax;

    if (dmax < 4096.0)
	dmax = 4096.0;

    double scale = dmax;
    dmax = 0.0;

    // Copy data to FFT input arrays

    for (int i = 0; i < LENGTH(buffer); i++)
    {
	if (dmax < abs(buffer[i]))
	    dmax = abs(buffer[i]);

	double window =
	    0.5 - 0.5 * cos(2.0 * M_PI *
			    i / SAMPLES);

	x[i].re = (double)buffer[i] / scale * window;
	x[i].im = 0.0;
    }

    // do FFT

    complexToComplex(FORWARD, SAMPLES, x);

    // do frequency calculation

    for (int i = 0; i < LENGTH(xa); i++)
    {
	double real = x[i].re;
	double imag = x[i].im;

	xa[i] = sqrt((real * real) + (imag * imag));

	// xm[i] = (xa[i] + (xm[i] * 19.0)) / 20.0;

	// if (xm[i] > xa[i])
	//     xm[i] = xa[i];

	// xd[i] = xa[i] - xm[i];

	// if (xm[i] > xa[i])
	//     xm[i] = xa[i];

	// xd[i] = xa[i] - xm[i];

	double p = atan2(imag, real);
	double dp = xp[i] - p;
	xp[i] = p;

	dp -= (double)i * expect;

	int qpd = dp / M_PI;

	if (qpd >= 0)
	    qpd += qpd & 1;

	else
	    qpd -= qpd & 1;

	dp -=  M_PI * (double)qpd;

	double df = OVERSAMPLE * dp / (2.0 * M_PI);

	xf[i] = i * fps + df * fps;
    }

    double max = 0.0;
    double f = 0.0;
    int k = 0;

    for (int i = 0; i < LENGTH(xa); i++)
    {
	if (max < xa[i])
	{
	    max = xa[i];
	    f = xf[i];
	    k = i;
	}

	// if ((max > MIN) && (xd[i] < MIN))
	//     break;
    }

    BOOL found = FALSE;
    double ff = 0.0;
    double e = 0.0;
    double c = 0.0;
    int n = 0;

    if (max > MIN)
    {
	double cf =
	    -12.0 * (log(audio.reference / f) / log(2.0));

	n = round(cf) + A5_OFFSET;

	if (n < 0)
	    n = 0;

	if (n > C8_OFFSET)
	    n = C8_OFFSET;

	ff = audio.reference * pow(2.0, (n - A5_OFFSET) / 12.0);

	c = -12.0 * (log(ff / f) / log(2.0));

	e = f - ff;

	if (abs(c) < 0.5)
	    found = TRUE;
    }

    if (found)
    {
	// Update scope window

	InvalidateRgn(scope.hwnd, NULL, TRUE);

	// Update spectrum window

	InvalidateRgn(spectrum.hwnd, NULL, TRUE);

	// Update the display struct

	display.f = f;
	display.ff = ff;
	display.c = c;
	display.e = e;
	display.n = n;

	spectrum.k = k;

	// Update display

	InvalidateRgn(display.hwnd, NULL, TRUE);

	// Update meter

	meter.c = c;

	// Update strobe

	strobe.c = c;

	// Update log

	static char text[128];

	if (audio.trigger)
	{
	    for (int i = k - 2; i < k + 3; i++)
	    {
		sprintf(text, "%d, %lf, %lf, %lf, %lf\r\n",
			i, fps * i, xf[i], xp[i], xa[i]);
		SendMessage(edit.hwnd, EM_REPLACESEL, FALSE, (LPARAM)text);
	    }

	    audio.trigger = FALSE;
	}
    }
}

// Timer callback

VOID CALLBACK TimerCallback(PVOID lpParameter, BOOL TimerFired)
{
    static MMTIME mmt =
	{TIME_SAMPLES};

    static long count = 0;
    static double sample = 11025.0;
    static double correction = 1.0;

    // HWAVEIN hwi = *(HWAVEIN *)lpParameter;

    DWORD now = timeGetTime();
    waveInGetPosition(audio.hwi, &mmt, sizeof(mmt));

    count++;

    double elapsed = (double)(now - audio.then) / 1000.0;
    double rate = (double)(mmt.u.sample - audio.sample) / elapsed;

    audio.then = now;
    audio.sample = mmt.u.sample;

    sample = ((sample * 9.0) + rate) / 10.0;
    correction = ((correction * 9.0) +
		  (SAMPLE_RATE / sample)) / 10.0;

    static char text[64];

    // if (count <= 120)
    // {
    // 	sprintf(text, "%ld, %lf, %ld, %lf, %lf, %lf\r\n",
    // 		count, elapsed, mmt.u.sample, rate, sample, correction);
    // 	SendMessage(edit.hwnd, EM_REPLACESEL, FALSE, (LPARAM)text);
    // }

    // if (count >= 30)
    // {
    // 	sprintf(text, " %11.5lf\t\t%11.9lf ", sample, correction);
    // 	SendMessage(status.hwnd, SB_SETTEXT, 0, (LPARAM)text);
    // }
}

// FFT

void complexToComplex(int sign, int n, COMPLEXP a)
{
    double scale = sqrt(1.0 / n);

    for (int i = 0, j = 0; i < n; i++)
    {
	if (j >= i)
	{
	    double tr = a[j].re * scale;
	    double ti = a[j].im * scale;

	    a[j].re = a[i].re * scale;
	    a[j].im = a[i].im * scale;

	    a[i].re = tr;
	    a[i].im = ti;
	}

	int m = n / 2;
	while (m >= 1 && j >= m)
	{
	    j -= m;
	    m /= 2;
	}
	j += m;
    }
    
    for (int mmax = 1, istep = 2 * mmax; mmax < n;
	 mmax = istep, istep = 2 * mmax)
    {
	double delta = (sign * M_PI / mmax);
	for (int m = 0; m < mmax; m++)
	{
	    double w = m * delta;
	    double wr = cos(w);
	    double wi = sin(w);

	    for (int i = m; i < n; i += istep)
	    {
		int j = i + mmax;
		double tr = wr * a[j].re - wi * a[j].im;
		double ti = wr * a[j].im + wi * a[j].re;
		a[j].re = a[i].re - tr;
		a[j].im = a[i].im - ti;
		a[i].re += tr;
		a[i].im += ti;
	    }
	}
    }
}
