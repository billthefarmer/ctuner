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

#define _WIN32_IE    0x0501
#define _WIN32_WINNT 0x0500

#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>

// Macros

#define LENGTH(a) (sizeof(a) / sizeof(a[0]))

#define WCLASS "MainWClass"
#define PCLASS "PopupClass"

#define OCTAVE 12
#define MIN   0.5

// Global handle

HINSTANCE hInst;

// Tool ids

enum
    {SCOPE_ID = 101,
     SPECTRUM_ID,
     DISPLAY_ID,
     RESIZE_ID,
     STROBE_ID,
     METER_ID,
     PLUS_ID,
     MINUS_ID,
     VOLUME_ID,
     STATUS_ID,
     SLIDER_ID,
     QUIT_ID,
     ZOOM_ID,
     TEXT_ID,
     EDIT_ID,
     SAVE_ID,
     DONE_ID,
     LOCK_ID,
     FILTER_ID,
     UPDOWN_ID,
     ENABLE_ID,
     MULTIPLE_ID,
     OPTIONS_ID,
     REFERENCE_ID};

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
     RANGE = SAMPLES * 5 / 8,
     STEP = SAMPLES / OVERSAMPLE};

// Tuner reference values

enum
    {A5_REFNCE = 440,
     A5_OFFSET = 60};

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
    {STROBE_DELAY = 100};

// Window size

enum
    {WIDTH  = 320,
     HEIGHT = 396};

// Global data

typedef struct
{
    double r;
    double i;
} complex;

typedef struct
{
    HWND hwnd;
    BOOL zoom;
    RECT rwnd;
    RECT rclt;
} WINDOW, *WINDOWP;

WINDOW window;

typedef struct
{
    HWND hwnd;
} TOOL, *TOOLP;

TOOL status;

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
    float f;
    float r;
    float x[2];
    int count;
    double *data;
    float *values;
} SPECTRUM, *SPECTRUMP;

SPECTRUM spectrum;

typedef struct
{
    HWND hwnd;
    HANDLE timer;
    BOOL multiple;
    BOOL lock;
    double f;
    double fr;
    double c;
    int n;
    int count;
    double *maxima;
} DISPLAY, *DISPLAYP;

DISPLAY display;

TOOL adjust;
TOOL options;
TOOL multiple;
TOOL group;
TOOL zoom;
TOOL text;
TOOL edit;
TOOL lock;
TOOL resize;
TOOL filter;
TOOL updown;
TOOL enable;

struct
{
    TOOL plus;
    TOOL minus;
    TOOL options;
    TOOL save;
    TOOL done;
    TOOL quit;
} button;

struct
{
    TOOL reference;
    TOOL sample;
} legend;

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
} STROBE, STROBEP;

STROBE strobe;

typedef struct
{
    DWORD id;
    BOOL filter;
    HWAVEIN hwi;
    HANDLE thread;
    double correction;
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
LRESULT CALLBACK PopupProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ResizeProc(HWND, LPARAM);
BOOL RegisterMainClass(HINSTANCE);
VOID GetStatus(VOID);
BOOL ResizeWindow(WPARAM, LPARAM);
BOOL DrawItem(WPARAM, LPARAM);
BOOL DrawStrobe(HDC, RECT);
BOOL DrawScope(HDC, RECT);
BOOL DrawSpectrum(HDC, RECT);
BOOL DrawDisplay(HDC, RECT);
BOOL DrawMeter(HDC, RECT);
BOOL DrawMinus(HDC, RECT, UINT);
BOOL DrawPlus(HDC, RECT, UINT);
BOOL DisplayContextMenu(HWND, POINTS);
BOOL DisplayOptions(WPARAM, LPARAM);
BOOL DisplayOptionsMenu(HWND, POINTS);
BOOL DisplayClicked(WPARAM, LPARAM);
BOOL SpectrumClicked(WPARAM, LPARAM);
BOOL StrobeClicked(WPARAM, LPARAM);
BOOL FilterClicked(WPARAM, LPARAM);
BOOL ResizeClicked(WPARAM, LPARAM);
BOOL MinusClicked(WPARAM, LPARAM);
BOOL MinusPressed(WPARAM, LPARAM);
BOOL ScopeClicked(WPARAM, LPARAM);
BOOL LockClicked(WPARAM, LPARAM);
BOOL PlusClicked(WPARAM, LPARAM);
BOOL PlusPressed(WPARAM, LPARAM);
BOOL ZoomClicked(WPARAM, LPARAM);
BOOL MultipleClicked(WPARAM, LPARAM);
BOOL EnableClicked(WPARAM, LPARAM);
BOOL EditChange(WPARAM, LPARAM);
BOOL ChangeVolume(WPARAM, LPARAM);
BOOL CharPressed(WPARAM, LPARAM);
BOOL CopyDisplay(WPARAM, LPARAM);
BOOL ChangeCorrection(WPARAM, LPARAM);
BOOL ChangeReference(WPARAM, LPARAM);
BOOL SaveCorrection(WPARAM, LPARAM);
DWORD WINAPI AudioThread(LPVOID);
VOID WaveInData(WPARAM, LPARAM);
VOID UpdateMeter(METERP);
VOID CALLBACK DisplayCallback(PVOID, BOOL);
VOID CALLBACK TimerCallback(PVOID, BOOL);
VOID fftr(complex[], int);

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
	if (!RegisterMainClass(hInstance))
	    return FALSE;

    // Save the application-instance handle.

    hInst = hInstance;

    // Get saved status

    GetStatus();

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

BOOL RegisterMainClass(HINSTANCE hInst)
{
    // Fill in the window class structure with parameters
    // that describe the main window.

    WNDCLASS wc = 
	{CS_HREDRAW | CS_VREDRAW,
	 MainWndProc, 0, 0, hInst,
	 LoadIcon(hInst, "Tuner"),
	 LoadCursor(NULL, IDC_ARROW),
	 GetSysColorBrush(COLOR_WINDOW),
	 NULL, WCLASS};

    // Register the window class.

    return RegisterClass(&wc);
}

void GetStatus()
{
    HKEY hkey;
    LONG error;
    DWORD value;
    int size = sizeof(value);

    // Initial values

    strobe.enable = TRUE;
    audio.filter = TRUE;

    // Open key

    error = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\CTuner", 0,
			 KEY_READ, &hkey);

    if (error == ERROR_SUCCESS)
    {
	// Spectrum zoom

	error = RegQueryValueEx(hkey, "Zoom", NULL, NULL,
				(LPBYTE)&value, (LPDWORD)&size);
	// Update value

	if (error == ERROR_SUCCESS)
	    spectrum.zoom = value;

	// Strobe enable

	error = RegQueryValueEx(hkey, "Strobe", NULL, NULL,
				(LPBYTE)&value,(LPDWORD)&size);
	// Update value

	if (error == ERROR_SUCCESS)
	    strobe.enable = value;

	// Filter

	error = RegQueryValueEx(hkey, "Filter", NULL, NULL,
				(LPBYTE)&value,(LPDWORD)&size);
	// Update value

	if (error == ERROR_SUCCESS)
	    audio.filter = value;

	// Reference

	error = RegQueryValueEx(hkey, "Reference", NULL, NULL,
				(LPBYTE)&value,(LPDWORD)&size);
	// Update value

	if (error == ERROR_SUCCESS)
	    audio.reference = value / 10.0;

	// Close key

	RegCloseKey(hkey);
    }
}

// Main window procedure

