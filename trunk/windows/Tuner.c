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

#include "Tuner.h"

// Global handle

HINSTANCE hInst;

// Global data

WINDOW window;

TOOL status;

TOOLTIP tooltip;

TOOL volume;

SCOPE scope;

SPECTRUM spectrum;

DISPLAY display;

TOOL group;
TOOL zoom;
TOOL text;
TOOL lock;
TOOL down;
TOOL resize;
TOOL filter;
TOOL enable;
TOOL options;
TOOL multiple;
TOOL reference;
TOOL correction;

BUTTON button;

LEGEND legend;

METER meter;

STROBE strobe;

AUDIO audio;

MIXER mixer;

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

    GetSavedStatus();

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

VOID GetSavedStatus()
{
    HKEY hkey;
    LONG error;
    DWORD value;
    int size = sizeof(value);

    // Initial values

    strobe.enable = TRUE;
    audio.filter = TRUE;
    spectrum.expand = 1;

    // Reference initial value

    audio.reference = A5_REFNCE;

    // Open user key

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

	// Strobe colours

	error = RegQueryValueEx(hkey, "Colours", NULL, NULL,
				(LPBYTE)&value,(LPDWORD)&size);
	// Update value

	if (error == ERROR_SUCCESS)
	    strobe.colours = value;

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

    // Correction initial value

    audio.correction = 1.0;

    // Open local machine key

    error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\CTuner", 0,
			 KEY_READ, &hkey);

    if (error == ERROR_SUCCESS)
    {
	// Correction

	error = RegQueryValueEx(hkey, "Correction", NULL, NULL,
				(LPBYTE)&value, (LPDWORD)&size);
	// Update value

	if (error == ERROR_SUCCESS)
	    audio.correction = value / 100000.0;

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

	int border = (window.rwnd.right - window.rwnd.left) -
	    window.rclt.right;
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
	tooltip.info.lpszText = (LPSTR)"Microphone volume";

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
	tooltip.info.lpszText = (LPSTR)"Scope, click to filter audio";

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
	tooltip.info.lpszText = (LPSTR)"Spectrum, click to zoom";

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
	tooltip.info.lpszText = (LPSTR)"Display, click to lock";

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
	tooltip.info.lpszText = (LPSTR)"Strobe, click to disable/enable";

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
	tooltip.info.lpszText = (LPSTR)"Cents, click to lock";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create meter slider

	meter.slider.hwnd =
	    CreateWindow(TRACKBAR_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 TBS_HORZ | TBS_NOTICKS | TBS_TOP,
			 12, 272, width - 24, 26, hWnd,
			 (HMENU)SLIDER_ID, hInst, NULL);

	SendMessage(meter.slider.hwnd, TBM_SETRANGE, TRUE,
		    MAKELONG(MIN_METER, MAX_METER));
	SendMessage(meter.slider.hwnd, TBM_SETPOS, TRUE, REF_METER);

	// Add slider to tooltip

	tooltip.info.uId = (UINT_PTR)meter.slider.hwnd;
	tooltip.info.lpszText = (LPSTR)"Cents";

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
			 " Reference: 440.00Hz\t"
			 "\tCorrection: 1.00000 ",
			 WS_VISIBLE | WS_CHILD,
			 0, 0, 0, 0, hWnd,
			 (HMENU)STATUS_ID, hInst, NULL);

	// Start audio thread

	audio.thread = CreateThread(NULL, 0, AudioThread, hWnd,
				    0, &audio.id);

	// Start meter timer

	CreateTimerQueueTimer(&meter.timer, NULL,
			      (WAITORTIMERCALLBACK)MeterCallback,
			      &meter.hwnd, METER_DELAY, METER_DELAY,
			      WT_EXECUTEDEFAULT);

	// Start strobe timer

	CreateTimerQueueTimer(&meter.timer, NULL,
			      (WAITORTIMERCALLBACK)StrobeCallback,
			      &strobe.hwnd, STROBE_DELAY, STROBE_DELAY,
			      WT_EXECUTEDEFAULT);
	// Update status bar

	UpdateStatusBar();
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
	    // Scope

	case SCOPE_ID:
	    ScopeClicked(wParam, lParam);
	    break;

	    // Display

	case DISPLAY_ID:
	    DisplayClicked(wParam, lParam);
	    break;

	    // Spectrum

	case SPECTRUM_ID:
	    SpectrumClicked(wParam, lParam);
	    break;

	    // Strobe

	case STROBE_ID:
	    StrobeClicked(wParam, lParam);
	    break;

	    // Meter

	case METER_ID:
	    MeterClicked(wParam, lParam);
	    break;

	    // Zoom control

	case ZOOM_ID:
	    ZoomClicked(wParam, lParam);
	    break;

	    // Enable control

	case ENABLE_ID:
	    EnableClicked(wParam, lParam);
	    break;

	    // Filter control

	case FILTER_ID:
	    FilterClicked(wParam, lParam);
	    break;

	    // Downsample control

	case DOWN_ID:
	    DownClicked(wParam, lParam);
	    break;

	    // Lock control

	case LOCK_ID:
	    LockClicked(wParam, lParam);
	    break;

	    // Resize control

	case RESIZE_ID:
	    ResizeClicked(wParam, lParam);
	    break;

	    // Multiple control

	case MULTIPLE_ID:
	    MultipleClicked(wParam, lParam);
	    break;

	    // Options

	case OPTIONS_ID:
	    DisplayOptions(wParam, lParam);
	    break;

	    // Quit

	case QUIT_ID:
	    mixerClose(mixer.hmx);
	    waveInStop(audio.hwi);
	    waveInClose(audio.hwi);
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

	// Audio input data

    case MM_WIM_DATA:
	WaveInData(wParam, lParam);
	break;

	// Mixer control change

    case MM_MIXM_CONTROL_CHANGE:
	VolumeChange(wParam, lParam);
	break;

	// Notify

    case WM_NOTIFY:
	switch (((LPNMHDR)lParam)->code)
	{
	    // Tooltip

	case TTN_SHOW:
	    TooltipShow(wParam, lParam);
	    break;

	case TTN_POP:
	    TooltipPop(wParam, lParam);
	    break;
	}
	break;

        // Process other messages.

    case WM_DESTROY:
	mixerClose(mixer.hmx);
	waveInStop(audio.hwi);
	waveInClose(audio.hwi);
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

    // Resize child windows

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

    // Get window rect and convert to client coordinates

    GetWindowRect(hwnd, &rect.r);
    ScreenToClient(window.hwnd, &rect.p[0]);
    ScreenToClient(window.hwnd, &rect.p[1]);

    // Calculate dimensions

    int width = rect.r.right - rect.r.left;
    int height = rect.r.bottom - rect.r.top;

    // Resize each window

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

    // Change transform

    if (window.zoom)
    {
	XFORM xform =
	    {2.0, 0.0, 0.0, 2.0, 0, 0};

	SetWorldTransform(hdc, &xform);

	rect.right /= 2;
	rect.bottom /= 2;
    }

    switch (wParam)
    {
	// Scope

    case SCOPE_ID:
	return DrawScope(hdc, rect);
	break;

	// Spectrum

    case SPECTRUM_ID:
	return DrawSpectrum(hdc, rect);
	break;

	// Strobe

    case STROBE_ID:
	return DrawStrobe(hdc, rect);
	break;

	// Display

    case DISPLAY_ID:
	return DrawDisplay(hdc, rect);
	break;

	// Meter

    case METER_ID:
	return DrawMeter(hdc, rect);
	break;
    }
}

