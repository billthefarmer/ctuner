////////////////////////////////////////////////////////////////////////////////
//
//  SigGen - A signal generator written in C.
//
//  Copyright (C) 2010  Bill Farmer
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

#define WCLASS   "MainWClass"
#define KNOBCLASS "KnobClass"

// Global handle

HINSTANCE hInst;

// Tool ids

enum
    {SCALE_ID = 101,
     KNOB_ID,
     FINE_ID,
     LEVEL_ID,
     DISPLAY_ID,
     SINE_ID,
     SQUARE_ID,
     SAWTOOTH_ID,
     MUTE_ID,
     QUIT_ID,
     STATUS_ID};

// Wave out values

enum
    {SAMPLE_RATE = 44100L,
     BITS_PER_SAMPLE = 16,
     MAX_LEVEL = 32767,
     BLOCK_ALIGN = 2,
     CHANNELS = 1};

// Buffer size

enum
    {SAMPLES = 4096};

// Window size

enum
    {WIDTH  = 320,
     HEIGHT = 276};

// Frequency scale

enum
    {FREQ_SCALE = 250,
     FREQ_MAX   = 850,
     FREQ_MIN   = 0};

// Fine slider

enum
    {FINE_MAX  = 100,
     FINE_MIN  = 0,
     FINE_REF  = 50,
     FINE_STEP = 10};

// Level slider

enum
    {LEVEL_MAX = 100,
     LEVEL_MIN = 0,
     LEVEL_REF = 80,
     LEVEL_STEP = 10};

// Waveform

enum
    {SINE,
     SQUARE,
     SAWTOOTH};

// Global data

typedef struct
{
    HWND hwnd;
} TOOL, *TOOLP;

TOOL window;
TOOL knob;
TOOL fine;
TOOL level;
TOOL group;
TOOL status;

typedef struct
{
    HWND hwnd;
    INT value;
} SCALE, *SCALEP;

SCALE scale =
    {NULL, FREQ_SCALE * 2};

typedef struct
{
    HWND hwnd;
    double frequency;
    double decibels;
} DISPLAY, *DISPLAYP;

DISPLAY display =
    {NULL, 1000.0, -20.0};

struct
{
    TOOL sine;
    TOOL square;
    TOOL sawtooth;
    TOOL mute;
    TOOL quit;
} button;

typedef struct
{
    HWND hwnd;
    TOOLINFO info;
} TOOLTIP, *TOOLTIPP;

TOOLTIP tooltip;

typedef struct
{
    HWAVEOUT hwo;
    HANDLE thread;
    int waveform;
    int level;
    BOOL mute;
    DWORD id;
} AUDIO, *AUDIOP;

AUDIO audio;

// Function prototypes.

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK KnobProc(HWND, UINT, WPARAM, LPARAM);
BOOL RegisterMainClass(HINSTANCE);
BOOL RegisterKnobClass(HINSTANCE);
BOOL DrawItem(WPARAM, LPARAM);
BOOL DrawDisplay(HDC, RECT, UINT);
BOOL DrawScale(HDC, RECT, UINT);
BOOL DrawKnob(HDC, RECT, UINT);
VOID CharPressed(WPARAM, LPARAM);
VOID KeyDown(WPARAM, LPARAM);
VOID KnobClicked(HWND, WPARAM, LPARAM);
VOID MouseMove(HWND, WPARAM, LPARAM);
BOOL SliderChange(WPARAM, LPARAM);
VOID UpdateValues(VOID);
DWORD WINAPI AudioThread(LPVOID);

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

    // Register knob class

    RegisterKnobClass(hInstance);

    // Create the main window.

    window.hwnd =
	CreateWindow(WCLASS, "Audio Signal Generator",
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
	 MainWndProc,
	 0, 0, hInst,
	 LoadIcon(hInst, "SigGen"),
	 LoadCursor(NULL, IDC_ARROW),
	 GetSysColorBrush(COLOR_WINDOW),
	 NULL, WCLASS};

    // Register the window class.

    return RegisterClass(&wc);
}