LRESULT CALLBACK MainWndProc(HWND hWnd,
			     UINT uMsg,
			     WPARAM wParam,
			     LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:

	// Get the window and client dimensions

	GetWindowRect(hWnd, &window.rwnd);
	GetClientRect(hWnd, &window.rclt);

	// Calculate desired window width and height

	int border = (window.rwnd.right - window.rwnd.left) - window.rclt.right;
	int width  = WIDTH + border;
	int height = HEIGHT;

	// Set new dimensions

	SetWindowPos(hWnd, NULL, 0, 0,
		     width, height,
		     SWP_NOMOVE | SWP_NOZORDER);

	// Get client dimensions

	GetWindowRect(hWnd, &window.rwnd);
	GetClientRect(hWnd, &window.rclt);

	width = window.rclt.right - window.rclt.left;
	height = window.rclt.bottom - window.rclt.top;

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
	tooltip.info.lpszText = "Scope, click to filter audio";

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
			 8, 88, width - 16, 102, hWnd,
			 (HMENU)DISPLAY_ID, hInst, NULL);

	// Add display to tooltip

	tooltip.info.uId = (UINT_PTR)display.hwnd;
	tooltip.info.lpszText = "Display, click to lock";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create strobe

	strobe.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 8, 198, width - 16, 44, hWnd,
			 (HMENU)STROBE_ID, hInst, NULL);

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
			 8, 250, width - 16, 52, hWnd,
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
			 12, 272, width - 23, 26, hWnd,
			 (HMENU)SLIDER_ID, hInst, NULL);

	SendMessage(meter.slider.hwnd, TBM_SETRANGE, TRUE,
		    MAKELONG(MIN_METER, MAX_METER));
	SendMessage(meter.slider.hwnd, TBM_SETPOS, TRUE, REF_METER);

	// Add slider to tooltip

	tooltip.info.uId = (UINT_PTR)meter.slider.hwnd;
	tooltip.info.lpszText = "Cents";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create options button

	button.options.hwnd =
	    CreateWindow(WC_BUTTON, "Options...",
			 WS_VISIBLE | WS_CHILD |
			 BS_PUSHBUTTON,
			 7, 310, 85, 26,
			 hWnd, (HMENU)OPTIONS_ID, hInst, NULL);

	// Create quit button

	button.quit.hwnd =
	    CreateWindow(WC_BUTTON, "Quit",
			 WS_VISIBLE | WS_CHILD |
			 BS_PUSHBUTTON,
			 226, 310, 85, 26,
			 hWnd, (HMENU)QUIT_ID, hInst, NULL);

	// Create status bar

	status.hwnd =
	    CreateWindow(STATUSCLASSNAME,
			 " Sample rate: 11025.0\t"
			 "\tCorrection: 1.00000 ",
			 WS_VISIBLE | WS_CHILD,
			 0, 0, 0, 0, hWnd,
			 (HMENU)STATUS_ID, hInst, NULL);

	// Start audio thread

	audio.thread = CreateThread(NULL, 0, AudioThread, hWnd, 0, &audio.id);

	// Start display timer

	CreateTimerQueueTimer(&display.timer, NULL,
			      (WAITORTIMERCALLBACK)DisplayCallback,
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

	// Set the focus back to the window by clicking

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
	SetFocus(hWnd);
	break;

    case WM_RBUTTONDOWN:
	DisplayContextMenu(hWnd, MAKEPOINTS(lParam));
	break;

	// Buttons

    case WM_COMMAND:
	switch (LOWORD(wParam))
	{
	case SCOPE_ID:
	    ScopeClicked(wParam, lParam);
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

	case ZOOM_ID:
	    ZoomClicked(wParam, lParam);

	    // Set the focus back to the window

	    SetFocus(hWnd);
	    break;

	    // Enable control

	case ENABLE_ID:
	    EnableClicked(wParam, lParam);

	    // Set the focus back to the window

	    SetFocus(hWnd);
	    break;

	    // Filter control

	case FILTER_ID:
	    FilterClicked(wParam, lParam);

	    // Set the focus back to the window

	    SetFocus(hWnd);
	    break;

	    // Lock control

	case LOCK_ID:
	    LockClicked(wParam, lParam);

	    // Set the focus back to the window

	    SetFocus(hWnd);
	    break;

	    // Resize control

	case RESIZE_ID:
	    ResizeClicked(wParam, lParam);
	    break;

	case MULTIPLE_ID:
	    MultipleClicked(wParam, lParam);
	    break;

	    // Options

	case OPTIONS_ID:
	    DisplayOptions(wParam, lParam);
	    break;

	    // Quit

	case QUIT_ID:
	    PostQuitMessage(0);
	    break;
	}

	// Set the focus back to the window

	SetFocus(hWnd);
	break;

	// Char pressed

    case WM_CHAR:
	CharPressed(wParam, lParam);
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

// Resize window

BOOL ResizeWindow(WPARAM wParam, LPARAM lParam)
{
    int width, height;

    GetWindowRect(window.hwnd, &window.rwnd);

    int border = (window.rwnd.right - window.rwnd.left) - window.rclt.right;

    if (window.zoom)
    {
	// Calculate desired window width and height

	width  = WIDTH * 2 + border;
	height = HEIGHT * 2 - 50;
    }

    else
    {
	// Calculate desired window width and height

	width  = WIDTH + border;
	height = HEIGHT;
    }

    // Set new dimensions

    SetWindowPos(window.hwnd, NULL, 0, 0,
		 width, height, SWP_NOMOVE | SWP_NOZORDER);

    // Get client dimensions

    GetClientRect(window.hwnd, &window.rclt);

    EnumChildWindows(window.hwnd, ResizeProc, window.zoom);
}

// Resize procedure

BOOL CALLBACK ResizeProc(HWND hwnd, LPARAM lParam)
{
    union
    {
	RECT r;
	POINT p[2];
    } rect;

    GetWindowRect(hwnd, &rect.r);
    ScreenToClient(window.hwnd, &rect.p[0]);
    ScreenToClient(window.hwnd, &rect.p[1]);

    int width = rect.r.right - rect.r.left;
    int height = rect.r.bottom - rect.r.top;

    if (lParam)
	MoveWindow(hwnd, rect.r.left * 2, rect.r.top * 2,
		     width * 2, height * 2, TRUE);

    else
	MoveWindow(hwnd, rect.r.left / 2, rect.r.top / 2,
		     width / 2, height / 2, TRUE);

    return TRUE;
}

// Draw item

BOOL DrawItem(WPARAM wParam, LPARAM lParam)
{
    LPDRAWITEMSTRUCT lpdi = (LPDRAWITEMSTRUCT)lParam;
    UINT state = lpdi->itemState;
    RECT rect = lpdi->rcItem;
    HDC hdc = lpdi->hDC;

    SetGraphicsMode(hdc, GM_ADVANCED);

    switch (wParam)
    {
    case PLUS_ID:
    case MINUS_ID:
	break;

    default:
	if (window.zoom)
	{
	    XFORM xform =
		{2.0, 0.0, 0.0, 2.0, 0, 0};

	    SetWorldTransform(hdc, &xform);

	    rect.right /= 2;
	    rect.bottom /= 2;
	}
	break;
    }

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

// Display context menu

BOOL DisplayContextMenu(HWND hWnd, POINTS points)
{
    HMENU menu;
    POINT point;

    POINTSTOPOINT(point, points);
    ClientToScreen(hWnd, &point);

    menu = CreatePopupMenu();

    AppendMenu(menu, spectrum.zoom? MF_STRING | MF_CHECKED:
	       MF_STRING, ZOOM_ID, "Zoom spectrum");
    AppendMenu(menu, strobe.enable? MF_STRING | MF_CHECKED:
	       MF_STRING, ENABLE_ID, "Display strobe");
    AppendMenu(menu, audio.filter? MF_STRING | MF_CHECKED:
	       MF_STRING, FILTER_ID, "Audio filter");
    AppendMenu(menu, display.lock? MF_STRING | MF_CHECKED:
	       MF_STRING, LOCK_ID, "Lock display");
    AppendMenu(menu, window.zoom? MF_STRING | MF_CHECKED:
	       MF_STRING, RESIZE_ID, "Resize display");
    AppendMenu(menu, display.multiple? MF_STRING | MF_CHECKED:
	       MF_STRING, RESIZE_ID, "Multiple notes");
    AppendMenu(menu, MF_SEPARATOR, 0, 0);
    AppendMenu(menu, MF_STRING, OPTIONS_ID, "Options...");
    AppendMenu(menu, MF_SEPARATOR, 0, 0);
    AppendMenu(menu, MF_STRING, QUIT_ID, "Quit");

    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		   point.x, point.y,
		   0, hWnd, NULL);
}

// Display options

BOOL DisplayOptions(WPARAM wParam, LPARAM lParam)
{
    if (options.hwnd == NULL)
    {
	WNDCLASS wc = 
	    {CS_HREDRAW | CS_VREDRAW, PopupProc,
	     0, 0, hInst,
	     LoadIcon(NULL, IDI_WINLOGO),
	     LoadCursor(NULL, IDC_ARROW),
	     GetSysColorBrush(COLOR_WINDOW),
	     NULL, PCLASS};

	// Register the window class.

	RegisterClass(&wc);

	RECT rWnd;

	GetWindowRect(window.hwnd, &rWnd);

	options.hwnd =
	    CreateWindow(PCLASS, "Tuner Options",
			 WS_VISIBLE | WS_POPUP |
			 WS_CAPTION,
			 rWnd.right + 10, rWnd.top,
			 WIDTH, 340, window.hwnd,
			 (HMENU)NULL, hInst, NULL);
    }

    else
	ShowWindow(options.hwnd, TRUE);
}

// Popup Procedure

LRESULT CALLBACK PopupProc(HWND hWnd,
			   UINT uMsg,
			   WPARAM wParam,
			   LPARAM lParam)
{
    RECT cRect;
    static char s[64];

    GetClientRect(hWnd, &cRect);
    int width = cRect.right;

    switch (uMsg)
    {
    case WM_CREATE:

	// Create group box

	group.hwnd =
	    CreateWindow(WC_BUTTON, NULL,
			 WS_VISIBLE | WS_CHILD |
			 BS_GROUPBOX,
			 10, 2, width - 20, 148,
			 hWnd, NULL, hInst, NULL);

	// Create zoom tickbox

	zoom.hwnd =
	    CreateWindow(WC_BUTTON, "Zoom spectrum:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 20, 20, 124, 24,
			 hWnd, (HMENU)ZOOM_ID, hInst, NULL);

	SendMessage(zoom.hwnd, BM_SETCHECK,
		    spectrum.zoom? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip

	tooltip.info.uId = (UINT_PTR)zoom.hwnd;
	tooltip.info.lpszText = "Zoom spectrum, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create strobe enable tickbox

	enable.hwnd =
	    CreateWindow(WC_BUTTON, "Display strobe:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 width / 2 + 10, 20, 124, 24,
			 hWnd, (HMENU)ENABLE_ID, hInst, NULL);

	SendMessage(enable.hwnd, BM_SETCHECK,
		    strobe.enable? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip

	tooltip.info.uId = (UINT_PTR)enable.hwnd;
	tooltip.info.lpszText = "Display strobe, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create filter tickbox

	filter.hwnd =
	    CreateWindow(WC_BUTTON, "Audio filter:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 20, 54, 124, 24,
			 hWnd, (HMENU)FILTER_ID, hInst, NULL);

	SendMessage(filter.hwnd, BM_SETCHECK,
		    audio.filter? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip

	tooltip.info.uId = (UINT_PTR)filter.hwnd;
	tooltip.info.lpszText = "Audio filter, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create lock tickbox

	lock.hwnd =
	    CreateWindow(WC_BUTTON, "Lock display:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 width / 2 + 10, 54, 124, 24,
			 hWnd, (HMENU)LOCK_ID, hInst, NULL);

	SendMessage(lock.hwnd, BM_SETCHECK,
		    display.lock? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip

	tooltip.info.uId = (UINT_PTR)lock.hwnd;
	tooltip.info.lpszText = "Lock display, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create resize tickbox

	resize.hwnd =
	    CreateWindow(WC_BUTTON, "Resize display:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 20, 88, 124, 24,
			 hWnd, (HMENU)RESIZE_ID, hInst, NULL);

	SendMessage(resize.hwnd, BM_SETCHECK,
		    window.zoom? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip

	tooltip.info.uId = (UINT_PTR)resize.hwnd;
	tooltip.info.lpszText = "Resize display, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create multiple tickbox

	multiple.hwnd =
	    CreateWindow(WC_BUTTON, "Multiple notes:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 width / 2 + 10, 88, 124, 24,
			 hWnd, (HMENU)MULTIPLE_ID, hInst, NULL);

	SendMessage(multiple.hwnd, BM_SETCHECK,
		    display.multiple? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip

	tooltip.info.uId = (UINT_PTR)multiple.hwnd;
	tooltip.info.lpszText = "Display multiple notes, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create text

	sprintf(s, "Reference:  %6.2lfHz", audio.reference);

	legend.reference.hwnd =
	    CreateWindow(WC_STATIC, s,
			 WS_VISIBLE | WS_CHILD |
			 SS_LEFT,
			 20, 124, width - 40, 20,
			 hWnd, (HMENU)TEXT_ID, hInst, NULL);

	// Create minus button

	button.minus.hwnd =
	    CreateWindow(WC_BUTTON, NULL,
			 WS_VISIBLE | WS_CHILD |
			 BS_OWNERDRAW,
			 10, 160, 16, 16, hWnd,
			 (HMENU)MINUS_ID, hInst, NULL);

	// Add minus button to tooltip

	tooltip.info.uId = (UINT_PTR)button.minus.hwnd;
	tooltip.info.lpszText = "Reference frequency, "
	    "click to decrease";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create reference slider

	adjust.hwnd =
	    CreateWindow(TRACKBAR_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 TBS_HORZ | TBS_NOTICKS | TBS_TOP,
			 26, 156, width - 52, 24, hWnd,
			 (HMENU)REFERENCE_ID, hInst, NULL);

	SendMessage(adjust.hwnd, TBM_SETRANGE, TRUE,
		    MAKELONG(MIN_REF, MAX_REF));
	SendMessage(adjust.hwnd, TBM_SETPAGESIZE, 0, STEP_REF);
	SendMessage(adjust.hwnd, TBM_SETPOS, TRUE, (audio.reference == 0)?
		    REF_REF: audio.reference * 10);

	// Add adjust to tooltip

	tooltip.info.uId = (UINT_PTR)adjust.hwnd;
	tooltip.info.lpszText = "Reference frequency, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create plus button

	button.plus.hwnd =
	    CreateWindow(WC_BUTTON, NULL,
			 WS_VISIBLE | WS_CHILD |
			 BS_OWNERDRAW,
			 width - 26, 160, 16, 16, hWnd,
			 (HMENU)PLUS_ID, hInst, NULL);

	// Add plus button to tooltip

	tooltip.info.uId = (UINT_PTR)button.plus.hwnd;
	tooltip.info.lpszText = "Reference frequency, "
	    "click to increase";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create group box

	group.hwnd =
	    CreateWindow(WC_BUTTON, NULL,
			 WS_VISIBLE | WS_CHILD |
			 BS_GROUPBOX,
			 10, 180, width - 20, 124,
			 hWnd, NULL, hInst, NULL);

	// Create text

	text.hwnd =
	    CreateWindow(WC_STATIC, "Correction:",
			 WS_VISIBLE | WS_CHILD |
			 SS_LEFT,
			 20, 202, 76, 20,
			 hWnd, (HMENU)TEXT_ID, hInst, NULL);

	// Create edit control

	sprintf(s, "%6.5lf", audio.correction);

	edit.hwnd =
	    CreateWindow(WC_EDIT, s,
			 WS_VISIBLE | WS_CHILD |
			 WS_BORDER,
			 100, 200, 82, 20, hWnd,
			 (HMENU)EDIT_ID, hInst, NULL);

	// Add edit to tooltip

	tooltip.info.uId = (UINT_PTR)edit.hwnd;
	tooltip.info.lpszText = "Correction, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create up-down control

	updown.hwnd =
	    CreateWindow(UPDOWN_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 UDS_AUTOBUDDY | UDS_ALIGNRIGHT,
			 0, 0, 0, 0, hWnd,
			 (HMENU)UPDOWN_ID, hInst, NULL);

	SendMessage(updown.hwnd, UDM_SETRANGE32, 99000, 110000);
	SendMessage(updown.hwnd, UDM_SETPOS32, 0, audio.correction * 100000);

	// Add updown to tooltip

	tooltip.info.uId = (UINT_PTR)updown.hwnd;
	tooltip.info.lpszText = "Correction, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create save button

	button.save.hwnd =
	    CreateWindow(WC_BUTTON, "Save",
			 WS_VISIBLE | WS_CHILD |
			 BS_PUSHBUTTON,
			 209, 197, 85, 26,
			 hWnd, (HMENU)SAVE_ID, hInst, NULL);

	// Create text

	text.hwnd =
	    CreateWindow(WC_STATIC,
			 "Use correction if your sound card clock "
			 "is significantly inaccurate.",
			 WS_VISIBLE | WS_CHILD |
			 SS_LEFT,
			 20, 230, width - 40, 40,
			 hWnd, (HMENU)TEXT_ID, hInst, NULL);

	// Create text

	sprintf(s, "Sample rate:  %6.1lf",
		(double)SAMPLE_RATE / audio.correction);

	legend.sample.hwnd =
	    CreateWindow(WC_STATIC, s,
			 WS_VISIBLE | WS_CHILD |
			 SS_LEFT,
			 20, 272, 152, 20,
			 hWnd, (HMENU)TEXT_ID, hInst, NULL);

	// Create done button

	button.done.hwnd =
	    CreateWindow(WC_BUTTON, "Done",
			 WS_VISIBLE | WS_CHILD |
			 BS_PUSHBUTTON,
			 209, 267, 85, 26,
			 hWnd, (HMENU)DONE_ID, hInst, NULL);

	break;

	// Colour static text

    case WM_CTLCOLORSTATIC:
	return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
	break;

	// Draw item

    case WM_DRAWITEM:
	return DrawItem(wParam, lParam);
	break;

	// Updown control

    case WM_VSCROLL:
	ChangeCorrection(wParam, lParam);
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
	SetFocus(hWnd);
	break;

    case WM_RBUTTONDOWN:
	DisplayOptionsMenu(hWnd, MAKEPOINTS(lParam));
	break;

	// Buttons

    case WM_COMMAND:
	switch (LOWORD(wParam))
	{
	    // Zoom control

	case ZOOM_ID:
	    ZoomClicked(wParam, lParam);

	    // Set the focus back to the window

	    SetFocus(hWnd);
	    break;

	    // Enable control

	case ENABLE_ID:
	    EnableClicked(wParam, lParam);

	    // Set the focus back to the window

	    SetFocus(hWnd);
	    break;

	    // Filter control

	case FILTER_ID:
	    FilterClicked(wParam, lParam);

	    // Set the focus back to the window

	    SetFocus(hWnd);
	    break;

	    // Lock control

	case LOCK_ID:
	    LockClicked(wParam, lParam);

	    // Set the focus back to the window

	    SetFocus(hWnd);
	    break;

	    // Resize control

	case RESIZE_ID:
	    ResizeClicked(wParam, lParam);

	    // Set the focus back to the window

	    SetFocus(hWnd);
	    break;

	    // Multiple control

	case MULTIPLE_ID:
	    MultipleClicked(wParam, lParam);

	    // Set the focus back to the window

	    SetFocus(hWnd);
	    break;

	    // Plus button

	case PLUS_ID:
	    PlusClicked(wParam, lParam);
	    break;

	    // Minus button

	case MINUS_ID:
	    MinusClicked(wParam, lParam);
	    break;

	    // Updown control

	case EDIT_ID:
	    EditChange(wParam, lParam);

	    // Set the focus back to the window

	    // SetFocus(hWnd);
	    break;

	case DONE_ID:
	    ShowWindow(hWnd, FALSE);

	    // Set the focus back to the window

	    SetFocus(hWnd);
	    break;

	case SAVE_ID:
	    SaveCorrection(wParam, lParam);

	    // Set the focus back to the window

	    SetFocus(hWnd);
	    break;
	}
	break;

	// Char pressed

    case WM_CHAR:
	CharPressed(wParam, lParam);
	break;

	// Everything else

    default:
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

// Zoom clicked

BOOL ZoomClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	SpectrumClicked(wParam, lParam);
    }
}

// Enable clicked

BOOL EnableClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	StrobeClicked(wParam, lParam);
    }
}

// Filter clicked

BOOL FilterClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	audio.filter = !audio.filter;

	SendMessage(filter.hwnd, BM_SETCHECK,
		    audio.filter? BST_CHECKED: BST_UNCHECKED, 0);
	break;

    default:
	return FALSE;
    }

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	RegSetValueEx(hkey, "Filter", 0, REG_DWORD,
		      (LPBYTE)&audio.filter, sizeof(audio.filter));

	RegCloseKey(hkey);
    }

    else
    {
	static char s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }
}

// Lock clicked

BOOL LockClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	display.lock = !display.lock;

	SendMessage(lock.hwnd, BM_SETCHECK,
		    display.lock? BST_CHECKED: BST_UNCHECKED, 0);

	InvalidateRgn(display.hwnd, NULL, TRUE);
	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Resize clicked

BOOL ResizeClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	window.zoom = !window.zoom;

	SendMessage(resize.hwnd, BM_SETCHECK,
		    window.zoom? BST_CHECKED: BST_UNCHECKED, 0);

	ResizeWindow(wParam, lParam);
	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Multiple clicked

BOOL MultipleClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	display.multiple = !display.multiple;

	SendMessage(multiple.hwnd, BM_SETCHECK,
		    display.multiple? BST_CHECKED: BST_UNCHECKED, 0);
	InvalidateRgn(display.hwnd, NULL, TRUE);
	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Edit change

BOOL EditChange(WPARAM wParam, LPARAM lParam)
{
    static char s[64];

    switch (HIWORD(wParam))
    {
    case EN_KILLFOCUS:
	SendMessage(edit.hwnd, WM_GETTEXT, sizeof(s), (ULONG)s);
	audio.correction = atof(s);

	SendMessage(updown.hwnd, UDM_SETPOS32, 0, audio.correction * 100000);

	sprintf(s, "Sample rate:  %6.1lf",
		(double)SAMPLE_RATE / audio.correction);
	SendMessage(legend.sample.hwnd, WM_SETTEXT, 0, (LPARAM)s);

	sprintf(s, " Sample rate: %6.1lf\t\tCorrection: %6.5lf ",
		(double)SAMPLE_RATE / audio.correction, audio.correction);
	SendMessage(status.hwnd, SB_SETTEXT, 0, (LPARAM)s);
	break;
    }
}

// Change correction

BOOL ChangeCorrection(WPARAM wParam, LPARAM lParam)
{
    static char s[64];

    long value = SendMessage(updown.hwnd, UDM_GETPOS32, 0, 0);
    audio.correction = (double)value / 100000.0;

    sprintf(s, "%6.5lf", audio.correction);
    SendMessage(edit.hwnd, WM_SETTEXT, 0, (LPARAM)s);

    sprintf(s, "Sample rate:  %6.1lf",
	    (double)SAMPLE_RATE / audio.correction);
    SendMessage(legend.sample.hwnd, WM_SETTEXT, 0, (LPARAM)s);

    sprintf(s, " Sample rate: %6.1lf\t\tCorrection: %6.5lf ",
	    (double)SAMPLE_RATE / audio.correction, audio.correction);
    SendMessage(status.hwnd, SB_SETTEXT, 0, (LPARAM)s);
}

// Save Correction

BOOL SaveCorrection(WPARAM wParam, LPARAM lParam)
{
    HKEY hkey;
    LONG error;
    DWORD value;
    int size = sizeof(value);

    error = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\CTuner", 0,
			   NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
			   NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	value = audio.correction * 100000.0;

	RegSetValueEx(hkey, "Correction", 0, REG_DWORD,
		      (LPBYTE)&value, sizeof(value));

	RegCloseKey(hkey);
    }

    else
    {
	static char s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }
}

// Display options menu

BOOL DisplayOptionsMenu(HWND hWnd, POINTS points)
{
    HMENU menu;
    POINT point;

    POINTSTOPOINT(point, points);
    ClientToScreen(hWnd, &point);

    menu = CreatePopupMenu();
    AppendMenu(menu, MF_STRING, DONE_ID, "Done");

    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		   point.x, point.y,
		   0, hWnd, NULL);
}