// Display context menu

BOOL DisplayContextMenu(HWND hWnd, POINTS points)
{
    HMENU menu;
    POINT point;

    // Convert coordinates

    POINTSTOPOINT(point, points);
    ClientToScreen(hWnd, &point);

    // Create menu

    menu = CreatePopupMenu();

    // Add menu items

    AppendMenu(menu, spectrum.zoom? MF_STRING | MF_CHECKED:
	       MF_STRING, ZOOM_ID, "Zoom spectrum");
    AppendMenu(menu, strobe.enable? MF_STRING | MF_CHECKED:
	       MF_STRING, ENABLE_ID, "Display strobe");
    AppendMenu(menu, audio.filter? MF_STRING | MF_CHECKED:
	       MF_STRING, FILTER_ID, "Audio filter");
    AppendMenu(menu, audio.downsample? MF_STRING | MF_CHECKED:
	       MF_STRING, DOWN_ID, "Downsample");
    AppendMenu(menu, display.lock? MF_STRING | MF_CHECKED:
	       MF_STRING, LOCK_ID, "Lock display");
    AppendMenu(menu, window.zoom? MF_STRING | MF_CHECKED:
	       MF_STRING, RESIZE_ID, "Resize display");
    AppendMenu(menu, display.multiple? MF_STRING | MF_CHECKED:
	       MF_STRING, MULTIPLE_ID, "Multiple notes");
    AppendMenu(menu, MF_SEPARATOR, 0, 0);
    AppendMenu(menu, MF_STRING, OPTIONS_ID, "Options...");
    AppendMenu(menu, MF_SEPARATOR, 0, 0);
    AppendMenu(menu, MF_STRING, QUIT_ID, "Quit");

    // Pop up the menu

    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		   point.x, point.y,
		   0, hWnd, NULL);
}

// Display options

BOOL DisplayOptions(WPARAM wParam, LPARAM lParam)
{
    // Check if exists

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

	// Get the main window rect

	RECT rWnd;

	GetWindowRect(window.hwnd, &rWnd);

	// Create the window, offset right

	options.hwnd =
	    CreateWindow(PCLASS, "Tuner Options",
			 WS_VISIBLE | WS_POPUP |
			 WS_CAPTION,
			 rWnd.right + 10, rWnd.top,
			 WIDTH, 320, window.hwnd,
			 (HMENU)NULL, hInst, NULL);
    }

    // Show existing window

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
    static TCHAR s[64];

    // Get the client rect

    GetClientRect(hWnd, &cRect);
    int width = cRect.right;

    // Switch on message

    switch (uMsg)
    {
    case WM_CREATE:

	// Create group box

	group.hwnd =
	    CreateWindow(WC_BUTTON, NULL,
			 WS_VISIBLE | WS_CHILD |
			 BS_GROUPBOX,
			 10, 2, width - 20, 156,
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
	tooltip.info.lpszText = (LPSTR)"Zoom spectrum, "
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
	tooltip.info.lpszText = (LPSTR)"Display strobe, "
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
	tooltip.info.lpszText = (LPSTR)"Audio filter, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create downsample tickbox

	down.hwnd =
	    CreateWindow(WC_BUTTON, "Downsample:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 width / 2 + 10, 54, 124, 24,
			 hWnd, (HMENU)DOWN_ID, hInst, NULL);

	SendMessage(down.hwnd, BM_SETCHECK,
		    audio.downsample? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip

	tooltip.info.uId = (UINT_PTR)lock.hwnd;
	tooltip.info.lpszText = (LPSTR)"Downsample, "
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
	tooltip.info.lpszText = (LPSTR)"Resize display, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create lock tickbox

	lock.hwnd =
	    CreateWindow(WC_BUTTON, "Lock display:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 width / 2 + 10, 88, 124, 24,
			 hWnd, (HMENU)LOCK_ID, hInst, NULL);

	SendMessage(lock.hwnd, BM_SETCHECK,
		    display.lock? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip

	tooltip.info.uId = (UINT_PTR)lock.hwnd;
	tooltip.info.lpszText = (LPSTR)"Lock display, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create text

	text.hwnd =
	    CreateWindow(WC_STATIC, "Ref:",
			 WS_VISIBLE | WS_CHILD |
			 SS_LEFT,
			 20, 126, 32, 20, hWnd,
			 (HMENU)TEXT_ID, hInst, NULL);

	// Create edit control

	sprintf(s, "%6.2lf", audio.reference);

	legend.reference.hwnd =
	    CreateWindow(WC_EDIT, s,
			 WS_VISIBLE | WS_CHILD |
			 WS_BORDER,
			 62, 124, 82, 20, hWnd,
			 (HMENU)REFERENCE_ID, hInst, NULL);

	// Add edit to tooltip

	tooltip.info.uId = (UINT_PTR)legend.reference.hwnd;
	tooltip.info.lpszText = (LPSTR)"Reference, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create up-down control

	reference.hwnd =
	    CreateWindow(UPDOWN_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 UDS_AUTOBUDDY | UDS_ALIGNRIGHT,
			 0, 0, 0, 0, hWnd,
			 (HMENU)REFERENCE_ID, hInst, NULL);

	SendMessage(reference.hwnd, UDM_SETRANGE32, MIN_REF, MAX_REF);
	SendMessage(reference.hwnd, UDM_SETPOS32, 0, audio.reference * 10);

	// Add updown to tooltip

	tooltip.info.uId = (UINT_PTR)reference.hwnd;
	tooltip.info.lpszText = (LPSTR)"Reference, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create multiple tickbox

	multiple.hwnd =
	    CreateWindow(WC_BUTTON, "Multiple notes:",
			 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
			 BS_CHECKBOX,
			 width / 2 + 10, 122, 124, 24,
			 hWnd, (HMENU)MULTIPLE_ID, hInst, NULL);

	SendMessage(multiple.hwnd, BM_SETCHECK,
		    display.multiple? BST_CHECKED: BST_UNCHECKED, 0);

	// Add tickbox to tooltip

	tooltip.info.uId = (UINT_PTR)multiple.hwnd;
	tooltip.info.lpszText = (LPSTR)"Display multiple notes, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create group box

	group.hwnd =
	    CreateWindow(WC_BUTTON, NULL,
			 WS_VISIBLE | WS_CHILD |
			 BS_GROUPBOX,
			 10, 160, width - 20, 124,
			 hWnd, NULL, hInst, NULL);

	// Create text

	text.hwnd =
	    CreateWindow(WC_STATIC, "Correction:",
			 WS_VISIBLE | WS_CHILD |
			 SS_LEFT,
			 20, 182, 76, 20,
			 hWnd, (HMENU)TEXT_ID, hInst, NULL);

	// Create edit control

	sprintf(s, "%6.5lf", audio.correction);

	legend.correction.hwnd =
	    CreateWindow(WC_EDIT, s,
			 WS_VISIBLE | WS_CHILD |
			 WS_BORDER,
			 100, 180, 82, 20, hWnd,
			 (HMENU)CORRECTION_ID, hInst, NULL);

	// Add edit to tooltip

	tooltip.info.uId = (UINT_PTR)legend.correction.hwnd;
	tooltip.info.lpszText = (LPSTR)"Correction, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create up-down control

	correction.hwnd =
	    CreateWindow(UPDOWN_CLASS, NULL,
			 WS_VISIBLE | WS_CHILD |
			 UDS_AUTOBUDDY | UDS_ALIGNRIGHT,
			 0, 0, 0, 0, hWnd,
			 (HMENU)CORRECTION_ID, hInst, NULL);

	SendMessage(correction.hwnd, UDM_SETRANGE32,
		    MIN_CORRECTION, MAX_CORRECTION);
	SendMessage(correction.hwnd, UDM_SETPOS32, 0,
		    audio.correction * 100000);

	// Add updown to tooltip

	tooltip.info.uId = (UINT_PTR)correction.hwnd;
	tooltip.info.lpszText = (LPSTR)"Correction, "
	    "click to change";

	SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
		    (LPARAM) &tooltip.info);

	// Create save button

	button.save.hwnd =
	    CreateWindow(WC_BUTTON, "Save",
			 WS_VISIBLE | WS_CHILD |
			 BS_PUSHBUTTON,
			 209, 177, 85, 26,
			 hWnd, (HMENU)SAVE_ID, hInst, NULL);

	// Create text

	text.hwnd =
	    CreateWindow(WC_STATIC,
			 "Use correction if your sound card clock "
			 "is significantly inaccurate.",
			 WS_VISIBLE | WS_CHILD |
			 SS_LEFT,
			 20, 210, width - 40, 40,
			 hWnd, (HMENU)TEXT_ID, hInst, NULL);

	// Create text

	sprintf(s, "Sample rate:  %6.1lf",
		(double)SAMPLE_RATE / audio.correction);

	legend.sample.hwnd =
	    CreateWindow(WC_STATIC, s,
			 WS_VISIBLE | WS_CHILD |
			 SS_LEFT,
			 20, 252, 152, 20,
			 hWnd, (HMENU)TEXT_ID, hInst, NULL);

	// Create close button

	button.close.hwnd =
	    CreateWindow(WC_BUTTON, "Close",
			 WS_VISIBLE | WS_CHILD |
			 BS_PUSHBUTTON,
			 209, 247, 85, 26,
			 hWnd, (HMENU)CLOSE_ID, hInst, NULL);

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
	switch ((UINT)GetMenu((HWND)lParam))
	{
	case REFERENCE_ID:
	    ChangeReference(wParam, lParam);
	    break;

	case CORRECTION_ID:
	    ChangeCorrection(wParam, lParam);
	    break;
	}

	// Set the focus back to the window

	SetFocus(hWnd);
	break;

	// Set the focus back to the window by clicking

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
	SetFocus(hWnd);
	break;

	// Display options menu

    case WM_RBUTTONDOWN:
	DisplayOptionsMenu(hWnd, MAKEPOINTS(lParam));
	break;

	// Commands

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

	    // Downsample

	case DOWN_ID:
	    DownClicked(wParam, lParam);

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

	    // Reference

	case REFERENCE_ID:
	    EditReference(wParam, lParam);
	    break;

	    // Correction

	case CORRECTION_ID:
	    EditCorrection(wParam, lParam);
	    break;

	    // Close

	case CLOSE_ID:
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
	break;

    default:
	return FALSE;
    }

    if (filter.hwnd != NULL)
	SendMessage(filter.hwnd, BM_SETCHECK,
		    audio.filter? BST_CHECKED: BST_UNCHECKED, 0);

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
	static TCHAR s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);

	return FALSE;
    }

    return TRUE;
}