// Main window procedure

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg,
			     WPARAM wParam, LPARAM lParam)
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

	// Create scale

	scale.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 8, 8, 160, 40, hWnd,
			 (HMENU)SCALE_ID, hInst, NULL);

	// Create tooltip

	tooltip.hwnd =
	    CreateWindow(TOOLTIPS_CLASS, NULL,
			 WS_POPUP | TTS_ALWAYSTIP,
			 CW_USEDEFAULT, CW_USEDEFAULT,
			 CW_USEDEFAULT, CW_USEDEFAULT,
			 hWnd, NULL, hInst, NULL);

	SetWindowPos(tooltip.hwnd, HWND_TOPMOST, 0, 0, 0, 0,
		     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// Add scale to tooltip

	tooltip.info.cbSize = sizeof(tooltip.info);
	tooltip.info.hwnd = hWnd;
	tooltip.info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;

	tooltip.info.uId = (UINT_PTR)scale.hwnd;
	tooltip.info.lpszText = "Frequency scale";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create display

	display.hwnd =
	    CreateWindow(WC_STATIC, NULL,
			 WS_VISIBLE | WS_CHILD |
			 SS_NOTIFY | SS_OWNERDRAW,
			 176, 8, 136, 60, hWnd,
			 (HMENU)DISPLAY_ID, hInst, NULL);

	// Add display to tooltip

	tooltip.info.uId = (UINT_PTR)display.hwnd;
	tooltip.info.lpszText = "Frequency display";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create knob

	knob.hwnd =
	    CreateWindow(KNOBCLASS, NULL,
			 WS_VISIBLE | WS_CHILD,
			 8, 56, 160, 160, hWnd,
			 (HMENU)KNOB_ID, hInst, NULL);

	// Add knob to tooltip

	tooltip.info.uId = (UINT_PTR)knob.hwnd;
	tooltip.info.lpszText = "Frequency adjust";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create frequency slider

	fine.hwnd =
	    CreateWindow(TRACKBAR_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 TBS_VERT | TBS_NOTICKS,
			 176, 76, 24, 140, hWnd,
			 (HMENU)FINE_ID, hInst, NULL);

	SendMessage(fine.hwnd, TBM_SETRANGE, TRUE,
		    MAKELONG(FINE_MIN, FINE_MAX));
	SendMessage(fine.hwnd, TBM_SETPAGESIZE, 0, FINE_STEP);
	SendMessage(fine.hwnd, TBM_SETPOS, TRUE, FINE_REF);

	// Add slider to tooltip

	tooltip.info.uId = (UINT_PTR)fine.hwnd;
	tooltip.info.lpszText = "Frequency adjust";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create level slider

	level.hwnd =
	    CreateWindow(TRACKBAR_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 TBS_VERT | TBS_NOTICKS | TBS_LEFT,
			 208, 76, 24, 140, hWnd,
			 (HMENU)LEVEL_ID, hInst, NULL);

	SendMessage(level.hwnd, TBM_SETRANGE, TRUE,
		    MAKELONG(LEVEL_MIN, LEVEL_MAX));
	SendMessage(level.hwnd, TBM_SETPAGESIZE, 0, LEVEL_STEP);
	SendMessage(level.hwnd, TBM_SETPOS, TRUE, LEVEL_REF);

	// Add slider to tooltip

	tooltip.info.uId = (UINT_PTR)level.hwnd;
	tooltip.info.lpszText = "Level adjust";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create sine button

	button.sine.hwnd =
	    CreateWindow(WC_BUTTON, "Sine",
			 WS_VISIBLE | WS_CHILD | WS_GROUP |
			 BS_AUTORADIOBUTTON | BS_PUSHLIKE,
			 240, 76, 72, 24, hWnd,
			 (HMENU)SINE_ID, hInst, NULL);

	SendMessage(button.sine.hwnd, BM_SETCHECK, BST_CHECKED, 0);

	// Create square button

	button.square.hwnd =
	    CreateWindow(WC_BUTTON, "Square",
			 WS_VISIBLE | WS_CHILD |
			 BS_AUTORADIOBUTTON | BS_PUSHLIKE,
			 240, 105, 72, 24, hWnd,
			 (HMENU)SQUARE_ID, hInst, NULL);

	// Create sawtooth button

	button.sawtooth.hwnd =
	    CreateWindow(WC_BUTTON, "Sawtooth",
			 WS_VISIBLE | WS_CHILD |
			 BS_AUTORADIOBUTTON | BS_PUSHLIKE,
			 240, 134, 72, 24, hWnd,
			 (HMENU)SAWTOOTH_ID, hInst, NULL);

	// Create mute button

	button.mute.hwnd =
	    CreateWindow(WC_BUTTON, "Mute",
			 WS_VISIBLE | WS_CHILD |
			 BS_AUTOCHECKBOX | BS_PUSHLIKE,
			 240, 161, 72, 24, hWnd,
			 (HMENU)MUTE_ID, hInst, NULL);

	// Create quit button

	button.quit.hwnd =
	    CreateWindow(WC_BUTTON, "Quit",
			 WS_VISIBLE | WS_CHILD,
			 240, 192, 72, 24, hWnd,
			 (HMENU)QUIT_ID, hInst, NULL);

	// Create status bar

	status.hwnd =
	    CreateWindow(STATUSCLASSNAME, "",
			 WS_VISIBLE | WS_CHILD,
			 0, 0, 0, 0, hWnd,
			 (HMENU)STATUS_ID, hInst, NULL);

	// Start audio thread

	audio.thread = CreateThread(NULL, 0, AudioThread,
				    hWnd, 0, &audio.id);
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

	// Char pressed

    case WM_CHAR:
	CharPressed(wParam, lParam);
	break;

	// Key pressed

    case WM_KEYDOWN:
	KeyDown(wParam, lParam);
	break;

	// Buttons

    case WM_COMMAND:
	switch (LOWORD(wParam))
	{
	case SINE_ID:
	    audio.waveform = SINE;
	    break;

	case SQUARE_ID:
	    audio.waveform = SQUARE;
	    break;

	case SAWTOOTH_ID:
	    audio.waveform = SAWTOOTH;
	    break;

	case MUTE_ID:
	    audio.mute = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0);
	    break;

	case QUIT_ID:
	    PostQuitMessage(0);
	    break;
	}

	// Set the focus back to the window

	SetFocus(hWnd);
	break;

	// Slider change

    case WM_VSCROLL:
	SliderChange(wParam, lParam);

	// Set the focus back to the window

	SetFocus(hWnd);
	break;

	// Mouse move

	// Set the focus back to the window by clicking

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
	SetFocus(hWnd);
	break;

    case WM_RBUTTONDOWN:
	// DisplayContextMenu(hWnd, MAKEPOINTS(lParam));
	break;

    case WM_MOUSEMOVE:
    	MouseMove(hWnd, wParam, lParam);
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