// Char pressed

BOOL CharPressed(WPARAM w, LPARAM l)
{
    switch (w)
    {
    case 'C':
    case 'c':
    case 0x3:
	CopyDisplay(w, l);
	break;

    case 'F':
    case 'f':
	FilterClicked(w, l);
	break;

    case 'L':
    case 'l':
	LockClicked(w, l);
	break;

    case 'O':
    case 'o':
	DisplayOptions(w, l);
	break;

    case 'R':
    case 'r':
	ResizeClicked(w, l);
	break;

    case 'S':
    case 's':
	EnableClicked(w, l);
	break;

    case 'M':
    case 'm':
    case 'T':
    case 't':
	MultipleClicked(w, l);
	break;

    case 'Z':
    case 'z':
	ZoomClicked(w, l);
	break;

    case '+':
	PlusPressed(w, l);
	break;

    case '-':
	MinusPressed(w, l);
	break;
    }
}

// CopyDisplay

BOOL CopyDisplay(WPARAM w, LPARAM l)
{
    static char s[64];

    static char *notes[] =
	{"A", "Bb", "B", "C", "C#", "D",
	 "Eb", "E", "F", "F#", "G", "Ab"};

    if (!OpenClipboard(window.hwnd))
	return FALSE;

    EmptyClipboard(); 

    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, 1024);

    if (mem == NULL)
    {
	CloseClipboard();
	return FALSE;
    }

    LPTSTR text = GlobalLock(mem);

    if (display.multiple)
    {
	for (int i = 0; i < display.count; i++)
	{
	    double f = display.maxima[i];

	    double cf =
		-12.0 * (log(audio.reference / f) / log(2.0));

	    // Reference freq

	    double fr = audio.reference * pow(2.0, round(cf) / 12.0);

	    int n = round(cf) + A5_OFFSET;

	    if (n < 0)
		n = 0;

	    double c = -12.0 * (log(fr / f) / log(2.0));

	    // Ignore silly values

	    if (!isfinite(c))
		continue;

	    sprintf(s, "%s%d\t%+6.2lf\t%9.2lf\t%9.2lf\t%+8.2lf\r\n",
		    notes[n % LENGTH(notes)], n / 12,
		    c * 100.0, fr, f, f - fr);

	    if (i == 0)
		strcpy(text, s);

	    else
		strcat(text, s);
	}
    }

    else
    {
	sprintf(s, "%s%d\t%+6.2lf\t%9.2lf\t%9.2lf\t%+8.2lf\r\n",
		notes[display.n % LENGTH(notes)], display.n / 12,
		display.c * 100.0, display.fr, display.f,
		display.f - display.fr);

	strcpy(text, s);
    }

    GlobalUnlock(text);
    SetClipboardData(CF_TEXT, mem);
    CloseClipboard(); 
 
    return TRUE; 
}