// Colour clicked

BOOL ColourClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	if (strobe.enable)
	{
	    strobe.colours++;

	    if (strobe.colours > MAGENTA)
		strobe.colours = 0;

	    strobe.changed = TRUE;
	}

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
	RegSetValueEx(hkey, "Colours", 0, REG_DWORD,
		      (LPBYTE)&strobe.colours, sizeof(strobe.colours));

	RegCloseKey(hkey);
    }

    else
    {
	static TCHAR s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);

	return FALSE;
    }

    return TRUE;
}

// Down clicked

BOOL DownClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	audio.downsample = !audio.downsample;
	break;

    default:
	return FALSE;
    }

    if (down.hwnd != NULL)
	SendMessage(down.hwnd, BM_SETCHECK,
		    audio.downsample? BST_CHECKED: BST_UNCHECKED, 0);
    return TRUE;
}

// Expand clicked

BOOL ExpandClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	if (spectrum.expand < 16)
	    spectrum.expand *= 2;

	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Contract clicked

BOOL ContractClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	if (spectrum.expand > 1)
	    spectrum.expand /= 2;

	break;

    default:
	return FALSE;
    }

    return TRUE;
}

// Lock clicked

BOOL LockClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	display.lock = !display.lock;
	break;

    default:
	return FALSE;
    }

    InvalidateRgn(display.hwnd, NULL, TRUE);

    if (lock.hwnd != NULL)
	SendMessage(lock.hwnd, BM_SETCHECK,
		    display.lock? BST_CHECKED: BST_UNCHECKED, 0);
    return TRUE;
}

// Resize clicked

BOOL ResizeClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	window.zoom = !window.zoom;
	break;

    default:
	return FALSE;
    }

    if (resize.hwnd != NULL)
	SendMessage(resize.hwnd, BM_SETCHECK,
		    window.zoom? BST_CHECKED: BST_UNCHECKED, 0);

    ResizeWindow(wParam, lParam);

    return TRUE;
}

// Multiple clicked

BOOL MultipleClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	display.multiple = !display.multiple;
	break;

    default:
	return FALSE;
    }

    if (multiple.hwnd != NULL)
	SendMessage(multiple.hwnd, BM_SETCHECK,
		    display.multiple? BST_CHECKED: BST_UNCHECKED, 0);

    InvalidateRgn(display.hwnd, NULL, TRUE);

    return TRUE;
}

// Edit correction

BOOL EditCorrection(WPARAM wParam, LPARAM lParam)
{
    static TCHAR s[64];

    switch (HIWORD(wParam))
    {
    case EN_KILLFOCUS:
	GetWindowText(legend.correction.hwnd, s, sizeof(s));
	audio.correction = atof(s);

	SendMessage(correction.hwnd, UDM_SETPOS32, 0,
		    audio.correction * 100000);

	sprintf(s, "Sample rate:  %6.1lf",
		(double)SAMPLE_RATE / audio.correction);
	SetWindowText(legend.sample.hwnd, s);

	UpdateStatusBar();
	break;
    }
}

// Change correction

BOOL ChangeCorrection(WPARAM wParam, LPARAM lParam)
{
    static TCHAR s[64];

    long value = SendMessage(correction.hwnd, UDM_GETPOS32, 0, 0);
    audio.correction = (double)value / 100000.0;

    sprintf(s, "%6.5lf", audio.correction);
    SetWindowText(legend.correction.hwnd, s);

    sprintf(s, "Sample rate:  %6.1lf",
	    (double)SAMPLE_RATE / audio.correction);
    SetWindowText(legend.sample.hwnd, s);

    UpdateStatusBar();
}

// Update status bar

VOID UpdateStatusBar()
{
    static TCHAR s[64];

    sprintf(s, " Reference: %5.2lfHz\t\tCorrection: %6.5lf ",
	    audio.reference, audio.correction);
    SetWindowText(status.hwnd, s);
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
	static TCHAR s[64];

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
    AppendMenu(menu, MF_STRING, CLOSE_ID, "Close");

    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		   point.x, point.y,
		   0, hWnd, NULL);
}