// Register class

BOOL RegisterKnobClass(HINSTANCE hinst)
{
    // Fill in the window class structure with parameters
    // that describe the main window.

    WNDCLASS wc = 
	{0, KnobProc,
	 0, 0, hinst,
	 NULL, LoadCursor(NULL, IDC_ARROW),
	 GetSysColorBrush(COLOR_WINDOW),
	 NULL, KNOBCLASS};

    // Register the window class.

    return RegisterClass(&wc);
}

// Knob procedure

LRESULT CALLBACK KnobProc(HWND hWnd, UINT uMsg,
			  WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    RECT rect;
    HDC hdc;

    switch (uMsg)
    {
	// Paint

    case WM_PAINT:
	hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &rect);
	DrawKnob(hdc, rect, 0);
	EndPaint(hWnd, &ps);
	break;

	// Char pressed

    case WM_CHAR:
    	CharPressed(wParam, lParam);
    	break;

	// Key pressed

    case WM_KEYDOWN:
    	KeyDown(wParam, lParam);
    	break;

	// Mouse move

    case WM_MOUSEMOVE:
    	MouseMove(hWnd, wParam, lParam);
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
    case SCALE_ID:
	DrawScale(hdc, rect, state);
	break;

    case DISPLAY_ID:
	DrawDisplay(hdc, rect, state);
	break;

    case KNOB_ID:
	DrawKnob(hdc, rect, state);
	break;
    }
}