// Display callback

VOID CALLBACK DisplayCallback(PVOID lpParameter, BOOL TimerFired)
{
    InvalidateRgn(strobe.hwnd, NULL, TRUE);
    UpdateMeter(&meter);
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

    // Create DC

    if (hbdc == NULL)
    {
	hbdc = CreateCompatibleDC(hdc);
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);
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

    if (scope.data == NULL)
    {
	// Copy the bitmap

	BitBlt(hdc, rect.left, rect.top, width, height,
	       hbdc, 0, 0, SRCCOPY);

	return TRUE;
    }

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
	    max = abs(scope.data[n + i]);

	int y = scope.data[n + i] / yscale;
	LineTo(hbdc, i, y);
    }

    // Move the origin back

    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Show F if filtered

    if (audio.filter)
    {
	MoveToEx(hbdc, 0, height - 7, NULL);
	LineTo(hbdc, 0, height);

	MoveToEx(hbdc, 0, height - 7, NULL);
	LineTo(hbdc, 4, height - 7);

	MoveToEx(hbdc, 0, height - 4, NULL);
	LineTo(hbdc, 3, height - 4);
    }

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

    // Create DC

    if (hbdc == NULL)
    {
	hbdc = CreateCompatibleDC(hdc);
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);
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

    if (spectrum.data == NULL)
    {
	// Copy the bitmap

	BitBlt(hdc, rect.left, rect.top, width, height,
	       hbdc, 0, 0, SRCCOPY);

	return TRUE;
    }

    // Green pen for spectrum trace

    SetDCPenColor(hbdc, RGB(0, 255, 0));

    // Move the origin

    SetViewportOrgEx(hbdc, 0, height - 1, NULL);

    static float max;

    if (max < 1.0)
	max = 1.0;

    // Calculate the scaling

    float yscale = (float)height / max;

    max = 0.0;

    // Draw the spectrum

    MoveToEx(hbdc, 0, 0, NULL);
    if (spectrum.zoom)
    {
	float xscale = ((float)width / (spectrum.r - spectrum.x[0])) / 2.0;

	for (int i = 0; i < ((float)width / xscale) + 1; i++)
	{
	    int n = spectrum.r + i - (width / (xscale * 2.0));

	    if (n >= 0 && n < spectrum.length)
	    {
		float value = spectrum.data[n];

		if (max < value)
		    max = value;

		int y = -value * yscale;
		int x = i * xscale;

		LineTo(hbdc, x, y);
	    }
	}

	MoveToEx(hbdc, width / 2, 0, NULL);
	LineTo(hbdc, width / 2, -height);

	// Yellow pen for frequency trace

	SetDCPenColor(hbdc, RGB(255, 255, 0));

	int x = (spectrum.f - spectrum.x[0]) * xscale;
	MoveToEx(hbdc, x, 0, NULL);
	LineTo(hbdc, x, -height);

	for (int i = 0; i < spectrum.count; i++)
	{
	    if (spectrum.values[i] > spectrum.x[0] &&
		spectrum.values[i] < spectrum.x[1])
	    {
		x = (spectrum.values[i] - spectrum.x[0]) * xscale;
		MoveToEx(hbdc, x, 0, NULL);
		LineTo(hbdc, x, -height);
	    }
	}
    }

    else
    {
	float xscale = (float)spectrum.length / (float)width;

	for (int x = 0; x < width; x++)
	{
	    float value = 0.0;

	    // Don't show DC component

	    if (x > 0)
	    {
		for (int j = 0; j < xscale; j++)
		{
		    int n = x * xscale + j;

		    if (value < spectrum.data[n])
			value = spectrum.data[n];
		}
	    }

	    if (max < value)
		max = value;

	    int y = -value * yscale;

	    LineTo(hbdc, x, y);
	}
    }

    // Move the origin back

    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Show T if multiple

    if (display.multiple)
    {	
	// Green pen for text

	SetDCPenColor(hbdc, RGB(0, 255, 0));

	MoveToEx(hbdc, 0, height - 7, NULL);
	LineTo(hbdc, 5, height - 7);

	MoveToEx(hbdc, 2, height - 7, NULL);
	LineTo(hbdc, 2, height);
    }

    // Copy the bitmap

    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw display