// Char pressed

BOOL CharPressed(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
	// Copy display

    case 'C':
    case 'c':
    case 0x3:
	CopyDisplay(wParam, lParam);
	break;

	// Downsample

    case 'D':
    case 'd':
	DownClicked(wParam, lParam);
	break;

	// Filter

    case 'F':
    case 'f':
	FilterClicked(wParam, lParam);
	break;

	// Colour

    case 'K':
    case 'k':
	ColourClicked(wParam, lParam);
	break;

	// Lock

    case 'L':
    case 'l':
	LockClicked(wParam, lParam);
	break;

	// Options

    case 'O':
    case 'o':
	DisplayOptions(wParam, lParam);
	break;

	// Resize

    case 'R':
    case 'r':
	ResizeClicked(wParam, lParam);
	break;

	// Strobe

    case 'S':
    case 's':
	EnableClicked(wParam, lParam);
	break;

	// Multiple

    case 'M':
    case 'm':
	MultipleClicked(wParam, lParam);
	break;

	// Zoom

    case 'Z':
    case 'z':
	ZoomClicked(wParam, lParam);
	break;

	// Expand

    case '+':
	ExpandClicked(wParam, lParam);
	break;

	// Contract

    case '-':
	ContractClicked(wParam, lParam);
	break;
    }
}

// Copy display

BOOL CopyDisplay(WPARAM wParam, LPARAM lParam)
{
    // Memory size

    enum
    {MEM_SIZE = 1024};

    static char s[64];

    static const char *notes[] =
	{"C", "C#", "D", "Eb", "E", "F",
	 "F#", "G", "Ab", "A", "Bb", "B"};

    // Open clipboard

    if (!OpenClipboard(window.hwnd))
	return FALSE;

    // Empty clipboard

    EmptyClipboard(); 

    // Allocate memory

    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, MEM_SIZE);

    if (mem == NULL)
    {
	CloseClipboard();
	return FALSE;
    }

    // Lock the memory

    LPTSTR text = (LPSTR)GlobalLock(mem);

    // Check if multiple

    if (display.multiple && display.count > 0)
    {
	// For each set of values

	for (int i = 0; i < display.count; i++)
	{
	    double f = display.maxima[i].f;

	    // Reference freq

	    double fr = display.maxima[i].fr;

	    int n = display.maxima[i].n;

	    if (n < 0)
		n = 0;

	    double c = -12.0 * log2(fr / f);

	    // Ignore silly values

	    if (!isfinite(c))
		continue;

	    // Print the text

	    sprintf(s, "%s%d\t%+6.2lf\t%9.2lf\t%9.2lf\t%+8.2lf\r\n",
		    notes[n % Length(notes)], n / 12,
		    c * 100.0, fr, f, f - fr);

	    // Copy to the memory

	    if (i == 0)
		strcpy(text, s);

	    else
		strcat(text, s);
	}
    }

    else
    {
	// Print the values

	sprintf(s, "%s%d\t%+6.2lf\t%9.2lf\t%9.2lf\t%+8.2lf\r\n",
		notes[display.n % Length(notes)], display.n / 12,
		display.c * 100.0, display.fr, display.f,
		display.f - display.fr);

	// Copy to the memory

	strcpy(text, s);
    }

    // Place in clipboard

    GlobalUnlock(text);
    SetClipboardData(CF_TEXT, mem);
    CloseClipboard(); 
 
    return TRUE;
}

// Meter callback

VOID CALLBACK MeterCallback(PVOID lpParam, BOOL TimerFired)
{
    METERP meter = (METERP)lpParam;

    static float mc;

    // Do calculation

    mc = ((mc * 7.0) + meter->c) / 8.0;

    int value = round(mc * MAX_METER) + REF_METER;

    // Update meter

    SendMessage(meter->slider.hwnd, TBM_SETPOS, TRUE, value);
}

// Strobe callback

VOID CALLBACK StrobeCallback(PVOID lpParam, BOOL TimerFired)
{
    STROBEP strobe = (STROBEP)lpParam;

    // Update strobe

    if (strobe->enable)
    	InvalidateRgn(strobe->hwnd, NULL, TRUE);
}

// Draw scope