// Draw Scale

BOOL DrawScale(HDC hdc, RECT rect, UINT state)
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
	SelectObject(hbdc, GetStockObject(DC_PEN));

	lf.lfHeight = 16;
	font = CreateFontIndirect(&lf);
	SelectObject(hbdc, font);
	SetTextAlign(hbdc, TA_CENTER | TA_BOTTOM);
    }

    // Erase background

    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, GetStockObject(WHITE_BRUSH));

    // Translate viewport

    SetViewportOrgEx(hbdc, width / 2, height / 2, NULL);

    // Draw scale

    for (int i = 1; i < 11; i++)
    {
	int x = round(FREQ_SCALE * log10(i)) - scale.value;

	for (int j = 0; j < 4; j++)
	{
	    MoveToEx(hbdc, x, 0, NULL);
	    LineTo(hbdc, x, height / 2);
	    x += FREQ_SCALE;
	}
    }

    for (int i = 3; i < 20; i += 2)
    {
	int x = round(FREQ_SCALE * log10(i / 2.0)) - scale.value;

	for (int j = 0; j < 4; j++)
	{
	    MoveToEx(hbdc, x, 6, NULL);
	    LineTo(hbdc, x, height / 2);
	    x += FREQ_SCALE;
	}
    }

    int a[] = {1, 2, 3, 4, 5, 6, 7, 8};
    for (int i = 0; i < LENGTH(a); i++)
    {
    	int x = round(FREQ_SCALE * log10(a[i])) - scale.value;

    	for (int j = 0; j < 2; j++)
    	{
	    static char s[8];

	    sprintf(s, "%d", a[i]);
    	    TextOut(hbdc, x, 0, s, strlen(s)); 

	    sprintf(s, "%d", a[i] * 10);
    	    TextOut(hbdc, x + FREQ_SCALE, 0, s, strlen(s));

    	    x += 2 * FREQ_SCALE;
    	}
    }

    // Move the origin back

    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Draw line

    MoveToEx(hbdc, width / 2, 0, NULL);
    LineTo(hbdc, width / 2, height);

    // Copy the bitmap

    BitBlt(hdc, rect.left, rect.top, width, height,
    	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw Display

BOOL DrawDisplay(HDC hdc, RECT rect, UINT state)
{
    // Font height

    enum
    {FONT_HEIGHT = 24};

    static HFONT font;
    static char text[16];

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

    if (font == NULL)
    {
	lf.lfHeight = FONT_HEIGHT;
	font = CreateFontIndirect(&lf);
    }

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate dimensions

    int x = rect.left + 8;
    int y = rect.top + 4;

    SelectObject(hdc, font);

    sprintf(text, "%6.1lfHz  ", display.frequency);
    TextOut(hdc, x, y, text, strlen(text));

    y += FONT_HEIGHT;

    sprintf(text, "%+6.1lfdB  ", display.decibels);
    TextOut(hdc, x, y, text, strlen(text));
}

// Draw Knob

BOOL DrawKnob(HDC hdc, RECT rect, UINT state)
{
    // Calculate dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    SelectObject(hdc, GetStockObject(DC_PEN));
    SelectObject(hdc, GetStockObject(DC_BRUSH));

    // Draw knob

    SetDCPenColor(hdc, GetSysColor(COLOR_3DSHADOW));
    SetDCBrushColor(hdc, GetSysColor(COLOR_BTNFACE));

    Ellipse(hdc, rect.left, rect.top, width, width);

    return TRUE;
}

// Char pressed

void CharPressed(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    default:
	return;
    }
}