BOOL DrawDisplay(HDC hdc, RECT rect)
{
    static HBITMAP bitmap;
    static HFONT medium;
    static HFONT large;
    static HFONT font;
    static HDC hbdc;

    enum
    {FONT_HEIGHT   = 16,
     LARGE_HEIGHT  = 42,
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

    static char s[64];

    // Create fonts

    if (font == NULL)
    {
	lf.lfHeight = FONT_HEIGHT;
	font = CreateFontIndirect(&lf);

	lf.lfHeight = LARGE_HEIGHT;
	large = CreateFontIndirect(&lf);

	lf.lfHeight = MEDIUM_HEIGHT;
	medium = CreateFontIndirect(&lf);
    }

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create bitmap

    if (bitmap == NULL)
    {
	bitmap = CreateCompatibleBitmap(hdc, width, height);

	// Create DC

	hbdc = CreateCompatibleDC(hdc);
	SelectObject(hbdc, bitmap);
    }

    // Erase background

    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, GetStockObject(WHITE_BRUSH));

    // Select text colour

    if (display.lock)
	SetTextColor(hbdc, RGB(64, 128, 128));

    else
	SetTextColor(hbdc, RGB(0, 0, 0));

    if (display.multiple)
    {
	// Select font

	SelectObject(hbdc, font);

	if (display.count == 0)
	{
	    // Display note

	    sprintf(s, "%4s%d", notes[display.n % LENGTH(notes)],
		    display.n / 12);
	    TextOut(hbdc, 0, 0, s, strlen(s));

	    // Display cents

	    sprintf(s, "%+7.2lf¢", display.c * 100.0);
	    TextOut(hbdc, 36, 0, s, strlen(s));

	    // Display reference

	    sprintf(s, "%8.2lfHz", display.fr);
	    TextOut(hbdc, 90, 0, s, strlen(s));

	    // Display frequency

	    sprintf(s, "%8.2lfHz", display.f);
	    TextOut(hbdc, 162, 0, s, strlen(s));

	    // Display difference

	    sprintf(s, "%+7.2lfHz", display.f - display.fr);
	    TextOut(hbdc, 234, 0, s, strlen(s));
	}

	for (int i = 0; i < display.count; i++)
	{
	    double f = display.maxima[i];

	    double cf =
		-12.0 * (log(audio.reference / f) / log(2.0));

	    // Reference freq

	    double fr = audio.reference * pow(2.0, round(cf) / 12.0);

	    int n = round(cf) + A5_OFFSET;

	    if (n < 0)
		n = 0;

	    double c = -12.0 * (log(fr / f) / log(2.0));

	    // Ignore silly values

	    if (!isfinite(c))
		continue;

	    // Display note

	    sprintf(s, "%4s%d", notes[n % LENGTH(notes)], n / 12);
	    TextOut(hbdc, 0, i * FONT_HEIGHT, s, strlen(s));

	    // Display cents

	    sprintf(s, "%+7.2lf¢", c * 100.0);
	    TextOut(hbdc, 36, i * FONT_HEIGHT, s, strlen(s));

	    // Display reference

	    sprintf(s, "%8.2lfHz", fr);
	    TextOut(hbdc, 90, i * FONT_HEIGHT, s, strlen(s));

	    // Display frequency

	    sprintf(s, "%8.2lfHz", f);
	    TextOut(hbdc, 162, i * FONT_HEIGHT, s, strlen(s));

	    // Display difference

	    sprintf(s, "%+7.2lfHz", f - fr);
	    TextOut(hbdc, 234, i * FONT_HEIGHT, s, strlen(s));

	    if (i == 5)
		break;
	}
    }

    else
    {
	// Select large font

	SelectObject(hbdc, large);

	// Display coordinates

	int y = 0;

	// Display note

	sprintf(s, "%4s%d", notes[display.n % LENGTH(notes)],
		display.n / 12); 
	TextOut(hbdc, 8, y, s, strlen(s));

	// Display cents

	sprintf(s, "%+6.2lf¢", display.c * 100.0);
	TextOut(hbdc, width / 2, y, s, strlen(s));

	y = LARGE_HEIGHT;

	// Select medium font

	SelectObject(hbdc, medium);

	// Display reference frequency

	sprintf(s, "%9.2lfHz", display.fr);
	TextOut(hbdc, 8, y, s, strlen(s));

	// Display actual frequency

	sprintf(s, "%9.2lfHz", display.f);
	TextOut(hbdc, width / 2, y, s, strlen(s));

	y += MEDIUM_HEIGHT;

	// Display reference

	sprintf(s, "%9.2lfHz", (audio.reference == 0)?
		A5_REFNCE: audio.reference);
	TextOut(hbdc, 8, y, s, strlen(s));

	// Display frequency difference

	sprintf(s, "%+8.2lfHz", display.f - display.fr);
	TextOut(hbdc, width / 2, y, s, strlen(s));
    }

    // Copy the bitmap

    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw meter

BOOL DrawMeter(HDC hdc, RECT rect)
{
    static HBITMAP bitmap;
    static HFONT font;
    static HDC hbdc;

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

    // Select font

    if (font == NULL)
    {
	lf.lfHeight = 16;
	font = CreateFontIndirect(&lf);
    }

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

	SelectObject(hbdc, font);
	SetTextAlign(hbdc, TA_CENTER);
    }

    // Erase background

    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, GetStockObject(WHITE_BRUSH));

    // Move origin

    SetViewportOrgEx(hbdc, width / 2, 0, NULL);

    // Draw the meter scale

    for (int i = 0; i < 6; i++)
    {
	int x = width / 11 * i;
	static char s[16];

	sprintf(s, "%d", i * 10);
	TextOut(hbdc, x + 1, 0, s, strlen(s));
	TextOut(hbdc, -x + 1, 0, s, strlen(s));

	MoveToEx(hbdc, x, 14, NULL);
	LineTo(hbdc, x, 20);
	MoveToEx(hbdc, -x, 14, NULL);
	LineTo(hbdc, -x, 20);

	for (int j = 1; j < 5; j++)
	{
	    if (i < 5)
	    {
		MoveToEx(hbdc, x + j * width / 55, 16, NULL);
		LineTo(hbdc, x + j * width / 55, 20);
	    }

	    MoveToEx(hbdc, -x + j * width / 55, 16, NULL);
	    LineTo(hbdc, -x + j * width / 55, 20);
	}
    }

    // Move origin back

    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Copy the bitmap

    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw strobe