BOOL DrawScope(HDC hdc, RECT rect)
{
    static HBITMAP bitmap;
    static HFONT font;
    static HDC hbdc;

    enum
    {FONT_HEIGHT   = 10};

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

    // Create font

    if (font == NULL)
    {
	lf.lfHeight = FONT_HEIGHT;
	font = CreateFontIndirect(&lf);
    }

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

	// Select font

	SelectObject(hbdc, font);
	SetTextAlign(hbdc, TA_LEFT | TA_BOTTOM);
	SetBkMode(hbdc, TRANSPARENT);
    }

    // Erase background

    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(BLACK_BRUSH));

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
	if (maxdx < dx)
	{
	    maxdx = dx;
	    n = i;
	}

	if (maxdx > 0 && dx < 0)
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

	int y = -scope.data[n + i] / yscale;
	LineTo(hbdc, i, y);
    }

    // Move the origin back

    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Show F if filtered

    if (audio.filter)
    {
	// Yellow text

	SetTextColor(hbdc, RGB(255, 255, 0));
	TextOut(hbdc, 0, height + 1, "F", 1);
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
    static HFONT font;
    static HDC hbdc;

    enum
    {FONT_HEIGHT   = 10};

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

    static TCHAR s[16];

    // Create font

    if (font == NULL)
    {
	lf.lfHeight = FONT_HEIGHT;
	font = CreateFontIndirect(&lf);
    }

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

	// Select font

	SelectObject(hbdc, font);
	SetBkMode(hbdc, TRANSPARENT);
    }

    // Erase background

    RECT brct =
	{0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(BLACK_BRUSH));

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

    // Move the origin

    SetViewportOrgEx(hbdc, 0, height - 1, NULL);

    // Green pen for spectrum trace

    SetDCPenColor(hbdc, RGB(0, 255, 0));

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
	// Calculate scale

	float xscale = ((float)width / (spectrum.r - spectrum.l)) / 2.0;

	for (int i = floor(spectrum.l); i <= ceil(spectrum.h); i++)
	{
	    if (i > 0 && i < spectrum.length)
	    {
		float value = spectrum.data[i];

		if (max < value)
		    max = value;

		int y = -round(value * yscale);
		int x = round(((float)i - spectrum.l) * xscale); 

		LineTo(hbdc, x, y);
	    }
	}

	MoveToEx(hbdc, width / 2, 0, NULL);
	LineTo(hbdc, width / 2, -height);

	// Yellow pen for frequency trace

	SetDCPenColor(hbdc, RGB(255, 255, 0));
	SetTextColor(hbdc, RGB(255, 255, 0));
	SetTextAlign(hbdc, TA_CENTER | TA_BOTTOM);

	// Draw lines for each frequency

	for (int i = 0; i < spectrum.count; i++)
	{
	    // Draw line for each that are in range

	    if (spectrum.values[i].f > spectrum.l &&
		spectrum.values[i].f < spectrum.h)
	    {
		int x = round((spectrum.values[i].f - spectrum.l) * xscale);
		MoveToEx(hbdc, x, 0, NULL);
		LineTo(hbdc, x, -height);

		double f = display.maxima[i].f;

		// Reference freq

		double fr = display.maxima[i].fr;
		double c = -12.0 * log2(fr / f);

		// Ignore silly values

		if (!isfinite(c))
		    continue;

		sprintf(s, "%+0.0f", c * 100.0);
		TextOut(hbdc, x, 2, s, strlen(s));
	    }
	}
    }

    else
    {
	float xscale = log((float)spectrum.length /
			   (float)spectrum.expand) / width;

	int last = 1;
	for (int x = 0; x < width; x++)
	{
	    float value = 0.0;

	    int index = (int)round(pow(M_E, x * xscale));
	    for (int i = last; i <= index; i++)
	    {
		// Don't show DC component

		if (i > 0 && i < spectrum.length)
		{
		    if (value < spectrum.data[i])
			value = spectrum.data[i];
		}
	    }

	    // Update last index

	    last = index + 1;

	    if (max < value)
		max = value;

	    int y = -round(value * yscale);

	    LineTo(hbdc, x, y);
	}

	// Yellow pen for frequency trace

	SetDCPenColor(hbdc, RGB(255, 255, 0));
	SetTextColor(hbdc, RGB(255, 255, 0));
	SetTextAlign(hbdc, TA_CENTER | TA_BOTTOM);

	// Draw lines for each frequency

	for (int i = 0; i < spectrum.count; i++)
	{
	    // Draw line for each

	    int x = round(log(spectrum.values[i].f) / xscale);
	    MoveToEx(hbdc, x, 0, NULL);
	    LineTo(hbdc, x, -height);

	    double f = display.maxima[i].f;

	    // Reference freq

	    double fr = display.maxima[i].fr;
	    double c = -12.0 * log2(fr / f);

	    // Ignore silly values

	    if (!isfinite(c))
		continue;

	    sprintf(s, "%+0.0f", c * 100.0);
	    TextOut(hbdc, x, 2, s, strlen(s));
	}

	SetTextAlign(hbdc, TA_LEFT | TA_BOTTOM);

	if (spectrum.expand > 1)
	{
	    sprintf(s, "x%d", spectrum.expand);
	    TextOut(hbdc, 0, 2, s, strlen(s));
	}
    }

    // D for downsample

    if (audio.downsample)
    {
	SetTextAlign(hbdc, TA_LEFT | TA_BOTTOM);
	TextOut(hbdc, 0, 10 - height, "D", 1);
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
    static HBITMAP bitmap;
    static HFONT musica;
    static HFONT medium;
    static HFONT larger;
    static HFONT large;
    static HFONT font;
    static HDC hbdc;

    enum
    {FONT_HEIGHT   = 16,
     MUSIC_HEIGHT  = 36,
     LARGE_HEIGHT  = 42,
     LARGER_HEIGHT = 56,
     MEDIUM_HEIGHT = 28};

    static const TCHAR *notes[] =
	{"C", "C", "D", "E", "E", "F",
	 "F", "G", "A", "A", "B", "B"};

    static const TCHAR *sharps[] =
	{"", "#", "", "b", "", "",
	 "#", "", "b", "", "b", ""};

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

    static TCHAR s[64];

    // Create fonts

    if (font == NULL)
    {
	lf.lfHeight = FONT_HEIGHT;
	font = CreateFontIndirect(&lf);

	lf.lfHeight = LARGE_HEIGHT;
	large = CreateFontIndirect(&lf);

	lf.lfHeight = LARGER_HEIGHT;
	larger = CreateFontIndirect(&lf);

	lf.lfHeight = MEDIUM_HEIGHT;
	medium = CreateFontIndirect(&lf);

	AddFontResourceEx("Musica.ttf", FR_PRIVATE, 0);
	lf.lfHeight = MUSIC_HEIGHT;
	// lf.lfWeight = FW_REGULAR;
	strcat(lf.lfFaceName, "Musica");

	musica = CreateFontIndirect(&lf);
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
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(WHITE_BRUSH));

    if (display.multiple)
    {
	// Select font

	SelectObject(hbdc, font);

	// Set text align

	SetTextAlign(hbdc, TA_TOP);

	if (display.count == 0)
	{
	    // Display note

	    sprintf(s, "%s%s%d", notes[display.n % Length(notes)],
		    sharps[display.n % Length(notes)], display.n / 12);
	    TextOut(hbdc, 8, 0, s, strlen(s));

	    // Display cents

	    sprintf(s, "%+4.2lf¢", display.c * 100.0);
	    TextOut(hbdc, 36, 0, s, strlen(s));

	    // Display reference

	    sprintf(s, "%4.2lfHz", display.fr);
	    TextOut(hbdc, 90, 0, s, strlen(s));

	    // Display frequency

	    sprintf(s, "%4.2lfHz", display.f);
	    TextOut(hbdc, 162, 0, s, strlen(s));

	    // Display difference

	    sprintf(s, "%+4.2lfHz", display.f - display.fr);
	    TextOut(hbdc, 234, 0, s, strlen(s));
	}

	for (int i = 0; i < display.count; i++)
	{
	    double f = display.maxima[i].f;

	    // Reference freq

	    double fr = display.maxima[i].fr;

	    int n = display.maxima[i].n;

	    if (n < 0)
		n = 0;

	    double c = -12.0 * log2(fr / f);

	    // Ignore silly values

	    if (!isfinite(c))
		continue;

	    // Display note

	    sprintf(s, "%s%s%d", notes[n % Length(notes)],
		    sharps[n % Length(notes)], n / 12);
	    TextOut(hbdc, 8, i * FONT_HEIGHT, s, strlen(s));

	    // Display cents

	    sprintf(s, "%+4.2lf¢", c * 100.0);
	    TextOut(hbdc, 36, i * FONT_HEIGHT, s, strlen(s));

	    // Display reference

	    sprintf(s, "%4.2lfHz", fr);
	    TextOut(hbdc, 90, i * FONT_HEIGHT, s, strlen(s));

	    // Display frequency

	    sprintf(s, "%4.2lfHz", f);
	    TextOut(hbdc, 162, i * FONT_HEIGHT, s, strlen(s));

	    // Display difference

	    sprintf(s, "%+4.2lfHz", f - fr);
	    TextOut(hbdc, 234, i * FONT_HEIGHT, s, strlen(s));

	    if (i == 5)
		break;
	}
    }

    else
    {
	// Select larger font

	SelectObject(hbdc, larger);

	// Display coordinates

	int y = 42;

	// Text size

	SIZE size = {0};

	// Set text align

	SetTextAlign(hbdc, TA_BASELINE|TA_LEFT);
	SetBkMode(hbdc, TRANSPARENT);

	// Display note

	sprintf(s, "%s", notes[display.n % Length(notes)]);
	TextOut(hbdc, 8, y, s, strlen(s));

	GetTextExtentPoint32(hbdc, s, strlen(s), &size);
	int x = size.cx + 8;

	// Select medium font

	SelectObject(hbdc, medium);

	sprintf(s, "%d", display.n / 12);
	TextOut(hbdc, x, y, s, strlen(s));

	// Select musica font

	SelectObject(hbdc, musica);

	sprintf(s, "%s", sharps[display.n % Length(sharps)]);
	TextOut(hbdc, x + 8, y - 20, s, strlen(s));

	// Select large font

	SelectObject(hbdc, large);
	SetTextAlign(hbdc, TA_BASELINE|TA_RIGHT);

	// Display cents

	sprintf(s, "%+4.2f¢", display.c * 100.0);
	TextOut(hbdc, width - 8, y, s, strlen(s));

	y += MEDIUM_HEIGHT;

	// Select medium font

	SelectObject(hbdc, medium);
	SetTextAlign(hbdc, TA_BASELINE|TA_LEFT);

	// Display reference frequency

	sprintf(s, "%4.2fHz", display.fr);
	TextOut(hbdc, 8, y, s, strlen(s));

	// Display actual frequency

	SetTextAlign(hbdc, TA_BASELINE|TA_RIGHT);
	sprintf(s, "%4.2fHz", display.f);
	TextOut(hbdc, width - 8, y, s, strlen(s));

	y += 24;

	// Display reference

	SetTextAlign(hbdc, TA_BASELINE|TA_LEFT);
	sprintf(s, "%4.2fHz", audio.reference);
	TextOut(hbdc, 8, y, s, strlen(s));

	// Display frequency difference

	SetTextAlign(hbdc, TA_BASELINE|TA_RIGHT);
	sprintf(s, "%+4.2lfHz", display.f - display.fr);
	TextOut(hbdc, width - 8, y, s, strlen(s));
    }

    // Show lock

    if (display.lock)
	DrawLock(hbdc, -1, height + 1);

    // Copy the bitmap

    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return TRUE;
}