// Key pressed

void KeyDown(WPARAM wParam, LPARAM lParam)
{
    int value;

    switch(wParam)
    {
    case VK_RIGHT:
	if (++scale.value > FREQ_MAX)
	    scale.value = FREQ_MAX;
	break;

    case VK_LEFT:
	if (--scale.value < FREQ_MIN)
	    scale.value = FREQ_MIN;
	break;

    case VK_UP:
	value = SendMessage(fine.hwnd, TBM_GETPOS, 0, 0);
	SendMessage(fine.hwnd, TBM_SETPOS, TRUE, --value);
	break;

    case VK_DOWN:
	value = SendMessage(fine.hwnd, TBM_GETPOS, 0, 0);
	SendMessage(fine.hwnd, TBM_SETPOS, TRUE, ++value);
	break;

    default:
	return;
    }

    UpdateValues();
}

// Knob clicked

void KnobClicked(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    switch(HIWORD(wParam))
    {
    case STN_CLICKED:
	break;
    }
}

// Mouse move

void MouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    static float last;
    static BOOL move;

    POINTS points = MAKEPOINTS(lParam);

    if (wParam & MK_LBUTTON)
    {
	RECT rect;

	GetClientRect(hwnd, &rect);

	int w2 = rect.right / 2;
	int h2 = rect.bottom / 2;

	int x = points.x - w2;
	int y = points.y - h2;

	float theta = atan2(x, -y);

	if (!move)
	    move = TRUE;

	else
	{
	    float delta = theta - last;

	    if (delta > M_PI)
		delta -= 2.0 * M_PI;

	    if (delta < -M_PI)
		delta += 2.0 * M_PI;

	    scale.value += round(delta * 100.0 / M_PI);

	    if (scale.value < FREQ_MIN)
		scale.value = FREQ_MIN;

	    if (scale.value > FREQ_MAX)
		scale.value = FREQ_MAX;

	    UpdateValues();
	}

	last = theta;
    }

    else
	if (move)
	    move = FALSE;
}

// Slider change

BOOL SliderChange(WPARAM wParam, LPARAM lParam)
{
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

    UpdateValues();
}

// Update values