BOOL DrawStrobe(HDC hdc, RECT rect)
{
    static float mc = 0.0;
    static float mx = 0.0;

    static HDC hbdc;
    static HBITMAP bitmap;

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create DC

    if (hbdc == NULL)
    {
	hbdc = CreateCompatibleDC(hdc);
	bitmap = CreateCompatibleBitmap(hdc, width, height);
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

    SelectObject(hdc, GetStockObject(BLACK_PEN));
    SelectObject(hdc, GetStockObject(BLACK_BRUSH));

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

    SelectObject(hdc, GetStockObject(BLACK_PEN));
    SelectObject(hdc, GetStockObject(BLACK_BRUSH));

    Rectangle(hdc, rect.left + 2, (rect.bottom - rect.top) / 2 + rect.top - 1,
	      rect.right - 2, (rect.bottom - rect.top) / 2 + rect.top + 1);
    Rectangle(hdc, (rect.right - rect.left) / 2 + rect.left - 1, rect.top + 2,
	      (rect.right - rect.left) / 2 + rect.left + 1, rect.bottom - 2);
}

// Plus clicked

BOOL PlusClicked(WPARAM wParam, LPARAM lParam)
{
    switch(HIWORD(wParam))
    {
    case BN_CLICKED:
	PlusPressed(wParam, lParam);
	break;

    default:
	return FALSE;
    }
}

// Plus pressed

BOOL PlusPressed(WPARAM wParam, LPARAM lParam)
{
    if (audio.reference == 0)
	return FALSE;

    int value = audio.reference * 10;

    SendMessage(adjust.hwnd, TBM_SETPOS, TRUE, ++value);
    audio.reference = value / 10.0;
    InvalidateRgn(display.hwnd, NULL, TRUE);

    static char s[64];

    sprintf(s, "Reference:  %6.2lfHz", audio.reference);
    SendMessage(legend.reference.hwnd, WM_SETTEXT, 0, (LPARAM)s);

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	RegSetValueEx(hkey, "Reference", 0, REG_DWORD,
		      (LPBYTE)&value, sizeof(value));

	RegCloseKey(hkey);
    }

    else
    {
	static char s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }

    return TRUE;
}