// Draw lock

BOOL DrawLock(HDC hdc, int x, int y)
{
    POINT point;
    POINT body[] =
	{{2, -3}, {8, -3}, {8, -8}, {2, -8}, {2, -3}};

    SetViewportOrgEx(hdc, x, y, &point);

    Polyline(hdc, body, Length(body));

    MoveToEx(hdc, 3, -8, NULL);
    LineTo(hdc, 3, -11);

    MoveToEx(hdc, 7, -8, NULL);
    LineTo(hdc, 7, -11);

    MoveToEx(hdc, 4, -11, NULL);
    LineTo(hdc, 7, -11);

    SetPixel(hdc, 3, -11, RGB(255, 170, 85));
    SetPixel(hdc, 6, -10, RGB(255, 170, 85));

    SetPixel(hdc, 4, -10, RGB(85, 170, 255));
    SetPixel(hdc, 7, -11, RGB(85, 170, 255));

    SetPixel(hdc, 7, -7, RGB(255, 170, 85));
    SetPixel(hdc, 7, -4, RGB(255, 170, 85));

    SetPixel(hdc, 3, -7, RGB(85, 170, 255));
    SetPixel(hdc, 3, -4, RGB(85, 170, 255));

    SetViewportOrgEx(hdc, point.x, point.y, NULL);
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
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // Move origin

    SetViewportOrgEx(hbdc, width / 2, 0, NULL);

    // Draw the meter scale

    for (int i = 0; i < 6; i++)
    {
	int x = width / 11 * i;
	static TCHAR s[16];

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

    // Draw + and - signs

    MoveToEx(hbdc, -(width / 2) + 6, 17, NULL);
    LineTo(hbdc, -(width / 2) + 11, 17);

    MoveToEx(hbdc, (width / 2) - 6, 17, NULL);
    LineTo(hbdc, (width / 2) - 11, 17);
    MoveToEx(hbdc, (width / 2) - 8, 15, NULL);
    LineTo(hbdc, (width / 2) - 8, 20);

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

    static HBRUSH sbrush;
    static HBRUSH sshade;
    static HBRUSH mbrush;
    static HBRUSH mshade;
    static HBRUSH lbrush;
    static HBRUSH lshade;
    static HBRUSH ebrush;

    // Colours

    static int colours[][2] =
	{{RGB(63, 63, 255), RGB(63, 255, 255)},
	 {RGB(111, 111, 0), RGB(191, 255, 191)},
	 {RGB(255, 63, 255), RGB(255, 255, 63)}};

    // Draw nice etched edge

    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate dimensions

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create brushes

    int foreground = colours[strobe.colours][0];
    int background = colours[strobe.colours][1];

    if (sbrush == NULL || strobe.changed)
    {
	if (sbrush != NULL)
	    DeleteObject(sbrush);

	if (sshade != NULL)
	    DeleteObject(sshade);

	if (mbrush != NULL)
	    DeleteObject(mbrush);

	if (mshade != NULL)
	    DeleteObject(mshade);

	if (lbrush != NULL)
	    DeleteObject(lbrush);

	if (lshade != NULL)
	    DeleteObject(lshade);

	if (ebrush != NULL)
	    DeleteObject(ebrush);

	HDC hbdc = CreateCompatibleDC(hdc);

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, 20, 10));
	SelectObject(hbdc, GetStockObject(DC_PEN));
	SelectObject(hbdc, GetStockObject(DC_BRUSH));

	SetDCPenColor(hbdc, foreground);
	SetDCBrushColor(hbdc, foreground);
	Rectangle(hbdc, 0, 0, 10, 10);
	SetDCPenColor(hbdc, background);
	SetDCBrushColor(hbdc, background);
	Rectangle(hbdc, 10, 0, 20, 10);

	sbrush = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, 20, 10));

	TRIVERTEX vertex[] =
	    {{0, 0,
	      GetRValue(foreground) << 8,
	      GetGValue(foreground) << 8,
	      GetBValue(foreground) << 8,
	      0},
	     {10, 10,
	      GetRValue(background) << 8,
	      GetGValue(background) << 8,
	      GetBValue(background) << 8,
	      0},
	     {20, 0,
	      GetRValue(foreground) << 8,
	      GetGValue(foreground) << 8,
	      GetBValue(foreground) << 8,
	      0}};

	GRADIENT_RECT gradient[] =
	    {{0, 1}, {1, 2}};

	GradientFill(hbdc, vertex, Length(vertex),
		     gradient, Length(gradient), GRADIENT_FILL_RECT_H);

	sshade = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, 40, 10));

	SetDCPenColor(hbdc, foreground);
	SetDCBrushColor(hbdc, foreground);
	Rectangle(hbdc, 0, 0, 20, 10);
	SetDCPenColor(hbdc, background);
	SetDCBrushColor(hbdc, background);
	Rectangle(hbdc, 20, 0, 40, 10);

	mbrush = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, 40, 10));

	vertex[1].x = 20;
	vertex[2].x = 40;

	GradientFill(hbdc, vertex, 3, gradient, 2, GRADIENT_FILL_RECT_H);

	mshade = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, 80, 10));

	SetDCPenColor(hbdc, foreground);
	SetDCBrushColor(hbdc, foreground);
	Rectangle(hbdc, 0, 0, 40, 10);
	SetDCPenColor(hbdc, background);
	SetDCBrushColor(hbdc, background);
	Rectangle(hbdc, 40, 0, 80, 10);

	lbrush = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, 80, 10));

	vertex[1].x = 40;
	vertex[2].x = 80;

	GradientFill(hbdc, vertex, 3, gradient, 2, GRADIENT_FILL_RECT_H);

	lshade = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, 160, 10));

	SetDCPenColor(hbdc, foreground);
	SetDCBrushColor(hbdc, foreground);
	Rectangle(hbdc, 0, 0, 80, 10);
	SetDCPenColor(hbdc, background);
	SetDCBrushColor(hbdc, background);
	Rectangle(hbdc, 80, 0, 160, 10);

	ebrush = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	DeleteDC(hbdc);
	strobe.changed = FALSE;
    }

    // Erase background

    FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
    SetViewportOrgEx(hdc, rect.left, rect.top, NULL);

    if (strobe.enable)
    {
    	mc = ((7.0 * mc) + strobe.c) / 8.0;
    	mx += mc * 50.0;

    	if (mx > 160.0)
    	    mx = 0.0;

    	if (mx < 0.0)
    	    mx = 160.0;

    	int rx = round(mx - 160.0);
	SetBrushOrgEx(hdc, rx, 0, NULL);
	SelectObject(hdc, GetStockObject(NULL_PEN));

	if (fabs(mc) > 0.4)
	{
	    SelectObject(hdc, GetStockObject(DC_BRUSH));
	    SetDCBrushColor(hdc, background);
	}

	else if (fabs(mc) > 0.2)
	    SelectObject(hdc, sshade);

	else
	    SelectObject(hdc, sbrush);
	Rectangle(hdc, 0, 0, width, 10);

	if (fabs(mc) > 0.3)
	    SelectObject(hdc, mshade);

	else
	    SelectObject(hdc, mbrush);
	Rectangle(hdc, 0, 10, width, 20);

	if (fabs(mc) > 0.4)
	    SelectObject(hdc, lshade);

	else
	    SelectObject(hdc, lbrush);
	Rectangle(hdc, 0, 20, width, 30);

	SelectObject(hdc, ebrush);
	Rectangle(hdc, 0, 30, width, 40);
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
	static TCHAR s[64];

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

    InvalidateRgn(strobe.hwnd, NULL, TRUE);

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
	static TCHAR s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }

    return TRUE;
}