void UpdateValues()
{
    // Update frequency

    int value = SendMessage(fine.hwnd, TBM_GETPOS, 0, 0);
    double fine = -(double)(value - FINE_REF) / 10000.0;

    double frequency = pow(10.0, (double)scale.value / FREQ_SCALE) * 10.0;
    display.frequency = frequency + (frequency * fine);

    // Update level

    value = SendMessage(level.hwnd, TBM_GETPOS, 0, 0);
    audio.level = MAX_LEVEL * (LEVEL_MAX - value) / LEVEL_MAX;
    display.decibels = log10((double)(LEVEL_MAX - value) / 200.0) * 20.0;

    if (display.decibels < -80.0)
	display.decibels = -80.0;

    InvalidateRgn(display.hwnd, NULL, TRUE);
    InvalidateRgn(scale.hwnd, NULL, TRUE);
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

    // Open a waveform audio output device

    mmr = waveOutOpen(&audio.hwo, WAVE_MAPPER | WAVE_FORMAT_DIRECT, &wf,
		     (DWORD_PTR)audio.id,  (DWORD_PTR)NULL, CALLBACK_THREAD);
    if (mmr != MMSYSERR_NOERROR)
    {
	char text[64];

	waveOutGetErrorText(mmr, text, sizeof(text));
	MessageBox(window.hwnd, text, "WaveOutOpen", MB_OK | MB_ICONERROR);
	return mmr;
    }

    // Create the waveform audio output buffers and structures

    static short data[2][SAMPLES];
    static WAVEHDR hdrs[2] =
	{{(LPSTR)data[0], sizeof(data[0]), 0, 0, 0, 0},
	 {(LPSTR)data[1], sizeof(data[1]), 0, 0, 0, 0}};

    for (int i = 0; i < LENGTH(hdrs); i++)
    {
	// Prepare a waveform audio input header

	mmr = waveOutPrepareHeader(audio.hwo, &hdrs[i], sizeof(WAVEHDR));
	if (mmr != MMSYSERR_NOERROR)
	{
	    static char s[64];

	    waveOutGetErrorText(mmr, s, sizeof(s));
	    MessageBox(window.hwnd, s, "WaveOutPrepareHeader",
		       MB_OK | MB_ICONERROR);
	    return mmr;
	}
    }

    // Create a message loop for processing thread messages

    MSG msg;
    BOOL flag;

    while ((flag = GetMessage(&msg, (HWND)-1, 0, 0)) != 0)
    {
	static float K = 2.0 * M_PI / SAMPLE_RATE;
	static float q = 0.0;
	static float f;
	static float l;

	WAVEHDR *hdrp;
	short *datap;

	if (flag == -1)
	    break;

	// Process messages

	switch (msg.message)
	{
	    // Audio input opened

	case MM_WOM_OPEN:

	    f = display.frequency;
	    audio.level = MAX_LEVEL * 20 / 100;

	    for (int i = 0; i < LENGTH(data); i++)
	    {
		for (int j = 0; j < SAMPLES; j++)
		{
		    f += ((display.frequency - f) / (float)SAMPLES);
		    l += ((audio.level - l) / (float)SAMPLES);

		    q += (q < M_PI)? f * K: (f * K) - (2.0 * M_PI);

		    data[i][j] = round(sin(q) * l);
		}

		mmr = waveOutWrite(audio.hwo, &hdrs[i], sizeof(WAVEHDR));
		if (mmr != MMSYSERR_NOERROR)
		{
		    static char s[64];

		    waveOutGetErrorText(mmr, s, sizeof(s));
		    MessageBox(window.hwnd, s, "WaveOutWrite",
			       MB_OK | MB_ICONERROR);
		    return mmr;
		}
	    }
	    break;

	    // Audio input data

	case MM_WOM_DONE:

	    hdrp = (WAVEHDR *) msg.lParam;
	    datap = (short *)hdrp->lpData;

	    for (int i = 0; i < SAMPLES; i++)
	    {
		f += ((display.frequency - f) / (float)SAMPLES);
		l += audio.mute? -l / (float)SAMPLES: 
		    (audio.level - l) / (float)SAMPLES;

		q += (q < M_PI)? f * K: (f * K) - (2.0 * M_PI);

		switch (audio.waveform)
		{
		case SINE:
		    datap[i] = round(sin(q) * l);
		    break;

		case SQUARE:
		    datap[i] = (q > 0.0)? l: -l;
		    break;

		case SAWTOOTH:
		    datap[i] = round((q / M_PI) * l);
		    break;
		}
	    }

	    mmr =  waveOutWrite(audio.hwo, hdrp, sizeof(WAVEHDR));
	    if (mmr != MMSYSERR_NOERROR)
	    {
		static char s[64];

		waveOutGetErrorText(mmr, s, sizeof(s));
		MessageBox(window.hwnd, s, "WaveOutWrite",
			   MB_OK | MB_ICONERROR);
		return mmr;
	    }

	    break;

	    // Audio input closed

	case MM_WOM_CLOSE:

	    // Not used

	    break;
	}
    }

    return msg.wParam;
}