// Minus clicked

BOOL MinusClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	MinusPressed(wParam, lParam);
	break;

    default:
	return FALSE;
    }
}

BOOL MinusPressed(WPARAM wParam, LPARAM lParam)
{
    if (audio.reference == 0)
	return FALSE;

    int value = audio.reference * 10;

    SendMessage(adjust.hwnd, TBM_SETPOS, TRUE, --value);
    audio.reference = value / 10.0;
    InvalidateRgn(display.hwnd, NULL, TRUE);

    static char s[64];

    sprintf(s, "Reference:  %6.2lfHz", audio.reference);
    SendMessage(legend.reference.hwnd, WM_SETTEXT, 0, (LPARAM)s);

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	RegSetValueEx(hkey, "Reference", 0, REG_DWORD,
		      (LPBYTE)&value, sizeof(value));

	RegCloseKey(hkey);
    }

    else
    {
	static char s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }

    return TRUE;
}

// Display clicked

BOOL DisplayClicked(WPARAM wParam, LPARAM lParam)
{

    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	display.lock = !display.lock;
	break;

    default:
	return FALSE;
    }

    if (lock.hwnd != NULL)
	SendMessage(lock.hwnd, BM_SETCHECK,
		    display.lock? BST_CHECKED: BST_UNCHECKED, 0);

    InvalidateRgn(display.hwnd, NULL, TRUE);

    return TRUE;
}

// Scope clicked

BOOL ScopeClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	FilterClicked(wParam, lParam);
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

    if (zoom.hwnd != NULL)
	SendMessage(zoom.hwnd, BM_SETCHECK,
		    spectrum.zoom? BST_CHECKED: BST_UNCHECKED, 0);

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	RegSetValueEx(hkey, "Zoom", 0, REG_DWORD,
		      (LPBYTE)&spectrum.zoom, sizeof(spectrum.zoom));

	RegCloseKey(hkey);
    }

    else
    {
	static char s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);
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

    if (enable.hwnd != NULL)
	SendMessage(enable.hwnd, BM_SETCHECK,
		    strobe.enable? BST_CHECKED: BST_UNCHECKED, 0);

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	RegSetValueEx(hkey, "Strobe", 0, REG_DWORD,
		      (LPBYTE)&strobe.enable, sizeof(strobe.enable));

	RegCloseKey(hkey);
    }

    else
    {
	static char s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);
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

    static char s[64];

    sprintf(s, "Reference:  %6.2lfHz", audio.reference);
    SendMessage(legend.reference.hwnd, WM_SETTEXT, 0, (LPARAM)s);

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	RegSetValueEx(hkey, "Reference", 0, REG_DWORD,
		      (LPBYTE)&value, sizeof(value));

	RegCloseKey(hkey);
    }

    else
    {
	static char s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }

    return TRUE;
}

// Audio thread

DWORD WINAPI AudioThread(LPVOID lpParameter)
{
    // Create wave format structure

    static WAVEFORMATEX wf =
	{WAVE_FORMAT_PCM, CHANNELS,
	 SAMPLE_RATE, SAMPLE_RATE * BLOCK_ALIGN,
	 BLOCK_ALIGN, BITS_PER_SAMPLE, 0};

    MMRESULT mmr;

    // Open a waveform audio input device

    mmr = waveInOpen(&audio.hwi, WAVE_MAPPER | WAVE_FORMAT_DIRECT, &wf,
		     (DWORD_PTR)audio.id,  (DWORD_PTR)NULL, CALLBACK_THREAD);

    if (mmr != MMSYSERR_NOERROR)
    {
	static char s[64];

	waveInGetErrorText(mmr, s, sizeof(s));
	MessageBox(window.hwnd, s, "WaveInOpen", MB_OK | MB_ICONERROR);
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

	mmr = mixerGetLineInfo((HMIXEROBJ)mixer.hmx, &mxl,
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
	// Prepare a waveform audio input header

	mmr = waveInPrepareHeader(audio.hwi, &hdrs[i], sizeof(WAVEHDR));
	if (mmr != MMSYSERR_NOERROR)
	{
	    static char s[64];

	    waveInGetErrorText(mmr, s, sizeof(s));
	    MessageBox(window.hwnd, s, "WaveInPrepareHeader",
		       MB_OK | MB_ICONERROR);
	    return mmr;
	}

	// Add a waveform audio input buffer

	mmr = waveInAddBuffer(audio.hwi, &hdrs[i], sizeof(WAVEHDR));
	if (mmr != MMSYSERR_NOERROR)
	{
	    static char s[64];

	    waveInGetErrorText(mmr, s, sizeof(s));
	    MessageBox(window.hwnd, s, "WaveInAddBuffer",
		       MB_OK | MB_ICONERROR);
	    return mmr;
	}
    }

    // Start the waveform audio input

    mmr = waveInStart(audio.hwi);
    if (mmr != MMSYSERR_NOERROR)
    {
	static char s[64];

	waveInGetErrorText(mmr, s, sizeof(s));
	MessageBox(window.hwnd, s, "WaveInStart", MB_OK | MB_ICONERROR);
	return mmr;
    }

    // Set up reference value

    if (audio.reference == 0)
	audio.reference = A5_REFNCE;

    // Set up correction value

    audio.correction = 1.0;

    // Check for a stored value

    HKEY hkey;
    LONG error;
    DWORD value;
    int size = sizeof(value);

    error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\CTuner", 0,
			 KEY_READ, &hkey);

    if (error == ERROR_SUCCESS)
    {
	error = RegQueryValueEx(hkey, "Correction", NULL, NULL,
				(LPBYTE)&value, (LPDWORD)&size);

	if (error == ERROR_SUCCESS)
	{
	    audio.correction = value / 100000.0;
	    static char s[64];

	    sprintf(s, " Sample rate: %6.1lf\t\tCorrection: %6.5lf ",
		    (double)SAMPLE_RATE / audio.correction, audio.correction);
	    SendMessage(status.hwnd, SB_SETTEXT, 0, (LPARAM)s);
	}

	RegCloseKey(hkey);
    }

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
	    // Not used
	    break;

	    // Audio input data

	case MM_WIM_DATA:
	    WaveInData(msg.wParam, msg.lParam);
	    break;

	    // Audio input closed

	case MM_WIM_CLOSE:
	    // Not used
	    break;
	}
    }

    return msg.wParam;
}