// Meter clicked

BOOL MeterClicked(WPARAM wParam, LPARAM lParam)
{
    return DisplayClicked(wParam, lParam);
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

// Volume change

BOOL VolumeChange(WPARAM wParam, LPARAM lParam)
{
    if (mixer.pmxc == NULL)
	return FALSE;

    if (lParam == mixer.pmxcd->dwControlID)
    {
	// Get the value

	mixerGetControlDetails((HMIXEROBJ)mixer.hmx, mixer.pmxcd,
			       MIXER_GETCONTROLDETAILSF_VALUE);

	// Set the slider

	int value = MAX_VOL - (mixer.pmxcdu->dwValue * (MAX_VOL - MIN_VOL) /
			       (mixer.pmxc->Bounds.dwMaximum -
				mixer.pmxc->Bounds.dwMinimum));

	SendMessage(volume.hwnd, TBM_SETPOS, TRUE, value);
    }

    return TRUE;
}

// Edit reference

BOOL EditReference(WPARAM wParam, LPARAM lParam)
{
    static TCHAR s[64];

    if (audio.reference == 0)
	return FALSE;

    switch (HIWORD(wParam))
    {
    case EN_KILLFOCUS:
	GetWindowText(legend.reference.hwnd, s, sizeof(s));
	audio.reference = atof(s);

	SendMessage(reference.hwnd, UDM_SETPOS32, 0,
		    audio.reference * 10);
	break;

    default:
	return FALSE;
    }

    InvalidateRgn(display.hwnd, NULL, TRUE);

    UpdateStatusBar();

    HKEY hkey;
    LONG error;

    error = RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\CTuner", 0,
			   NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (error == ERROR_SUCCESS)
    {
	int value = audio.reference * 10;

	RegSetValueEx(hkey, "Reference", 0, REG_DWORD,
		      (LPBYTE)&value, sizeof(value));

	RegCloseKey(hkey);
    }

    else
    {
	static TCHAR s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }

    return TRUE;
}

// Change reference

BOOL ChangeReference(WPARAM wParam, LPARAM lParam)
{
    static TCHAR s[64];

    long value = SendMessage(reference.hwnd, UDM_GETPOS32, 0, 0);
    audio.reference = (double)value / 10.0;

    sprintf(s, "%6.2lf", audio.reference);
    SetWindowText(legend.reference.hwnd, s);

    InvalidateRgn(display.hwnd, NULL, TRUE);

    UpdateStatusBar();

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
	static TCHAR s[64];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error,
		      0, s, sizeof(s), NULL);

	MessageBox(window.hwnd, s, "RegCreateKeyEx", MB_OK | MB_ICONERROR);
    }

    return TRUE;
}

// Tooltip show

VOID TooltipShow(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR)lParam;

    switch (GetDlgCtrlID((HWND)pnmh->idFrom))
    {
    case VOLUME_ID:
	SetWindowText(status.hwnd, " Microphone volume");
	break;

    case SCOPE_ID:
	SetWindowText(status.hwnd, " Scope, click to filter audio");
	break;

    case SPECTRUM_ID:
	SetWindowText(status.hwnd, " Spectrum, click to zoom");
	break;

    case DISPLAY_ID:
	SetWindowText(status.hwnd, " Display, click to lock");
	break;

    case STROBE_ID:
	SetWindowText(status.hwnd, " Strobe, click to disable/enable");
	break;

    case METER_ID:
    case SLIDER_ID:
	SetWindowText(status.hwnd, " Cents, click to lock");
	break;
    }
}

// Tooltip pop