void WaveInData(WPARAM wParam, LPARAM lParam)
{
    // Create buffers for processing the audio data

    static double buffer[SAMPLES];
    static complex x[SAMPLES];

    static double xa[RANGE];
    static double xp[RANGE];
    static double xf[RANGE];

    static double dxa[RANGE];
    static double dxf[RANGE];

    static double maxima[MAXIMA];
    static float values[MAXIMA];

    static double fps = (double)SAMPLE_RATE / (double)SAMPLES;
    static double expect = 2.0 * M_PI * (double)STEP / (double)SAMPLES;

    // initialise data structs

    if (scope.data == NULL)
    {
	scope.length = STEP;

	spectrum.data = xa;
	spectrum.length = RANGE;

	spectrum.values = values;
	display.maxima = maxima;
    }

    // Copy the input data

    memmove(buffer, buffer + STEP, (SAMPLES - STEP) * sizeof(double));

    short *data = (short *)((WAVEHDR *)lParam)->lpData;

    // Butterworth filter, 3dB/octave

    for (int i = 0; i < STEP; i++)
    {
	static double xv[2];
	static double yv[2];

	xv[0] = xv[1];
	xv[1] = (double)data[i] /
	    3.023332184e+01;

	yv[0] = yv[1];
	yv[1] = (xv[0] + xv[1]) +
	    (0.9338478249 * yv[0]);

	// Choose filtered/unfiltered data

	buffer[SAMPLES - STEP + i] =
	    audio.filter? yv[1]: (double)data[i];
    }

    // Give the buffer back

    waveInAddBuffer(audio.hwi, (WAVEHDR *)lParam, sizeof(WAVEHDR));

    // Maximum data value

    static double dmax;

    if (dmax < 4096.0)
	dmax = 4096.0;

    // Calculate normalising value

    double norm = dmax;
    dmax = 0.0;

    // Copy data to FFT input arrays

    for (int i = 0; i < LENGTH(buffer); i++)
    {
	// Find the magnitude

	if (dmax < fabs(buffer[i]))
	    dmax = fabs(buffer[i]);

	// Calculate the window

	double window =
	    0.5 - 0.5 * cos(2.0 * M_PI *
			    i / SAMPLES);

	// Normalise and window the input data

	x[i].r = (double)buffer[i] / norm * window;
    }

    // do FFT

    fftr(x, LENGTH(x));

    // Process FFT output

    for (int i = 1; i < LENGTH(xa); i++)
    {
	double real = x[i].r;
	double imag = x[i].i;

	xa[i] = sqrt((real * real) + (imag * imag));
#ifdef NOISE
	// Do noise cancellation

	xm[i] = (xa[i] + (xm[i] * 19.0)) / 20.0;

	if (xm[i] > xa[i])
	    xm[i] = xa[i];

	xd[i] = xa[i] - xm[i];
#endif
	// Do frequency calculation

	double p = atan2(imag, real);
	double dp = xp[i] - p;
	xp[i] = p;

	// Calculate phase difference

	dp -= (double)i * expect;

	int qpd = dp / M_PI;

	if (qpd >= 0)
	    qpd += qpd & 1;

	else
	    qpd -= qpd & 1;

	dp -=  M_PI * (double)qpd;

	// Calculate frequency difference

	double df = OVERSAMPLE * dp / (2.0 * M_PI);

	// Calculate actual frequency from slot frequency plus
	// frequency difference and correction value

	xf[i] = (i * fps + df * fps) / audio.correction;

	// Calculate differences for finding maxima

	dxa[i] = xa[i] - xa[i - 1];
	dxf[i] = xf[i] - xf[i - 1];
    }

    // Maximum FFT output

    double max = 0.0;
    double f = 0.0;
    int count = 0;

    // Find maximum value, and list of maxima

    for (int i = 1; i < LENGTH(xa) - 1; i++)
    {
	if (xa[i] > max)
	{
	    max = xa[i];
	    f = xf[i];
	}

	// If display not locked, find maxima and add to list

	if (!display.lock &&
	    xa[i] > MIN && xa[i] > (max / 2) &&
	    dxa[i] > 0.0 && dxa[i + 1] < 0.0)
	    maxima[count++] = xf[i];

	if (count == LENGTH(maxima))
	    break;
    }

    // Reference note frequency and lower and upper limits

    double fr = 0.0;
    double fx0 = 0.0;
    double fx1 = 0.0;

    // Note number

    int n = 0;

    // Found flag and cents value

    BOOL found = FALSE;
    double c = 0.0;

    // Do the note and cents calculations

    if (max > MIN)
    {
	// Cents relative to reference

	double cf =
	    -12.0 * (log(audio.reference / f) / log(2.0));

	// Reference note

	fr = audio.reference * pow(2.0, round(cf) / 12.0);

	// Lower and upper freq

	fx0 = audio.reference * pow(2.0, (round(cf) - 0.5) / 12.0);
	fx1 = audio.reference * pow(2.0, (round(cf) + 0.5) / 12.0);

	// Note number

	n = round(cf) + A5_OFFSET;

	if (n < 0)
	    n = 0;

	// Find nearest maximum to reference note

	double df = 1000.0;

	for (int i = 0; i < count; i++)
	{
	    if (fabs(maxima[i] - fr) < df)
	    {
		df = fabs(maxima[i] - fr);
		f = maxima[i];
	    }
	}

	// Cents relative to reference note

	c = -12.0 * (log(fr / f) / log(2.0));

	// Ignore silly values

	if (!isfinite(c))
	    return;

	// Ignore if not within 50 cents of reference note

	if (fabs(c) < 0.5)
	    found = TRUE;
    }

    // If display not locked

    if (!display.lock)
    {
	// Update scope window

	scope.data = data;
	InvalidateRgn(scope.hwnd, NULL, TRUE);

	// Update spectrum window

	for (int i = 0; i < count; i++)
	    values[i] = maxima[i] / fps * audio.correction;

	spectrum.count = count;
	InvalidateRgn(spectrum.hwnd, NULL, TRUE);
    }

    if (found)
    {
	// If display not locked

	if (!display.lock)
	{
	    // Update the display struct

	    display.f = f;
	    display.fr = fr;
	    display.c = c;
	    display.n = n;
	    display.count = count;

	    // Update meter

	    meter.c = c;

	    // Update strobe

	    strobe.c = c;
	}

	// Update display

	InvalidateRgn(display.hwnd, NULL, TRUE);

	spectrum.f = f  / fps * audio.correction;
	spectrum.r = fr / fps * audio.correction;
	spectrum.x[0] = fx0 / fps * audio.correction;
	spectrum.x[1] = fx1 / fps * audio.correction;
    }
}

// Real to complex FFT, ignores imaginary values in input array

void fftr(complex a[], int n)
{
    double norm = sqrt(1.0 / n);

    for (int i = 0, j = 0; i < n; i++)
    {
	if (j >= i)
	{
	    double tr = a[j].r * norm;

	    a[j].r = a[i].r * norm;
	    a[j].i = 0.0;

	    a[i].r = tr;
	    a[i].i = 0.0;
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
	double delta = (M_PI / mmax);
	for (int m = 0; m < mmax; m++)
	{
	    double w = m * delta;
	    double wr = cos(w);
	    double wi = sin(w);

	    for (int i = m; i < n; i += istep)
	    {
		int j = i + mmax;
		double tr = wr * a[j].r - wi * a[j].i;
		double ti = wr * a[j].i + wi * a[j].r;
		a[j].r = a[i].r - tr;
		a[j].i = a[i].i - ti;
		a[i].r += tr;
		a[i].i += ti;
	    }
	}
    }
}