VOID TooltipPop(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR)lParam;

    UpdateStatusBar();
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
	static TCHAR s[64];

	waveInGetErrorText(mmr, s, sizeof(s));
	MessageBox(window.hwnd, s, "WaveInOpen", MB_OK | MB_ICONERROR);
	return mmr;
    }

    // Check for volume control

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

	mmr = mixerOpen(&mixer.hmx, (UINT)audio.hwi, (DWORD_PTR)window.hwnd, 0,
			CALLBACK_WINDOW | MIXER_OBJECTF_HWAVEIN);

	if (mmr != MMSYSERR_NOERROR)
	{
	    EnableWindow(volume.hwnd, FALSE);
	    break;
	}

	// Mixer line types

	DWORD types[] =
	    {MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE,
	     MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY,
	     MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER,
	     MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE,
	     MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER,
	     MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC,
	     MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED,
	     MIXERLINE_COMPONENTTYPE_SRC_DIGITAL,
	     MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT,
	     MIXERLINE_COMPONENTTYPE_SRC_ANALOG,
	     MIXERLINE_COMPONENTTYPE_SRC_LINE};

	// Get mixer line info

	for (int i = 0; i < Length(types); i++)
	{
	    // Try a component type

	    mxl.dwComponentType = types[i];

	    // Get the info

	    mmr = mixerGetLineInfo((HMIXEROBJ)mixer.hmx, &mxl,
				   MIXER_GETLINEINFOF_COMPONENTTYPE);

	    // Try again if error

	    if (mmr != MMSYSERR_NOERROR)
		continue;

	    // Check if line is active

	    if (mxl.fdwLine & MIXERLINE_LINEF_ACTIVE)
	    {
		mixer.pmxl = &mxl;
		break;
	    }
	}

	// No mixer line

	if (mixer.pmxl == NULL)
	{
	    EnableWindow(volume.hwnd, FALSE);
	    break;
	}

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

    for (int i = 0; i < Length(hdrs); i++)
    {
	// Prepare a waveform audio input header

	mmr = waveInPrepareHeader(audio.hwi, &hdrs[i], sizeof(WAVEHDR));
	if (mmr != MMSYSERR_NOERROR)
	{
	    static TCHAR s[64];

	    waveInGetErrorText(mmr, s, sizeof(s));
	    MessageBox(window.hwnd, s, "WaveInPrepareHeader",
		       MB_OK | MB_ICONERROR);
	    return mmr;
	}

	// Add a waveform audio input buffer

	mmr = waveInAddBuffer(audio.hwi, &hdrs[i], sizeof(WAVEHDR));
	if (mmr != MMSYSERR_NOERROR)
	{
	    static TCHAR s[64];

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
	static TCHAR s[64];

	waveInGetErrorText(mmr, s, sizeof(s));
	MessageBox(window.hwnd, s, "WaveInStart", MB_OK | MB_ICONERROR);
	return mmr;
    }

    // Set up reference value

    if (audio.reference == 0)
	audio.reference = A5_REFNCE;

    // Set up correction value

    if (audio.correction == 0)
	audio.correction = 1.0;

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

VOID WaveInData(WPARAM wParam, LPARAM lParam)
{
    enum
    {TIMER_COUNT = 16};

    // Create buffers for processing the audio data

    static double buffer[SAMPLES];
    static complex x[SAMPLES];

    static double xa[RANGE];
    static double xp[RANGE];
    static double xf[RANGE];

    static double x2[RANGE / 2];
    static double x3[RANGE / 3];
    static double x4[RANGE / 4];
    static double x5[RANGE / 5];

    static double dx[RANGE];

    static maximum maxima[MAXIMA];
    static value   values[MAXIMA];

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
	static double G = 3.023332184e+01;
	static double K = 0.9338478249;

	static double xv[2];
	static double yv[2];

	xv[0] = xv[1];
	xv[1] = (double)data[i] / G;

	yv[0] = yv[1];
	yv[1] = (xv[0] + xv[1]) + (K * yv[0]);

	// Choose filtered/unfiltered data

	buffer[(SAMPLES - STEP) + i] =
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

    // Copy data to FFT input arrays for tuner

    for (int i = 0; i < SAMPLES; i++)
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

    // do FFT for tuner

    fftr(x, SAMPLES);

    // Process FFT output for tuner

    for (int i = 1; i < RANGE; i++)
    {
	double real = x[i].r;
	double imag = x[i].i;

	xa[i] = hypot(real, imag);

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

	dx[i] = xa[i] - xa[i - 1];
    }

    // Downsample

    if (audio.downsample)
    {
	// x2 = xa << 2

	for (int i = 0; i < Length(x2); i++)
	{
	    x2[i] = 0.0;

	    for (int j = 0; j < 2; j++)
		x2[i] += xa[(i * 2) + j] / 2.0;
	}

	// x3 = xa << 3

	for (int i = 0; i < Length(x3); i++)
	{
	    x3[i] = 0.0;

	    for (int j = 0; j < 3; j++)
		x3[i] += xa[(i * 3) + j] / 3.0;
	}

	// x4 = xa << 4

	for (int i = 0; i < Length(x4); i++)
	{
	    x4[i] = 0.0;

	    for (int j = 0; j < 4; j++)
		x2[i] += xa[(i * 4) + j] / 4.0;
	}

	// x5 = xa << 5

	for (int i = 0; i < Length(x5); i++)
	{
	    x5[i] = 0.0;

	    for (int j = 0; j < 5; j++)
		x5[i] += xa[(i * 5) + j] / 5.0;
	}

	// Add downsamples

	for (int i = 1; i < Length(xa); i++)
	{
	    if (i < Length(x2))
		xa[i] += x2[i];

	    if (i < Length(x3))
		xa[i] += x3[i];

	    if (i < Length(x4))
		xa[i] += x4[i];

	    if (i < Length(x5))
		xa[i] += x5[i];

	    // Recalculate differences

	    dx[i] = xa[i] - xa[i - 1];
	}
    }

    // Maximum FFT output

    double max = 0.0;
    double f = 0.0;

    int count = 0;
    int limit = RANGE - 1;

    // Find maximum value, and list of maxima

    for (int i = 1; i < limit; i++)
    {
	if (xa[i] > max)
	{
	    max = xa[i];
	    f = xf[i];
	}

	// If display not locked, find maxima and add to list

	if (!display.lock && count < Length(maxima) &&
	    xa[i] > MIN && xa[i] > (max / 4.0) &&
	    dx[i] > 0.0 && dx[i + 1] < 0.0)
	{
	    maxima[count].f = xf[i];

	    // Cents relative to reference

	    double cf =
	    	-12.0 * log2(audio.reference / xf[i]);

	    // Reference note

	    maxima[count].fr = audio.reference * pow(2.0, round(cf) / 12.0);

	    // Note number

	    maxima[count].n = round(cf) + C5_OFFSET;

	    // Set limit to octave above

	    if (!audio.downsample && (limit > i * 2))
		limit = i * 2 - 1;

	    count++;
	}
    }

    // Reference note frequency and lower and upper limits

    double fr = 0.0;
    double fl = 0.0;
    double fh = 0.0;

    // Note number

    int n = 0;

    // Found flag and cents value

    BOOL found = FALSE;
    double c = 0.0;

    // Do the note and cents calculations

    if (max > MIN)
    {
	found = TRUE;

	// Frequency

	if (!audio.downsample)
	    f = maxima[0].f;

	// Cents relative to reference

	double cf =
	    -12.0 * log2(audio.reference / f);

	// Reference note

	fr = audio.reference * pow(2.0, round(cf) / 12.0);

	// Lower and upper freq

	fl = audio.reference * pow(2.0, (round(cf) - 0.55) / 12.0);
	fh = audio.reference * pow(2.0, (round(cf) + 0.55) / 12.0);

	// Note number

	n = round(cf) + C5_OFFSET;

	if (n < 0)
	    found = FALSE;

	// Find nearest maximum to reference note

	double df = 1000.0;

	for (int i = 0; i < count; i++)
	{
	    if (fabs(maxima[i].f - fr) < df)
	    {
		df = fabs(maxima[i].f - fr);
		f = maxima[i].f;
	    }
	}

	// Cents relative to reference note

	c = -12.0 * log2(fr / f);

	// Ignore silly values

	if (!isfinite(c))
	    c = 0.0;

	// Ignore if not within 50 cents of reference note

	if (fabs(c) > 0.5)
	    found = FALSE;
    }

    // If display not locked

    if (!display.lock)
    {
	// Update scope window

	scope.data = data;
	InvalidateRgn(scope.hwnd, NULL, TRUE);

	// Update spectrum window

	for (int i = 0; i < count; i++)
	    values[i].f = maxima[i].f / fps * audio.correction;

	spectrum.count = count;

	if (found)
	{
	    spectrum.f = f  / fps * audio.correction;
	    spectrum.r = fr / fps * audio.correction;
	    spectrum.l = fl / fps * audio.correction;
	    spectrum.h = fh / fps * audio.correction;
	}

	InvalidateRgn(spectrum.hwnd, NULL, TRUE);
    }

    static long timer;

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

	// Reset count;

	timer = 0;
    }

    else
    {
	// If display not locked

	if (!display.lock)
	{

	    if (timer > TIMER_COUNT)
	    {
		display.f = 0.0;
		display.fr = 0.0;
		display.c = 0.0;
		display.n = 0;
		display.count = 0;

		// Update meter

		meter.c = 0.0;

		// Update strobe

		strobe.c = 0.0;

		// Update spectrum

		spectrum.f = 0.0;
		spectrum.r = 0.0;
		spectrum.l = 0.0;
		spectrum.h = 0.0;
	    }

	    // Update display

	    InvalidateRgn(display.hwnd, NULL, TRUE);
	}
    }

    timer++;
}

// Real to complex FFT, ignores imaginary values in input array

VOID fftr(complex a[], int n)
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
