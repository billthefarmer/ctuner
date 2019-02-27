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

#include "Tuner.h"

// Application entry point.
int WINAPI WinMain(HINSTANCE hInstance,
		   HINSTANCE hPrevInstance,
		   LPSTR lpszCmdLine,
		   int nCmdShow)
{
    // Check for a previous instance of this app
    if (!hPrevInstance)
	if (!RegisterMainClass(hInstance))
	    return false;

    // Save the application-instance handle.
    hInst = hInstance;

    // Initialize common controls to get the new style controls, also
    // dependent on manifest file
    InitCommonControls();

    // Start Gdiplus
    GdiplusStartup(&token, &input, NULL);

    // Get saved status
    GetSavedStatus();

    // Create the main window.
    window.hwnd =
	CreateWindow(WCLASS, "Tuner",
		     WS_OVERLAPPED | WS_MINIMIZEBOX |
		     WS_SIZEBOX | WS_SYSMENU,
		     CW_USEDEFAULT, CW_USEDEFAULT,
		     CW_USEDEFAULT, CW_USEDEFAULT,
		     NULL, 0, hInst, NULL);

    // If the main window cannot be created, terminate
    // the application.
    if (!window.hwnd)
	return false;

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
    audio.filter = false;
    audio.reference = A5_REFNCE;
    audio.temperament = 8;
    display.transpose = 0;
    spectrum.expand = 1;
    spectrum.zoom = true;
    staff.enable = true;
    strobe.colours = 1;
    strobe.enable = false;

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
        {
	    strobe.enable = value;
            staff.enable = !value;
        }

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
        {
            // Create toolbar
            toolbar.hwnd =
                CreateWindow(TOOLBARCLASSNAME, NULL,
                             WS_VISIBLE | WS_CHILD | TBSTYLE_TOOLTIPS,
                             0, 0, 0, 0,
                             hWnd, (HMENU)TOOLBAR_ID, hInst, NULL);

            SendMessage(toolbar.hwnd, TB_BUTTONSTRUCTSIZE, 
                        (WPARAM)sizeof(TBBUTTON), 0);

            SendMessage(toolbar.hwnd, TB_SETBITMAPSIZE, 0, MAKELONG(24, 24));
            SendMessage(toolbar.hwnd, TB_SETMAXTEXTROWS, 0, 0);

            // Add bitmap
            AddToolbarBitmap(toolbar.hwnd, "Toolbar");

            // Add buttons
            AddToolbarButtons(toolbar.hwnd);

            // Resize toolbar
            SendMessage(toolbar.hwnd, TB_AUTOSIZE, 0, 0); 
            GetWindowRect(toolbar.hwnd, &toolbar.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&toolbar.rect, 2);

            // Get the window and client dimensions
            GetWindowRect(hWnd, &window.wind);
            GetClientRect(hWnd, &window.rect);

            // Calculate desired window width and height
            int border = (window.wind.right - window.wind.left) -
                window.rect.right;
            int header = (window.wind.bottom - window.wind.top) -
                window.rect.bottom;
            int width  = WIDTH + border;
            int height = HEIGHT + toolbar.rect.bottom + header;

            // Set new dimensions
            SetWindowPos(hWnd, NULL, 0, 0,
                         width, height,
                         SWP_NOMOVE | SWP_NOZORDER);

            // Get client dimensions
            GetWindowRect(hWnd, &window.wind);
            GetClientRect(hWnd, &window.rect);

            width = window.rect.right;
            height = window.rect.bottom;

            // Create tooltip
            tooltip.hwnd =
                CreateWindow(TOOLTIPS_CLASS, NULL,
                             WS_POPUP | TTS_ALWAYSTIP,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             hWnd, NULL, hInst, NULL);

            SetWindowPos(tooltip.hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

            tooltip.info.cbSize = sizeof(tooltip.info);
            tooltip.info.hwnd = hWnd;
            tooltip.info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;

            // Create scope display
            scope.hwnd =
                CreateWindow(WC_STATIC, NULL,
                             WS_VISIBLE | WS_CHILD |
                             SS_NOTIFY | SS_OWNERDRAW,
                             MARGIN, MARGIN, width - MARGIN * 2,
                             SCOPE_HEIGHT, hWnd,
                             (HMENU)SCOPE_ID, hInst, NULL);
            GetWindowRect(scope.hwnd, &scope.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&scope.rect, 2);

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
                             MARGIN, scope.rect.bottom + SPACING,
                             width - MARGIN * 2,
                             SPECTRUM_HEIGHT, hWnd,
                             (HMENU)SPECTRUM_ID, hInst, NULL);
            GetWindowRect(spectrum.hwnd, &spectrum.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&spectrum.rect, 2);

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
                             MARGIN, spectrum.rect.bottom + SPACING,
                             width - MARGIN * 2, DISPLAY_HEIGHT, hWnd,
                             (HMENU)DISPLAY_ID, hInst, NULL);
            GetWindowRect(display.hwnd, &display.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&display.rect, 2);

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
                             MARGIN, display.rect.bottom + SPACING,
                             width - MARGIN * 2, STROBE_HEIGHT, hWnd,
                             (HMENU)STROBE_ID, hInst, NULL);
            GetWindowRect(strobe.hwnd, &strobe.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&strobe.rect, 2);

            // Create tooltip for strobe
            tooltip.info.uId = (UINT_PTR)strobe.hwnd;
            tooltip.info.lpszText = (LPSTR)"Strobe, click to disable/enable";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Show window
            ShowWindow(strobe.hwnd, strobe.enable? SW_SHOW: SW_HIDE);

            // Create staff
            staff.hwnd =
                CreateWindow(WC_STATIC, NULL,
                             WS_VISIBLE | WS_CHILD |
                             SS_NOTIFY | SS_OWNERDRAW,
                             MARGIN, display.rect.bottom + SPACING,
                             width - MARGIN * 2, STAFF_HEIGHT, hWnd,
                             (HMENU)STAFF_ID, hInst, NULL);
            GetWindowRect(staff.hwnd, &staff.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&staff.rect, 2);

            // Create tooltip for staff
            tooltip.info.uId = (UINT_PTR)staff.hwnd;
            tooltip.info.lpszText = (LPSTR)"Staff, click to disable/enable";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Show window
            ShowWindow(staff.hwnd, staff.enable? SW_SHOW: SW_HIDE);

            // Create meter
            meter.hwnd =
                CreateWindow(WC_STATIC, NULL,
                             WS_VISIBLE | WS_CHILD |
                             SS_NOTIFY | SS_OWNERDRAW,
                             MARGIN, strobe.rect.bottom + SPACING,
                             width - MARGIN * 2, METER_HEIGHT, hWnd,
                             (HMENU)METER_ID, hInst, NULL);
            GetWindowRect(meter.hwnd, &meter.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&meter.rect, 2);

            // Add meter to tooltip
            tooltip.info.uId = (UINT_PTR)meter.hwnd;
            tooltip.info.lpszText = (LPSTR)"Cents, click to lock";

            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Find font
            HRSRC hRes = FindResource(hInst, "Musica", RT_FONT);

            if (hRes) 
            {
                // Load font
                HGLOBAL mem = LoadResource(hInst, hRes);
                void *data = LockResource(mem);
                size_t size = SizeofResource(hInst, hRes);

                DWORD n;
                // Add font
                AddFontMemResourceEx(data, size, 0, &n);
            }

            // Start audio thread
            audio.thread = CreateThread(NULL, 0, AudioThread, hWnd,
                                        0, &audio.id);

            // Start meter timer
            CreateTimerQueueTimer(&meter.timer, NULL,
                                  (WAITORTIMERCALLBACK)MeterCallback,
                                  &meter.hwnd, METER_DELAY, METER_DELAY,
                                  WT_EXECUTEDEFAULT);

            // Start strobe timer
            CreateTimerQueueTimer(&strobe.timer, NULL,
                                  (WAITORTIMERCALLBACK)StrobeCallback,
                                  &strobe.hwnd, STROBE_DELAY, STROBE_DELAY,
                                  WT_EXECUTEDEFAULT);
        }
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

	// Set the focus back to the window by clicking
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
	SetFocus(hWnd);
	break;

        // Context menu
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

	    // Staff
	case STAFF_ID:
	    StaffClicked(wParam, lParam);
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

	    // Multiple control
	case MULT_ID:
	    MultipleClicked(wParam, lParam);
	    break;

	    // Options
	case OPTIONS_ID:
	    DisplayOptions(wParam, lParam);
	    break;

	    // Quit
	case QUIT_ID:
            Gdiplus::GdiplusShutdown(token);
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

	// Size
    case WM_SIZE:
        WindowResize(hWnd, wParam, lParam);
	break;

	// Sizing
    case WM_SIZING:
	return WindowResizing(hWnd, wParam, lParam);
	break;

        // Process other messages.
    case WM_DESTROY:
        Gdiplus::GdiplusShutdown(token);
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

// WindowResize
BOOL WindowResize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    int width = LOWORD(lParam);
    int height = HIWORD(lParam) - toolbar.rect.bottom;

    // Get the window and client dimensions
    GetWindowRect(hWnd, &window.wind);
    GetClientRect(hWnd, &window.rect);

    if (width < (height * WIDTH) / HEIGHT)
    {
        // Calculate desired window width and height
        int border = (window.wind.right - window.wind.left) -
            window.rect.right;
        int header = (window.wind.bottom - window.wind.top) -
            window.rect.bottom;
        width = ((height * WIDTH) / HEIGHT) + border;
        height = height + toolbar.rect.bottom + header;

        // Set new dimensions
        SetWindowPos(hWnd, NULL, 0, 0,
                     width, height,
                     SWP_NOMOVE | SWP_NOZORDER);
        return true;
    }

    EnumChildWindows(hWnd, EnumChildProc, lParam);
    return true;
}

// Enum child proc for window resizing
BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
    int width = LOWORD(lParam);
    int height = HIWORD(lParam) - toolbar.rect.bottom;

    // Switch by id to resize tool windows.
    switch ((DWORD)GetWindowLongPtr(hWnd, GWLP_ID))
    {
	// Toolbar, let it resize itself
    case TOOLBAR_ID:
	SendMessage(hWnd, WM_SIZE, 0, lParam);
	GetWindowRect(hWnd, &toolbar.rect);
	MapWindowPoints(NULL, window.hwnd, (POINT *)&toolbar.rect, 2);
	break;

	// Scope, resize it
    case SCOPE_ID:
	MoveWindow(hWnd, MARGIN, toolbar.rect.bottom + MARGIN,
                   width - MARGIN * 2,
                   (height - TOTAL) * SCOPE_HEIGHT / TOTAL_HEIGHT,
                   false);
	InvalidateRgn(hWnd, NULL, true);
	GetWindowRect(hWnd, &scope.rect);
	MapWindowPoints(NULL, window.hwnd, (POINT *)&scope.rect, 2);
	break;

	// Spectrum, resize it
    case SPECTRUM_ID:
	MoveWindow(hWnd, MARGIN, scope.rect.bottom + SPACING,
		   width - MARGIN * 2,
                   (height - TOTAL) * SPECTRUM_HEIGHT / TOTAL_HEIGHT,
                   false);
	InvalidateRgn(hWnd, NULL, true);
	GetWindowRect(hWnd, &spectrum.rect);
	MapWindowPoints(NULL, window.hwnd, (POINT *)&spectrum.rect, 2);
	break;

	// Display, resize it
    case DISPLAY_ID:
	MoveWindow(hWnd, MARGIN, spectrum.rect.bottom + SPACING,
                   width - MARGIN * 2,
                   (height - TOTAL) * DISPLAY_HEIGHT / TOTAL_HEIGHT,
                   false);
	InvalidateRgn(hWnd, NULL, true);
	GetWindowRect(hWnd, &display.rect);
	MapWindowPoints(NULL, window.hwnd, (POINT *)&display.rect, 2);
	break;

	// Strobe, resize it
    case STROBE_ID:
	MoveWindow(hWnd, MARGIN, display.rect.bottom + SPACING,
                   width - MARGIN * 2,
                   (height - TOTAL) * STROBE_HEIGHT / TOTAL_HEIGHT,
                   false);
	InvalidateRgn(hWnd, NULL, true);
	GetWindowRect(hWnd, &strobe.rect);
	MapWindowPoints(NULL, window.hwnd, (POINT *)&strobe.rect, 2);
	break;

	// Staff, resize it
    case STAFF_ID:
	MoveWindow(hWnd, MARGIN, display.rect.bottom + SPACING,
                   width - MARGIN * 2,
                   (height - TOTAL) * STAFF_HEIGHT / TOTAL_HEIGHT,
                   false);
	InvalidateRgn(hWnd, NULL, true);
	GetWindowRect(hWnd, &staff.rect);
	MapWindowPoints(NULL, window.hwnd, (POINT *)&staff.rect, 2);
	break;

	// Meter, resize it
    case METER_ID:
	MoveWindow(hWnd, MARGIN, strobe.rect.bottom + SPACING,
                   width - MARGIN * 2,
                   (height - TOTAL) * METER_HEIGHT / TOTAL_HEIGHT,
                   false);
	InvalidateRgn(hWnd, NULL, true);
	GetWindowRect(hWnd, &meter.rect);
	MapWindowPoints(NULL, window.hwnd, (POINT *)&meter.rect, 2);
	break;
    }

    return true;
}

// Window resizing
BOOL WindowResizing(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PRECT rectp = (PRECT)lParam;

    // Get the window and client dimensions
    GetWindowRect(hWnd, &window.wind);
    GetClientRect(hWnd, &window.rect);

    // Edges
    int border = (window.wind.right - window.wind.left) -
        window.rect.right;
    int header = (window.wind.bottom - window.wind.top) -
        window.rect.bottom;

    // Window minimum width and height
    int width  = WIDTH + border;
    int height = HEIGHT + toolbar.rect.bottom + header;

    // Minimum size
    if (rectp->right - rectp->left < width)
	rectp->right = rectp->left + width;

    if (rectp->bottom - rectp->top < height)
	rectp->bottom = rectp->top + height;

    // Maximum width
    if (rectp->right - rectp->left > STEP + border)
        rectp->right = rectp->left + STEP + border;

    // Offered width and height
    width = rectp->right - rectp->left;
    height = rectp->bottom - rectp->top;

    switch (wParam)
    {
    case WMSZ_LEFT:
    case WMSZ_RIGHT:
        height = (((width - border) * HEIGHT) / WIDTH) +
            toolbar.rect.bottom + header;
        rectp->bottom = rectp->top + height;
        break;

    case WMSZ_TOP:
    case WMSZ_BOTTOM:
        width = ((((height - toolbar.rect.bottom) - header) *
                  WIDTH) / HEIGHT) + border;
        rectp->right = rectp->left + width;
        break;

    default:
        width = ((((height - toolbar.rect.bottom) - header) *
                  WIDTH) / HEIGHT) + border;
        rectp->right = rectp->left + width;
        break;
    }

    return true;
}

// Add toolbar bitmap
BOOL AddToolbarBitmap(HWND control, LPCTSTR name)
{
    // Load bitmap
    HBITMAP hbm = (HBITMAP)
	LoadImage(hInst, name, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

    // Create DC
    HDC hdc = CreateCompatibleDC(NULL);

    // Select the bitmap
    SelectObject(hdc, hbm);

    // Select a brush
    SelectObject(hdc, GetSysColorBrush(COLOR_BTNFACE));

    // Get the colour of the first pixel
    COLORREF colour = GetPixel(hdc, 0, 0);

    // Flood fill the bitmap
    ExtFloodFill(hdc, 0, 0, colour, FLOODFILLSURFACE);

    // And the centre of the icon
    if (GetPixel(hdc, 15, 15) == colour)
            ExtFloodFill(hdc, 15, 15, colour, FLOODFILLSURFACE);

    // Delete the DC
    DeleteObject(hdc);

    // Add bitmap
    TBADDBITMAP bitmap =
	{NULL, (UINT_PTR)hbm};

    SendMessage(control, TB_ADDBITMAP, 1, (LPARAM)&bitmap);

    return true;
}

BOOL AddToolbarButtons(HWND control)
{
    // Define the buttons
    TBBUTTON buttons[] =
	{{0, 0, 0, BTNS_SEP},
	 {OPTIONS_BM, OPTIONS_ID, TBSTATE_ENABLED, BTNS_BUTTON,
	  {0}, 0, (INT_PTR)"Options"},
	 {0, 0, 0, BTNS_SEP}};

    // Add to toolbar
    SendMessage(control, TB_ADDBUTTONS,
		Length(buttons), (LPARAM)&buttons);

    return true;
}

// Draw item
BOOL DrawItem(WPARAM wParam, LPARAM lParam)
{
    LPDRAWITEMSTRUCT lpdi = (LPDRAWITEMSTRUCT)lParam;
    RECT rect = lpdi->rcItem;
    HDC hdc = lpdi->hDC;

    SetGraphicsMode(hdc, GM_ADVANCED);

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

	// Staff
    case STAFF_ID:
	return DrawStaff(hdc, rect);
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

    return false;
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
    AppendMenu(menu, audio.down? MF_STRING | MF_CHECKED:
	       MF_STRING, DOWN_ID, "Downsample");
    AppendMenu(menu, display.lock? MF_STRING | MF_CHECKED:
	       MF_STRING, LOCK_ID, "Lock display");
    AppendMenu(menu, display.mult? MF_STRING | MF_CHECKED:
	       MF_STRING, MULT_ID, "Multiple notes");
    AppendMenu(menu, MF_SEPARATOR, 0, 0);
    AppendMenu(menu, MF_STRING, OPTIONS_ID, "Options...");
    AppendMenu(menu, MF_SEPARATOR, 0, 0);
    AppendMenu(menu, MF_STRING, QUIT_ID, "Quit");

    // Pop up the menu
    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		   point.x, point.y,
		   0, hWnd, NULL);
    return true;
}

// Display options
BOOL DisplayOptions(WPARAM wParam, LPARAM lParam)
{
    WNDCLASS wc = 
        {CS_HREDRAW | CS_VREDRAW, OptionWProc,
         0, 0, hInst,
         LoadIcon(hInst, "Tuner"),
         LoadCursor(NULL, IDC_ARROW),
         GetSysColorBrush(COLOR_WINDOW),
         NULL, PCLASS};

    // Register the window class.
    RegisterClass(&wc);

    // Get the main window rect
    GetWindowRect(window.hwnd, &window.wind);

    // Create the window, offset right
    options.hwnd =
        CreateWindow(PCLASS, "Tuner Options",
                     WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION,
                     window.wind.left + OFFSET,
                     window.wind.top + OFFSET,
                     OPTIONS_WIDTH, OPTIONS_HEIGHT,
                     window.hwnd, (HMENU)NULL, hInst, NULL);
    return true;
}

// Options Procedure
LRESULT CALLBACK OptionWProc(HWND hWnd,
                             UINT uMsg,
                             WPARAM wParam,
                             LPARAM lParam)
{
    // Switch on message
    switch (uMsg)
    {
    case WM_CREATE:
        {
            static TCHAR s[64];

            // Get the window and client dimensions
            GetWindowRect(hWnd, &options.wind);
            GetClientRect(hWnd, &options.rect);

            // Calculate desired window width and height
            int border = (options.wind.right - options.wind.left) -
                options.rect.right;
            int header = (options.wind.bottom - options.wind.top) -
                options.rect.bottom;
            int width  = OPTIONS_WIDTH + border;
            int height = OPTIONS_HEIGHT + header;

            // Set new dimensions
            SetWindowPos(hWnd, NULL, 0, 0,
                         width, height,
                         SWP_NOMOVE | SWP_NOZORDER);

            // Get client dimensions
            GetWindowRect(hWnd, &options.wind);
            GetClientRect(hWnd, &options.rect);

            width = options.rect.right;
            height = options.rect.bottom;

            // Create group box
            group.hwnd =
                CreateWindow(WC_BUTTON, NULL,
                             WS_VISIBLE | WS_CHILD |
                             BS_GROUPBOX,
                             MARGIN, MARGIN, width - MARGIN * 2, GROUP_HEIGHT,
                             hWnd, NULL, hInst, NULL);
            GetWindowRect(group.hwnd, &group.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&group.rect, 2);

            // Create zoom tickbox
            zoom.hwnd =
                CreateWindow(WC_BUTTON, "Zoom spectrum:",
                             WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
                             BS_CHECKBOX,
                             group.rect.left + MARGIN, group.rect.top + MARGIN,
                             CHECK_WIDTH, CHECK_HEIGHT,
                             hWnd, (HMENU)ZOOM_ID, hInst, NULL);
            GetWindowRect(zoom.hwnd, &zoom.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&zoom.rect, 2);

            Button_SetCheck(zoom.hwnd,
                            spectrum.zoom? BST_CHECKED: BST_UNCHECKED);

            // Add tickbox to tooltip
            tooltip.info.uId = (UINT_PTR)zoom.hwnd;
            tooltip.info.lpszText = (LPSTR)"Zoom spectrum";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create strobe enable tickbox
            enable.hwnd =
                CreateWindow(WC_BUTTON, "Display strobe:",
                             WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
                             BS_CHECKBOX,
                             width / 2 + MARGIN, group.rect.top + MARGIN,
                             CHECK_WIDTH, CHECK_HEIGHT,
                             hWnd, (HMENU)ENABLE_ID, hInst, NULL);
            GetWindowRect(enable.hwnd, &enable.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&enable.rect, 2);

            Button_SetCheck(enable.hwnd,
                            strobe.enable? BST_CHECKED: BST_UNCHECKED);

            // Add tickbox to tooltip
            tooltip.info.uId = (UINT_PTR)enable.hwnd;
            tooltip.info.lpszText = (LPSTR)"Display strobe";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create filter tickbox
            filt.hwnd =
                CreateWindow(WC_BUTTON, "Audio filter:",
                             WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
                             BS_CHECKBOX,
                             group.rect.left + MARGIN,
                             zoom.rect.bottom + SPACING,
                             CHECK_WIDTH, CHECK_HEIGHT,
                             hWnd, (HMENU)FILTER_ID, hInst, NULL);
            GetWindowRect(filt.hwnd, &filt.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&filt.rect, 2);

            Button_SetCheck(filt.hwnd,
                            audio.filter? BST_CHECKED: BST_UNCHECKED);

            // Add tickbox to tooltip
            tooltip.info.uId = (UINT_PTR)filt.hwnd;
            tooltip.info.lpszText = (LPSTR)"Audio filter";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create downsample tickbox
            down.hwnd =
                CreateWindow(WC_BUTTON, "Downsample:",
                             WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
                             BS_CHECKBOX,
                             width / 2 + MARGIN, enable.rect.bottom + SPACING,
                             CHECK_WIDTH, CHECK_HEIGHT,
                             hWnd, (HMENU)DOWN_ID, hInst, NULL);
            GetWindowRect(down.hwnd, &down.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&down.rect, 2);

            Button_SetCheck(down.hwnd,
                            audio.down? BST_CHECKED: BST_UNCHECKED);

            // Add tickbox to tooltip
            tooltip.info.uId = (UINT_PTR)lock.hwnd;
            tooltip.info.lpszText = (LPSTR)"Downsample";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create multiple tickbox
            mult.hwnd =
                CreateWindow(WC_BUTTON, "Multiple notes:",
                             WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
                             BS_CHECKBOX,
                             group.rect.left + MARGIN,
                             filt.rect.bottom + SPACING,
                             CHECK_WIDTH, CHECK_HEIGHT,
                             hWnd, (HMENU)MULT_ID, hInst, NULL);
            GetWindowRect(mult.hwnd, &mult.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&mult.rect, 2);

            Button_SetCheck(mult.hwnd,
                            display.mult? BST_CHECKED: BST_UNCHECKED);

            // Add tickbox to tooltip
            tooltip.info.uId = (UINT_PTR)mult.hwnd;
            tooltip.info.lpszText = (LPSTR)"Display multiple notes";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create lock tickbox
            lock.hwnd =
                CreateWindow(WC_BUTTON, "Lock display:",
                             WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
                             BS_CHECKBOX,
                             width / 2 + MARGIN,
                             down.rect.bottom + SPACING,
                             CHECK_WIDTH, CHECK_HEIGHT,
                             hWnd, (HMENU)LOCK_ID, hInst, NULL);
            GetWindowRect(lock.hwnd, &lock.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&lock.rect, 2);

            Button_SetCheck(lock.hwnd,
                            display.lock? BST_CHECKED: BST_UNCHECKED);

            // Add tickbox to tooltip
            tooltip.info.uId = (UINT_PTR)lock.hwnd;
            tooltip.info.lpszText = (LPSTR)"Lock display";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create fundamental tickbox
            fund.hwnd =
                CreateWindow(WC_BUTTON, "Fundamental:",
                             WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
                             BS_CHECKBOX,
                             group.rect.left + MARGIN,
                             mult.rect.bottom + SPACING,
                             CHECK_WIDTH, CHECK_HEIGHT,
                             hWnd, (HMENU)FUND_ID, hInst, NULL);
            GetWindowRect(fund.hwnd, &fund.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&fund.rect, 2);

            Button_SetCheck(fund.hwnd,
                            audio.fund? BST_CHECKED: BST_UNCHECKED);

            // Add tickbox to tooltip
            tooltip.info.uId = (UINT_PTR)fund.hwnd;
            tooltip.info.lpszText = (LPSTR)"Fundamental filter";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create note filter tickbox
            note.hwnd =
                CreateWindow(WC_BUTTON, "Note filter:",
                             WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
                             BS_CHECKBOX,
                             width / 2 + MARGIN,
                             lock.rect.bottom + SPACING,
                             CHECK_WIDTH, CHECK_HEIGHT,
                             hWnd, (HMENU)NOTE_ID, hInst, NULL);
            GetWindowRect(note.hwnd, &note.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&note.rect, 2);

            Button_SetCheck(note.hwnd,
                            audio.note? BST_CHECKED: BST_UNCHECKED);

            // Add tickbox to tooltip
            tooltip.info.uId = (UINT_PTR)note.hwnd;
            tooltip.info.lpszText = (LPSTR)"Note filter";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create group box
            group.hwnd =
                CreateWindow(WC_BUTTON, NULL,
                             WS_VISIBLE | WS_CHILD |
                             BS_GROUPBOX,
                             MARGIN, group.rect.bottom + SPACING,
                             width - MARGIN * 2, EXPAND_HEIGHT,
                             hWnd, NULL, hInst, NULL);
            GetWindowRect(group.hwnd, &group.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&group.rect, 2);

            // Create text
            text.hwnd =
                CreateWindow(WC_STATIC, "Spectrum expand:",
                             WS_VISIBLE | WS_CHILD | SS_LEFT,
                             group.rect.left + MARGIN,
                             group.rect.top + MARGIN,
                             CHECK_WIDTH, CHECK_HEIGHT, hWnd,
                             (HMENU)TEXT_ID, hInst, NULL);
            GetWindowRect(text.hwnd, &text.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&text.rect, 2);

            // Create combo box
            expand.hwnd =
                CreateWindow(WC_COMBOBOX, "",
                             WS_VISIBLE | WS_CHILD |
                             CBS_DROPDOWNLIST,
                             width / 2 + MARGIN, text.rect.top,
                             CHECK_WIDTH, CHECK_HEIGHT, hWnd,
                             (HMENU)EXPAND_ID, hInst, NULL);
            GetWindowRect(expand.hwnd, &expand.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&expand.rect, 2);

            const char *sizes[] =
                {" x 1", " x 2", " x 4", " x 8", " x 16"};
            for (unsigned int i = 0; i < Length(sizes); i++)
                ComboBox_AddString(expand.hwnd, sizes[i]);

            // Select x 1
            ComboBox_SelectString(expand.hwnd, -1, " x 1");

            // Add edit to tooltip
            tooltip.info.uId = (UINT_PTR)expand.hwnd;
            tooltip.info.lpszText = (LPSTR)"Spectrum expand";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create text
            text.hwnd =
                CreateWindow(WC_STATIC, "Strobe colours:",
                             WS_VISIBLE | WS_CHILD | SS_LEFT,
                             group.rect.left + MARGIN,
                             text.rect.bottom + SPACING,
                             CHECK_WIDTH, CHECK_HEIGHT, hWnd,
                             (HMENU)TEXT_ID, hInst, NULL);
            GetWindowRect(text.hwnd, &text.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&text.rect, 2);

            // Create combo box
            colours.hwnd =
                CreateWindow(WC_COMBOBOX, "",
                             WS_VISIBLE | WS_CHILD |
                             CBS_DROPDOWNLIST,
                             width / 2, text.rect.top,
                             CHECK_WIDTH + MARGIN, CHECK_HEIGHT, hWnd,
                             (HMENU)COLOURS_ID, hInst, NULL);
            GetWindowRect(colours.hwnd, &colours.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&colours.rect, 2);

            const char *strings[] =
                {" Blue/Cyan", " Olive/Aqua", " Magenta/Yellow"};
            for (unsigned int i = 0; i < Length(strings); i++)
                ComboBox_AddString(colours.hwnd, strings[i]);

            // Select Olive/Aqua
            ComboBox_SetCurSel(colours.hwnd, strobe.colours);

            // Add edit to tooltip
            tooltip.info.uId = (UINT_PTR)colours.hwnd;
            tooltip.info.lpszText = (LPSTR)"Strobe colours";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create text
            text.hwnd =
                CreateWindow(WC_STATIC, "Reference:",
                             WS_VISIBLE | WS_CHILD | SS_LEFT,
                             group.rect.left + MARGIN,
                             text.rect.bottom + SPACING,
                             CHECK_WIDTH, CHECK_HEIGHT, hWnd,
                             (HMENU)TEXT_ID, hInst, NULL);
            GetWindowRect(text.hwnd, &text.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&text.rect, 2);

            // Create edit control
            sprintf(s, " %6.2lf", audio.reference);

            reference.hwnd =
                CreateWindow(WC_EDIT, s,
                             WS_VISIBLE | WS_CHILD |
                             WS_BORDER,
                             width / 2 + MARGIN, text.rect.top,
                             CHECK_WIDTH, CHECK_HEIGHT, hWnd,
                             (HMENU)REFERENCE_ID, hInst, NULL);
            GetWindowRect(reference.hwnd, &reference.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&reference.rect, 2);

            // Add edit to tooltip
            tooltip.info.uId = (UINT_PTR)reference.hwnd;
            tooltip.info.lpszText = (LPSTR)"Reference";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create up-down control
            updown.hwnd =
                CreateWindow(UPDOWN_CLASS, NULL,
                             WS_VISIBLE | WS_CHILD |
                             UDS_AUTOBUDDY | UDS_ALIGNRIGHT,
                             0, 0, 0, 0, hWnd,
                             (HMENU)UPDOWN_ID, hInst, NULL);

            SendMessage(updown.hwnd, UDM_SETRANGE32, MIN_REF, MAX_REF);
            SendMessage(updown.hwnd, UDM_SETPOS32, 0, audio.reference * 10);

            // Add updown to tooltip
            tooltip.info.uId = (UINT_PTR)updown.hwnd;
            tooltip.info.lpszText = (LPSTR)"Reference";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create text
            text.hwnd =
                CreateWindow(WC_STATIC, "Transpose:",
                             WS_VISIBLE | WS_CHILD | SS_LEFT,
                             group.rect.left + MARGIN,
                             text.rect.bottom + SPACING,
                             CHECK_WIDTH, CHECK_HEIGHT, hWnd,
                             (HMENU)TEXT_ID, hInst, NULL);
            GetWindowRect(text.hwnd, &text.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&text.rect, 2);

            // Create combo box
            transpose.hwnd =
                CreateWindow(WC_COMBOBOX, "",
                             WS_VISIBLE | WS_CHILD |
                             CBS_DROPDOWNLIST,
                             width / 2 + MARGIN, text.rect.top,
                             CHECK_WIDTH, CHECK_HEIGHT, hWnd,
                             (HMENU)TRANSPOSE_ID, hInst, NULL);
            GetWindowRect(transpose.hwnd, &transpose.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&transpose.rect, 2);

            const char *trans[] =
                {" +6[Key:F#]", " +5[Key:F]", " +4[Key:E]",
                 " +3[Key:Eb]", " +2[Key:D]",
                 " +1[Key:C#]", " +0[Key:C]", " -1[Key:B]",
                 " -2[Key:Bb]", " -3[Key:A]",
                 " -4[Key:Ab]", " -5[Key:G]",
                 " -6[Key:F#]"};
            for (unsigned int i = 0; i < Length(trans); i++)
                ComboBox_AddString(transpose.hwnd, trans[i]);

            // Select +0[Key:C]
            ComboBox_SetCurSel(transpose.hwnd, 6 - display.transpose);

            // Add to tooltip
            tooltip.info.uId = (UINT_PTR)transpose.hwnd;
            tooltip.info.lpszText = (LPSTR)"Transpose display";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create text
            text.hwnd =
                CreateWindow(WC_STATIC, "Temperament:",
                             WS_VISIBLE | WS_CHILD | SS_LEFT,
                             group.rect.left + MARGIN,
                             text.rect.bottom + SPACING,
                             CHECK_WIDTH, CHECK_HEIGHT, hWnd,
                             (HMENU)TEXT_ID, hInst, NULL);
            GetWindowRect(text.hwnd, &text.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&text.rect, 2);

            // Create combo box
            temperament.hwnd =
                CreateWindow(WC_COMBOBOX, "",
                             WS_VISIBLE | WS_CHILD | WS_VSCROLL |
                             CBS_DROPDOWNLIST,
                             width / 2 - MARGIN, text.rect.top,
                             CHECK_WIDTH + MARGIN * 2, CHECK_HEIGHT, hWnd,
                             (HMENU)TEMPERAMENT_ID, hInst, NULL);
            GetWindowRect(temperament.hwnd, &temperament.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&temperament.rect, 2);

            const char *temp[] =
                {" Kirnberger II", " Kirnberger III",
                 " Werckmeister III", " Werckmeister IV",
                 " Werckmeister V", " Werckmeister VI",
                 " Bach (Klais)", " Just (Barbour)",
                 " Equal Temperament", " Pythagorean",
                 " Van Zwolle", " Meantone (-1/4)",
                 " Silbermann (-1/6)", " Salinas (-1/3)",
                 " Zarlino (-2/7)", " Rossi (-1/5)",
                 " Rossi (-2/9)", " Rameau (-1/4)",
                 " Kellner", " Vallotti",
                 " Young II", " Bendeler III",
                 " Neidhardt I", " Neidhardt II",
                 " Neidhardt III", " Bruder 1829",
                 " Barnes 1977", " Lambert 1774",
                 " Schlick (H. Vogel)", " Meantone # (-1/4)",
                 " Meantone b (-1/4)", " Lehman-Bach"};
            for (unsigned int i = 0; i < Length(temp); i++)
                ComboBox_AddString(temperament.hwnd, temp[i]);

            // Select Equal Temperament
            ComboBox_SetCurSel(temperament.hwnd, audio.temperament);

            // Add edit to tooltip
            tooltip.info.uId = (UINT_PTR)temperament.hwnd;
            tooltip.info.lpszText = (LPSTR)"Temperament";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create text
            text.hwnd =
                CreateWindow(WC_STATIC, "Key:",
                             WS_VISIBLE | WS_CHILD | SS_LEFT,
                             group.rect.left + MARGIN,
                             text.rect.bottom + SPACING,
                             CHECK_WIDTH, CHECK_HEIGHT, hWnd,
                             (HMENU)TEXT_ID, hInst, NULL);
            GetWindowRect(text.hwnd, &text.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&text.rect, 2);

            // Create combo box
            key.hwnd =
                CreateWindow(WC_COMBOBOX, "",
                             WS_VISIBLE | WS_CHILD |
                             CBS_DROPDOWNLIST,
                             width / 4, text.rect.top,
                             MARGIN * 2, CHECK_HEIGHT, hWnd,
                             (HMENU)KEY_ID, hInst, NULL);
            GetWindowRect(key.hwnd, &key.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&key.rect, 2);

            const char *keys[] =
                {" C", " C#", " D", " Eb",
                 " E", " F", " F#", " G",
                 " Ab", " A", " Bb", " B"};
            for (unsigned int i = 0; i < Length(keys); i++)
                ComboBox_AddString(key.hwnd, keys[i]);

            // Select C
            ComboBox_SetCurSel(key.hwnd, audio.key);

            // Add edit to tooltip
            tooltip.info.uId = (UINT_PTR)key.hwnd;
            tooltip.info.lpszText = (LPSTR)"Key";
            SendMessage(tooltip.hwnd, TTM_ADDTOOL, 0,
                        (LPARAM) &tooltip.info);

            // Create filter button
            button.filter.hwnd =
                CreateWindow(WC_BUTTON, "Filters...",
                             WS_VISIBLE | WS_CHILD |
                             BS_PUSHBUTTON,
                             width / 2 + MARGIN, text.rect.top,
                             CHECK_WIDTH, BUTTON_HEIGHT,
                             hWnd, (HMENU)FILTERS_ID, hInst, NULL);

            // Enable button
            Button_Enable(button.filter.hwnd, audio.note);

            // Create close button
            // button.close.hwnd =
            //     CreateWindow(WC_BUTTON, "Close",
            //                  WS_VISIBLE | WS_CHILD |
            //                  BS_PUSHBUTTON,
            //                  width / 2 + MARGIN,
            //                  text.rect.bottom + SPACING,
            //                  CHECK_WIDTH, BUTTON_HEIGHT,
            //                  hWnd, (HMENU)CLOSE_ID, hInst, NULL);
        }
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
	switch (GetWindowLongPtr((HWND)lParam, GWLP_ID))
	{
	case UPDOWN_ID:
	    ChangeReference(wParam, lParam);
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

	    // Multiple control
	case MULT_ID:
	    MultipleClicked(wParam, lParam);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;

            // Fundamental control
        case FUND_ID:
            FundamentalClicked(wParam, lParam);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;

            // Note filter control
        case NOTE_ID:
            NoteFilterClicked(wParam, lParam);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;

            // Expand
        case EXPAND_ID:
            ExpandClicked(wParam, lParam);
	    break;

            // Colours
        case COLOURS_ID:
            ColoursClicked(wParam, lParam);
	    break;

	    // Reference
	case REFERENCE_ID:
	    EditReference(wParam, lParam);
	    break;

            // Transpose
        case TRANSPOSE_ID:
            TransposeClicked(wParam, lParam);
            break;

            // Temperament
        case TEMPERAMENT_ID:
            TemperamentClicked(wParam, lParam);
            break;

            // Key
        case KEY_ID:
            KeyClicked(wParam, lParam);
            break;

	    // Filters
	case FILTERS_ID:
	    DisplayFilters(wParam, lParam);
	    break;

	    // Close
	case CLOSE_ID:
            SendMessage(hWnd, WM_DESTROY, 0, 0);
	    ShowWindow(hWnd, false);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;
	}
	break;

	// Char pressed
    case WM_CHAR:
	CharPressed(wParam, lParam);
	break;

    case WM_DESTROY:
        options.hwnd = NULL;
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
    return true;
}

// Enable clicked

BOOL EnableClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	StrobeClicked(wParam, lParam);
    }
    return true;
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
	return false;
    }

    if (filt.hwnd != NULL)
	Button_SetCheck(filt.hwnd, audio.filter? BST_CHECKED: BST_UNCHECKED);

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

	return false;
    }

    return true;
}

// Colours clicked
BOOL ColoursClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
        strobe.colours++;

        if (strobe.colours > MAGENTA)
            strobe.colours = BLUE;

        if (colours.hwnd != NULL)
            ComboBox_SetCurSel(colours.hwnd, strobe.colours);

        strobe.changed = true;
	break;

    case CBN_SELENDOK:
        strobe.colours = ComboBox_GetCurSel(colours.hwnd);
        strobe.changed = true;
        break;

    default:
	return false;
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

	return false;
    }

    return true;
}

// Down clicked
BOOL DownClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	audio.down = !audio.down;
	break;

    default:
	return false;
    }

    if (down.hwnd != NULL)
	Button_SetCheck(down.hwnd, audio.down? BST_CHECKED: BST_UNCHECKED);

    return true;
}

// Fundamental clicked
BOOL FundamentalClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	audio.fund = !audio.fund;
	break;

    default:
	return false;
    }

    if (fund.hwnd != NULL)
	Button_SetCheck(fund.hwnd, audio.fund? BST_CHECKED: BST_UNCHECKED);

    return true;
}

// Note filter clicked
BOOL NoteFilterClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	audio.note = !audio.note;
	break;

    default:
	return false;
    }

    if (note.hwnd != NULL)
	Button_SetCheck(note.hwnd, audio.note? BST_CHECKED: BST_UNCHECKED);

    if (button.filter.hwnd != NULL)
        Button_Enable(button.filter.hwnd, audio.note);

    return true;
}

// Expand clicked
BOOL ExpandClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	if (spectrum.expand < 16)
	    spectrum.expand *= 2;

        if (expand.hwnd != NULL)
            ComboBox_SetCurSel(expand.hwnd, round(log2(spectrum.expand)));
	break;

    case CBN_SELENDOK:
        spectrum.expand = round(pow(2, ComboBox_GetCurSel(expand.hwnd)));
        break;

    default:
	return false;
    }

    return true;
}

// Contract clicked
BOOL ContractClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	if (spectrum.expand > 1)
	    spectrum.expand /= 2;

        if (expand.hwnd != NULL)
            ComboBox_SetCurSel(expand.hwnd, round(log2(spectrum.expand)));
	break;

    default:
	return false;
    }

    return true;
}

// Transpose clicked
BOOL TransposeClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case CBN_SELENDOK:
        display.transpose = 6 - ComboBox_GetCurSel(transpose.hwnd);
        staff.transpose = display.transpose;
        break;

    default:
	return false;
    }

    return true;
}

// Temperament clicked
BOOL TemperamentClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case CBN_SELENDOK:
        audio.temperament = ComboBox_GetCurSel(temperament.hwnd);
        break;

    default:
	return false;
    }

    return true;
}

// Key clicked
BOOL KeyClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case CBN_SELENDOK:
        audio.key = ComboBox_GetCurSel(key.hwnd);
        break;

    default:
	return false;
    }

    return true;
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
	return false;
    }

    InvalidateRgn(display.hwnd, NULL, true);

    if (lock.hwnd != NULL)
	Button_SetCheck(lock.hwnd, display.lock? BST_CHECKED: BST_UNCHECKED);
    return true;
}

// Multiple clicked
BOOL MultipleClicked(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	display.mult = !display.mult;
	break;

    default:
	return false;
    }

    if (mult.hwnd != NULL)
	Button_SetCheck(mult.hwnd, display.mult? BST_CHECKED: BST_UNCHECKED);

    InvalidateRgn(display.hwnd, NULL, true);

    return true;
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
    return true;
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
	ColoursClicked(wParam, lParam);
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

    return true;
}

// Copy display
BOOL CopyDisplay(WPARAM wParam, LPARAM lParam)
{
    // Memory size
    enum
    {MEM_SIZE = 1024};

    static TCHAR s[64];

    static const TCHAR *notes[] =
	{"C", "C#", "D", "Eb", "E", "F",
	 "F#", "G", "Ab", "A", "Bb", "B"};

    // Open clipboard
    if (!OpenClipboard(window.hwnd))
	return false;

    // Empty clipboard
    EmptyClipboard(); 

    // Allocate memory
    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, MEM_SIZE);

    if (mem == NULL)
    {
	CloseClipboard();
	return false;
    }

    // Lock the memory
    LPTSTR text = (LPTSTR)GlobalLock(mem);

    // Check if multiple
    if (display.mult && display.count > 0)
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
	    wsprintf(s, "%s%d\t%+6.2lf\t%9.2lf\t%9.2lf\t%+8.2lf\r\n",
		    notes[n % Length(notes)], n / 12,
		    c * 100.0, fr, f, f - fr);

	    // Copy to the memory
	    if (i == 0)
		lstrcpy(text, s);

	    else
		lstrcat(text, s);
	}
    }

    else
    {
	// Print the values
	wsprintf(s, "%s%d\t%+6.2lf\t%9.2lf\t%9.2lf\t%+8.2lf\r\n",
		notes[display.n % Length(notes)], display.n / 12,
		display.c * 100.0, display.fr, display.f,
		display.f - display.fr);

	// Copy to the memory
	lstrcpy(text, s);
    }

    // Place in clipboard
    GlobalUnlock(text);
    SetClipboardData(CF_TEXT, mem);
    CloseClipboard(); 
 
    return true;
}

// Display filters
BOOL DisplayFilters(WPARAM wParam, LPARAM lParam)
{
    WNDCLASS wc = 
        {CS_HREDRAW | CS_VREDRAW, FilterWProc,
         0, 0, hInst,
         LoadIcon(hInst, "Tuner"),
         LoadCursor(NULL, IDC_ARROW),
         GetSysColorBrush(COLOR_WINDOW),
         NULL, FCLASS};

    // Register the window class.
    RegisterClass(&wc);

    // Get the options window rect
    GetWindowRect(options.hwnd, &options.rect);

    // Create the window, offset right
    filters.hwnd =
        CreateWindow(FCLASS, "Tuner Filters",
                     WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION,
                     options.rect.left + OFFSET,
                     options.rect.top + OFFSET,
                     FILTERS_WIDTH, FILTERS_HEIGHT,
                     window.hwnd, (HMENU)NULL, hInst, NULL);
    return true;
}

// Filters Procedure
LRESULT CALLBACK FilterWProc(HWND hWnd,
                             UINT uMsg,
                             WPARAM wParam,
                             LPARAM lParam)
{
    // Switch on message
    switch (uMsg)
    {
    case WM_CREATE:
        {
            // Get the window and client dimensions
            GetWindowRect(hWnd, &filters.wind);
            GetClientRect(hWnd, &filters.rect);

            // Calculate desired window width and height
            int border = (filters.wind.right - filters.wind.left) -
                filters.rect.right;
            int header = (filters.wind.bottom - filters.wind.top) -
                filters.rect.bottom;
            int width  = FILTERS_WIDTH + border;
            int height = FILTERS_HEIGHT + header;

            // Set new dimensions
            SetWindowPos(hWnd, NULL, 0, 0,
                         width, height,
                         SWP_NOMOVE | SWP_NOZORDER);

            // Get client dimensions
            GetWindowRect(hWnd, &filters.wind);
            GetClientRect(hWnd, &filters.rect);

            width = filters.rect.right;
            height = filters.rect.bottom;

            // Create group box
            group.hwnd =
                CreateWindow(WC_BUTTON, NULL,
                             WS_VISIBLE | WS_CHILD |
                             BS_GROUPBOX,
                             MARGIN, MARGIN,
                             width - MARGIN * 2, FILTER_HEIGHT,
                             hWnd, NULL, hInst, NULL);
            GetWindowRect(group.hwnd, &group.rect);
            MapWindowPoints(NULL, hWnd, (POINT *)&group.rect, 2);

            static const TCHAR *labels[] =
                {"C:", "C#:", "D:", "Eb:", "E:", "F:",
                 "F#:", "G:", "Ab:", "A:", "Bb:", "B:"};
            static const UINT_PTR noteIds[] =
                {NOTES_C, NOTES_Cs, NOTES_D, NOTES_Eb,
                 NOTES_E, NOTES_F, NOTES_Fs, NOTES_G,
                 NOTES_Ab, NOTES_A, NOTES_Bb, NOTES_B};
            static const UINT_PTR octaveIds[] =
                {OCTAVES_0, OCTAVES_1, OCTAVES_2,
                 OCTAVES_3, OCTAVES_4, OCTAVES_5,
                 OCTAVES_6, OCTAVES_7, OCTAVES_8};

            for (unsigned int i = 0; i < Length(labels); i++)
            {
                boxes.notes[i].hwnd =
                    CreateWindow(WC_BUTTON, labels[i],
                                 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
                                 BS_CHECKBOX, (i < Length(labels) / 2)?
                                 group.rect.left + MARGIN:
                                 group.rect.left + (MARGIN * 2) + NOTE_WIDTH,
                                 group.rect.top + MARGIN +
                                 (NOTE_HEIGHT + SPACING) *
                                 (i % (Length(labels) / 2)),
                                 NOTE_WIDTH, NOTE_HEIGHT,
                                 hWnd, (HMENU)noteIds[i], hInst, NULL);
                GetWindowRect(boxes.notes[i].hwnd, &boxes.notes[i].rect);
                MapWindowPoints(NULL, hWnd, (POINT *)&boxes.notes[i].rect, 2);

                Button_SetCheck(boxes.notes[i].hwnd,
                                filter.note[i]? BST_CHECKED: BST_UNCHECKED);
            }

            for (unsigned int i = 0; i < Length(filter.octave); i++)
            {
                static TCHAR s[64];
                sprintf(s, "Octave %d:", i);

                boxes.octaves[i].hwnd =
                    CreateWindow(WC_BUTTON, s,
                                 WS_VISIBLE | WS_CHILD | BS_LEFTTEXT |
                                 BS_CHECKBOX,
                                 (i < (Length(filter.octave) / 2) + 1)?
                                 group.rect.left + MARGIN +
                                 ((NOTE_WIDTH + MARGIN) * 2):
                                 group.rect.left + MARGIN +
                                 ((NOTE_WIDTH + MARGIN) * 2) +
                                 (OCTAVE_WIDTH + MARGIN),
                                 group.rect.top + MARGIN +
                                 (OCTAVE_HEIGHT + SPACING) *
                                 (i % ((Length(filter.octave) / 2) + 1)),
                                 OCTAVE_WIDTH, OCTAVE_HEIGHT,
                                 hWnd, (HMENU)octaveIds[i], hInst, NULL);
                GetWindowRect(boxes.octaves[i].hwnd, &boxes.octaves[i].rect);
                MapWindowPoints(NULL, hWnd, (POINT *)&boxes.octaves[i].rect, 2);

                Button_SetCheck(boxes.octaves[i].hwnd,
                                filter.octave[i]? BST_CHECKED: BST_UNCHECKED);
            }

            // Create close button
            button.close.hwnd =
                CreateWindow(WC_BUTTON, "Close",
                             WS_VISIBLE | WS_CHILD |
                             BS_PUSHBUTTON,
                             width / 2 + MARGIN,
                             boxes.notes[11].rect.top,
                             CHECK_WIDTH, BUTTON_HEIGHT,
                             hWnd, (HMENU)CLOSE_ID, hInst, NULL);
        }
	break;

	// Colour static text
    case WM_CTLCOLORSTATIC:
	return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
	break;

	// Draw item
    case WM_DRAWITEM:
	return DrawItem(wParam, lParam);
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
	    // Close
	case CLOSE_ID:
            SendMessage(hWnd, WM_DESTROY, 0, 0);
	    ShowWindow(hWnd, false);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;

        default:
            BoxClicked(wParam, lParam);

	    // Set the focus back to the window
	    SetFocus(hWnd);
	    break;
	}
	break;

	// Char pressed
    case WM_CHAR:
	CharPressed(wParam, lParam);
	break;

    case WM_DESTROY:
        options.hwnd = NULL;
        break;

	// Everything else
    default:
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

BOOL BoxClicked(WPARAM wParam, LPARAM lParam)
{
    static const UINT_PTR noteIds[] =
        {NOTES_C, NOTES_Cs, NOTES_D, NOTES_Eb,
         NOTES_E, NOTES_F, NOTES_Fs, NOTES_G,
         NOTES_Ab, NOTES_A, NOTES_Bb, NOTES_B};
    static const UINT_PTR octaveIds[] =
        {OCTAVES_0, OCTAVES_1, OCTAVES_2,
         OCTAVES_3, OCTAVES_4, OCTAVES_5,
         OCTAVES_6, OCTAVES_7, OCTAVES_8};

    int id = LOWORD(wParam);

    // Check notes
    for (unsigned int i = 0; i < Length(filter.note); i++)
    {
        if (id == noteIds[i])
        {
            filter.note[i] = !filter.note[i];
            Button_SetCheck((HWND)lParam,
                            filter.note[i]? BST_CHECKED: BST_UNCHECKED);
            return true;
        }
    }

    // Check octaves
    for (unsigned int i = 0; i < Length(filter.octave); i++)
    {
        if (id == octaveIds[i])
        {
            filter.octave[i] = !filter.octave[i];
            Button_SetCheck((HWND)lParam,
                            filter.octave[i]? BST_CHECKED: BST_UNCHECKED);
            return true;
        }
    }

    return false;
}

// Meter callback
VOID CALLBACK MeterCallback(PVOID lpParam, BOOL TimerFired)
{
    METERP meter = (METERP)lpParam;

    // Update meter
    InvalidateRgn(meter->hwnd, NULL, true);
}

// Strobe callback
VOID CALLBACK StrobeCallback(PVOID lpParam, BOOL TimerFired)
{
    STROBEP strobe = (STROBEP)lpParam;

    // Update strobe
    if (strobe->enable)
    	InvalidateRgn(strobe->hwnd, NULL, true);
}

// Draw scope
BOOL DrawScope(HDC hdc, RECT rect)
{
    using Gdiplus::SmoothingModeAntiAlias;
    using Gdiplus::Graphics;
    using Gdiplus::PointF;
    using Gdiplus::Color;
    using Gdiplus::Pen;

    static HBITMAP bitmap;
    static HFONT font;
    static SIZE size;
    static HDC hbdc;

    enum
    {FONT_HEIGHT   = 10};

    // Bold font
    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_BOLD,
	 false, false, false,
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
	SelectObject(hbdc, GetStockObject(DC_PEN));

	// Select font
	SelectObject(hbdc, font);
	SetTextAlign(hbdc, TA_LEFT | TA_BOTTOM);
	SetBkMode(hbdc, TRANSPARENT);
    }

    // Create new bitmaps if resized
    if (width != size.cx || height != size.cy)
    {
	// Delete old bitmap
	if (bitmap != NULL)
	    DeleteObject(bitmap);

	// Create new bitmap
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);

	size.cx = width;
	size.cy = height;
    }

    // Erase background
    // SetViewportOrgEx(hbdc, 0, 0, NULL);
    RECT brct = {0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(BLACK_BRUSH));

    // Dark green graticule
    SetDCPenColor(hbdc, RGB(0, 64, 0));

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

	return true;
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

    static float max;

    if (max < 4096)
	max = 4096;

    float yscale = max / (height / 2);
    max = 0.0;
    
    // Graphics
    Graphics graphics(hbdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    // Move the origin
    graphics.TranslateTransform(0.0, height / 2.0);

    // Green pen for scope trace
    Pen pen(Color(0, 255, 0), 1);

    // Draw the trace
    PointF last(-1.0, 0.0);

    for (unsigned int i = 0; i < width && i < scope.length; i++)
    {
	if (max < abs(scope.data[n + i]))
	    max = abs(scope.data[n + i]);

	float y = -scope.data[n + i] / yscale;
        PointF point(i, y);
        graphics.DrawLine(&pen, last, point);

        last = point;
    }

    // Move the origin back
    // SetViewportOrgEx(hbdc, 0, 0, NULL);

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

    return true;
}

// Draw spectrum
BOOL DrawSpectrum(HDC hdc, RECT rect)
{
    using Gdiplus::SmoothingModeAntiAlias;
    using Gdiplus::GraphicsPath;
    using Gdiplus::SolidBrush;
    using Gdiplus::Graphics;
    using Gdiplus::PointF;
    using Gdiplus::Color;
    using Gdiplus::Pen;

    static HBITMAP bitmap;
    static HFONT font;
    static SIZE size;
    static HDC hbdc;

    enum
    {FONT_HEIGHT   = 10};

    // Bold font
    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_BOLD,
	 false, false, false,
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
    DrawEdge(hdc, &rect, EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate bitmap dimensions
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create DC
    if (hbdc == NULL)
    {
	hbdc = CreateCompatibleDC(hdc);
	SelectObject(hbdc, GetStockObject(DC_PEN));

	// Select font
	SelectObject(hbdc, font);
	SetBkMode(hbdc, TRANSPARENT);
    }

    // Create new bitmaps if resized
    if (width != size.cx || height != size.cy)
    {
	// Delete old bitmap
	if (bitmap != NULL)
	    DeleteObject(bitmap);

	// Create new bitmap
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);

	size.cx = width;
	size.cy = height;
    }

    // Erase background
    SetViewportOrgEx(hbdc, 0, 0, NULL);
    RECT brct = {0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(BLACK_BRUSH));

    // Dark green graticule
    SetDCPenColor(hbdc, RGB(0, 64, 0));

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

	return true;
    }

    static float max;

    if (max < 1.0)
	max = 1.0;

    // Calculate the scaling
    float yscale = (float)height / max;

    max = 0.0;

    // Graphics
    Graphics graphics(hbdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.TranslateTransform(0.0, height - 1.0);
    // Green pen for spectrum trace
    Pen pen(Color(0, 255, 0), 1);
    // Transparent green brush for spectrum fill
    SolidBrush brush(Color(63, 0, 255, 0));

    // Draw the spectrum
    PointF lastp(0.0, 0.0);
    GraphicsPath path;

    if (spectrum.zoom)
    {
	// Calculate scale
	float xscale = ((float)width / (spectrum.r - spectrum.l)) / 2.0;

	for (unsigned int i = floor(spectrum.l); i <= ceil(spectrum.h); i++)
	{
	    if (i > 0 && i < spectrum.length)
	    {
		float value = spectrum.data[i];

		if (max < value)
		    max = value;

                PointF point((i - spectrum.l) * xscale, -value * yscale);
                path.AddLine(lastp, point);
                lastp = point;
	    }
	}

        graphics.DrawPath(&pen, &path);
        path.AddLine(lastp, PointF(width, 0.0));
        path.CloseFigure();
        graphics.FillPath(&brush, &path);

        SetViewportOrgEx(hbdc, 0, height - 1, NULL);
	SetDCPenColor(hbdc, RGB(0, 255, 0));
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
	    if (spectrum.values[i] > spectrum.l &&
		spectrum.values[i] < spectrum.h)
	    {
		int x = round((spectrum.values[i] - spectrum.l) * xscale);
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
		TextOut(hbdc, x, 2, s, lstrlen(s));
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
	    for (unsigned int i = last; i <= index; i++)
	    {
		// Don't show DC component
		if (i > 0 && i < spectrum.length)
		{
		    if (value < spectrum.data[i])
			value = spectrum.data[i];
		}
	    }

	    // Update last index
	    last = index;

	    if (max < value)
		max = value;

            float y = -value * yscale;

	    PointF point(x, y);
            path.AddLine(lastp, point);

            lastp = point;
	}

        graphics.DrawPath(&pen, &path);
        path.AddLine(lastp, PointF(width, 0.0));
        path.CloseFigure();
        graphics.FillPath(&brush, &path);

	// Yellow pen for frequency trace
        SetViewportOrgEx(hbdc, 0, height - 1, NULL);
	SetDCPenColor(hbdc, RGB(255, 255, 0));
	SetTextColor(hbdc, RGB(255, 255, 0));
	SetTextAlign(hbdc, TA_CENTER | TA_BOTTOM);

	// Draw lines for each frequency
	for (int i = 0; i < spectrum.count; i++)
	{
	    // Draw line for each
	    int x = round(log(spectrum.values[i]) / xscale);
	    MoveToEx(hbdc, x, 0, NULL);
	    LineTo(hbdc, x, -height);

	    double f = display.maxima[i].f;

	    // Reference freq
	    double fr = display.maxima[i].fr;
	    double c = -12.0 * log2(fr / f);

	    // Ignore silly values
	    if (!isfinite(c))
		continue;

	    sprintf(s,  "%+0.0f", c * 100.0);
	    TextOut(hbdc, x, 2, s, lstrlen(s));
	}

	SetTextAlign(hbdc, TA_LEFT | TA_BOTTOM);

	if (spectrum.expand > 1)
	{
	    sprintf(s, "x%d", spectrum.expand);
	    TextOut(hbdc, 0, 2, s, lstrlen(s));
	}
    }

    // D for downsample
    if (audio.down)
    {
	SetTextAlign(hbdc, TA_LEFT | TA_BOTTOM);
	TextOut(hbdc, 0, 10 - height, "D", 1);
    }

    // Move the origin back
    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Copy the bitmap
    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return true;
}

// Draw display
BOOL DrawDisplay(HDC hdc, RECT rect)
{
    static HBITMAP bitmap;
    static HFONT medium;
    static HFONT larger;
    static HFONT large;
    static HFONT music;
    static HFONT half;
    static HFONT font;
    static SIZE size;
    static HDC hbdc;

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
	 false, false, false,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 ANTIALIASED_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 ""};

    static TCHAR s[64];

    // Draw nice etched edge
    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate dimensions
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create new fonts if resized
    if (width != size.cx || height != size.cy)
    {
        if (font != NULL)
        {
            DeleteObject(font);
            DeleteObject(half);
            DeleteObject(large);
            DeleteObject(larger);
            DeleteObject(medium);
            DeleteObject(music);
        }

	lf.lfHeight = height / 3;
        lf.lfWeight = FW_BOLD;
        strcpy(lf.lfFaceName, "");
	large = CreateFontIndirect(&lf);

	lf.lfHeight = height / 2;
	larger = CreateFontIndirect(&lf);

	lf.lfHeight = height / 5;
	medium = CreateFontIndirect(&lf);

        lf.lfHeight = height / 8;
        lf.lfWeight = FW_NORMAL;
	font = CreateFontIndirect(&lf);

	lf.lfHeight = height / 4;
        lf.lfWeight = FW_BOLD;
        half = CreateFontIndirect(&lf);

	lf.lfHeight = height / 4;
        strcpy(lf.lfFaceName, "Musica");
	music = CreateFontIndirect(&lf);
    }

    // Create DC
    if (hbdc == NULL)
	hbdc = CreateCompatibleDC(hdc);

    // Create new bitmaps if resized
    if (width != size.cx || height != size.cy)
    {
	// Delete old bitmap
	if (bitmap != NULL)
	    DeleteObject(bitmap);

	// Create new bitmap
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);

	size.cx = width;
	size.cy = height;
    }

    // Erase background
    RECT brct = {0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(WHITE_BRUSH));

    if (display.mult)
    {
	// Select font
	SelectObject(hbdc, font);

	// Set text align
	SetTextAlign(hbdc, TA_TOP);

	if (display.count == 0)
	{
            int x = 4;

	    // Display note
	    sprintf(s, "%s%s%d", notes[(display.n - display.transpose +
                                        OCTAVE) % OCTAVE],
                sharps[(display.n - display.transpose +
                        OCTAVE) % OCTAVE], display.n / 12);
	    TextOut(hbdc, x, 0, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display cents
	    sprintf(s, "%+4.2lf¢", display.c * 100.0);
	    TextOut(hbdc, x, 0, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display reference
	    sprintf(s, "%4.2lfHz", display.fr);
	    TextOut(hbdc, x, 0, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display frequency
	    sprintf(s, "%4.2lfHz", display.f);
	    TextOut(hbdc, x, 0, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display difference
	    sprintf(s, "%+4.2lfHz", display.f - display.fr);
	    TextOut(hbdc, x, 0, s, lstrlen(s));
	}

        int y = 0;

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

            int x = 4;

	    // Display note
	    sprintf(s, "%s%s%d", notes[(n - display.transpose +
                                        OCTAVE) % OCTAVE],
		    sharps[(n  - display.transpose +
                            OCTAVE) % OCTAVE], n / 12);
	    TextOut(hbdc, x, y, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display cents
	    sprintf(s, "%+4.2lf¢", c * 100.0);
	    TextOut(hbdc, x, y, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display reference
	    sprintf(s, "%4.2lfHz", fr);
	    TextOut(hbdc, x, y, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display frequency
	    sprintf(s, "%4.2lfHz", f);
	    TextOut(hbdc, x, y, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            x += size.cx + 4;

	    // Display difference
	    sprintf(s, "%+4.2lfHz", f - fr);
	    TextOut(hbdc, x, y, s, lstrlen(s));

            GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
            y += size.cy;
        }
    }

    else
    {
	// Select larger font
	SelectObject(hbdc, larger);

	// Text size
	SIZE size = {0};

	// Set text align
	SetTextAlign(hbdc, TA_BOTTOM|TA_LEFT);
	SetBkMode(hbdc, TRANSPARENT);

	// Display note
	sprintf(s, "%s", notes[(display.n  - display.transpose +
                                OCTAVE) % OCTAVE]);
	GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);

        int y = size.cy;
	TextOut(hbdc, 8, y, s, lstrlen(s));

	int x = size.cx + 8;

	// Select music font
	SelectObject(hbdc, half);

	sprintf(s, "%d", display.n / 12);
	TextOut(hbdc, x, y, s, lstrlen(s));

	// Select music font
	SelectObject(hbdc, music);

	sprintf(s, "%s", sharps[(display.n  - display.transpose +
                                 OCTAVE) % OCTAVE]);
	GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
	TextOut(hbdc, x, y - size.cy, s, lstrlen(s));

	// Select large font
	SelectObject(hbdc, large);
	SetTextAlign(hbdc, TA_BOTTOM|TA_RIGHT);

	// Display cents
	sprintf(s, "%+4.2f¢", display.c * 100.0);
	TextOut(hbdc, width - 8, y, s, lstrlen(s));

	// Select medium font
	SelectObject(hbdc, medium);
	SetTextAlign(hbdc, TA_BOTTOM|TA_LEFT);

	// Display reference frequency
	sprintf(s, "%4.2fHz", display.fr);
	GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
	y += size.cy;
	TextOut(hbdc, 8, y, s, lstrlen(s));

	// Display actual frequency
	SetTextAlign(hbdc, TA_BOTTOM|TA_RIGHT);
	sprintf(s, "%4.2fHz", display.f);
	TextOut(hbdc, width - 8, y, s, lstrlen(s));


	// Display reference
	SetTextAlign(hbdc, TA_BOTTOM|TA_LEFT);
	sprintf(s, "%4.2fHz", audio.reference);
	GetTextExtentPoint32(hbdc, s, lstrlen(s), &size);
	y += size.cy;
	TextOut(hbdc, 8, y, s, lstrlen(s));

	// Display frequency difference
	SetTextAlign(hbdc, TA_BOTTOM|TA_RIGHT);
	sprintf(s, "%+4.2lfHz", display.f - display.fr);
	TextOut(hbdc, width - 8, y, s, lstrlen(s));
    }

    // Show lock
    if (display.lock)
	DrawLock(hbdc, -1, height + 1);

    // Copy the bitmap
    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return true;
}

// Draw lock
BOOL DrawLock(HDC hdc, int x, int y)
{
    POINT point;
    POINT body[] =
	{{2, -3}, {8, -3}, {8, -8}, {2, -8}, {2, -3}};
    POINT hasp[] =
        {{3, -8}, {3, -11}, {7, -11}, {7, -8}};

    SetViewportOrgEx(hdc, x, y, &point);

    Polyline(hdc, body, Length(body));
    Polyline(hdc, hasp, Length(hasp));

    SetPixel(hdc, 3, -11, RGB(255, 170, 85));
    SetPixel(hdc, 6, -10, RGB(255, 170, 85));

    SetPixel(hdc, 4, -10, RGB(85, 170, 255));
    SetPixel(hdc, 7, -11, RGB(85, 170, 255));

    SetPixel(hdc, 7, -7, RGB(255, 170, 85));
    SetPixel(hdc, 7, -4, RGB(255, 170, 85));

    SetPixel(hdc, 3, -7, RGB(85, 170, 255));
    SetPixel(hdc, 3, -4, RGB(85, 170, 255));

    SetViewportOrgEx(hdc, point.x, point.y, NULL);
    return true;
}

// Draw meter
BOOL DrawMeter(HDC hdc, RECT rect)
{
    using Gdiplus::SmoothingModeAntiAlias;
    using Gdiplus::LinearGradientBrush;
    using Gdiplus::WrapModeTileFlipX;
    using Gdiplus::GraphicsPath;
    using Gdiplus::SolidBrush; 
    using Gdiplus::Graphics; 
    using Gdiplus::Matrix;
    using Gdiplus::Color;
    using Gdiplus::Point;
    using Gdiplus::Pen;

    static HBITMAP bitmap;
    static HFONT font;
    static SIZE size;
    static HDC hbdc;

    static float mc;

    // Plain vanilla font
    static LOGFONT lf =
	{0, 0, 0, 0,
	 FW_NORMAL,
	 false, false, false,
	 DEFAULT_CHARSET,
	 OUT_DEFAULT_PRECIS,
	 CLIP_DEFAULT_PRECIS,
	 ANTIALIASED_QUALITY,
	 DEFAULT_PITCH | FF_DONTCARE,
	 ""};

    // Draw nice etched edge
    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate bitmap dimensions
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create DC
    if (hbdc == NULL)
    {
	// Create DC
	hbdc = CreateCompatibleDC(hdc);
	SetTextAlign(hbdc, TA_CENTER);
    }

    // Create new font and bitmap if resized
    if (width != size.cx || height != size.cy)
    {
        if (font != NULL)
            DeleteObject(font);

        lf.lfHeight = height / 3;
	font = CreateFontIndirect(&lf);
	SelectObject(hbdc, font);

	// Delete old bitmap
	if (bitmap != NULL)
	    DeleteObject(bitmap);

	// Create new bitmap
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);

	size.cx = width;
	size.cy = height;
    }

    // Erase background
    SetViewportOrgEx(hbdc, 0, 0, NULL);
    RECT brct = {0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // Move origin
    SetViewportOrgEx(hbdc, width / 2, 0, NULL);

    // Draw the meter scale
    for (int i = 0; i < 6; i++)
    {
	static TCHAR s[16];
	int x = width / 11 * i;

	sprintf(s, "%d", i * 10);
	TextOut(hbdc, x + 1, 0, s, lstrlen(s));
	TextOut(hbdc, -x + 1, 0, s, lstrlen(s));

	MoveToEx(hbdc, x, height / 3, NULL);
	LineTo(hbdc, x, height / 2);
	MoveToEx(hbdc, -x, height / 3, NULL);
	LineTo(hbdc, -x, height / 2);

	for (int j = 1; j < 5; j++)
	{
	    if (i < 5)
	    {
		MoveToEx(hbdc, x + j * width / 55, height * 3 / 8, NULL);
		LineTo(hbdc, x + j * width / 55, height / 2);
	    }

	    MoveToEx(hbdc, -x + j * width / 55, height * 3 / 8, NULL);
	    LineTo(hbdc, -x + j * width / 55, height / 2);
	}
    }

    RECT bar = {-width * 5 / 11, (height * 3 / 4) - 2,
                (width * 5 / 11) + 1, (height * 3 / 4) + 2};
    FrameRect(hbdc, &bar, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

    // Do calculation
    mc = ((mc * 19.0) + meter.c) / 20.0;

    GraphicsPath path;
    path.AddLine(0, 2, 1, 1);
    path.AddLine(1, 1, 1, -2);
    path.AddLine(1, -2, -1, -2);
    path.AddLine(-1, -2, -1, 1);
    path.CloseFigure();

    LinearGradientBrush brush(Point(0, 2), Point(0, -2),
                              Color(255, 255, 255), Color(63, 63, 63));
    brush.SetWrapMode(WrapModeTileFlipX);

    Matrix matrix;
    matrix.Translate(mc * width * 10.0 / 11.0, (height * 3 / 4) - 2);
    matrix.Scale(height / 12.0, -height / 12.0);

    path.Transform(&matrix);
    brush.ScaleTransform(height / 24.0, height / 24.0);

    Pen pen(Color(127, 127, 127));

    Graphics graphics(hbdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.FillPath(&brush, &path);
    graphics.DrawPath(&pen, &path);

    // Move origin back
    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Copy the bitmap
    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return true;
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

    static SIZE size;

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
    int block = height / 4;

    // Create brushes
    int foreground = colours[strobe.colours][0];
    int background = colours[strobe.colours][1];

    if (sbrush == NULL || strobe.changed ||
        size.cx != width || size.cy != height)
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

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 2, block));
	SelectObject(hbdc, GetStockObject(DC_PEN));
	SelectObject(hbdc, GetStockObject(DC_BRUSH));

	SetDCPenColor(hbdc, foreground);
	SetDCBrushColor(hbdc, foreground);
	Rectangle(hbdc, 0, 0, block, block);
	SetDCPenColor(hbdc, background);
	SetDCBrushColor(hbdc, background);
	Rectangle(hbdc, block, 0, block * 2, block);

	sbrush = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 2, block));

	TRIVERTEX vertex[] =
	    {{0, 0,
	      (COLOR16)(GetRValue(foreground) << 8),
	      (COLOR16)(GetGValue(foreground) << 8),
	      (COLOR16)(GetBValue(foreground) << 8),
	      0},
	     {block, block,
	      (COLOR16)(GetRValue(background) << 8),
	      (COLOR16)(GetGValue(background) << 8),
	      (COLOR16)(GetBValue(background) << 8),
	      0},
	     {block * 2, 0,
	      (COLOR16)(GetRValue(foreground) << 8),
	      (COLOR16)(GetGValue(foreground) << 8),
	      (COLOR16)(GetBValue(foreground) << 8),
	      0}};

	GRADIENT_RECT gradient[] =
	    {{0, 1}, {1, 2}};

	GradientFill(hbdc, vertex, Length(vertex),
		     gradient, Length(gradient), GRADIENT_FILL_RECT_H);

	sshade = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 4, block));

	SetDCPenColor(hbdc, foreground);
	SetDCBrushColor(hbdc, foreground);
	Rectangle(hbdc, 0, 0, block * 2, block);
	SetDCPenColor(hbdc, background);
	SetDCBrushColor(hbdc, background);
	Rectangle(hbdc, block * 2, 0, block * 4, block);

	mbrush = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 4, block));

	vertex[1].x = block * 2;
	vertex[2].x = block * 4;

	GradientFill(hbdc, vertex, 3, gradient, 2, GRADIENT_FILL_RECT_H);

	mshade = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 8, block));

	SetDCPenColor(hbdc, foreground);
	SetDCBrushColor(hbdc, foreground);
	Rectangle(hbdc, 0, 0, block * 4, block);
	SetDCPenColor(hbdc, background);
	SetDCBrushColor(hbdc, background);
	Rectangle(hbdc, block * 4, 0, block * 8, block);

	lbrush = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 8, block));

	vertex[1].x = block * 4;
	vertex[2].x = block * 8;

	GradientFill(hbdc, vertex, 3, gradient, 2, GRADIENT_FILL_RECT_H);

	lshade = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	SelectObject(hbdc, CreateCompatibleBitmap(hdc, block * 16, block));

	SetDCPenColor(hbdc, foreground);
	SetDCBrushColor(hbdc, foreground);
	Rectangle(hbdc, 0, 0, block * 8, block);
	SetDCPenColor(hbdc, background);
	SetDCBrushColor(hbdc, background);
	Rectangle(hbdc, block * 8, 0, block * 16, block);

	ebrush = CreatePatternBrush((HBITMAP)GetCurrentObject(hbdc,
							      OBJ_BITMAP));
	DeleteObject(GetCurrentObject(hbdc, OBJ_BITMAP));

	DeleteDC(hbdc);
	strobe.changed = false;
    }

    if (true)
    {
        // Transform viewport
        SetViewportOrgEx(hdc, rect.left, rect.top, NULL);

    	mc = ((19.0 * mc) + strobe.c) / 20.0;
    	mx += mc * 50.0;

    	if (mx > block * 16.0)
    	    mx = 0.0;

    	if (mx < 0.0)
    	    mx = block * 16.0;

    	int rx = round(mx - block * 16.0);
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
	Rectangle(hdc, 0, 0, width, block);

	if (fabs(mc) > 0.3)
	    SelectObject(hdc, mshade);

	else
	    SelectObject(hdc, mbrush);
	Rectangle(hdc, 0, block, width, block * 2);

	if (fabs(mc) > 0.4)
	    SelectObject(hdc, lshade);

	else
	    SelectObject(hdc, lbrush);
	Rectangle(hdc, 0, block * 2, width, block * 3);

	SelectObject(hdc, ebrush);
	Rectangle(hdc, 0, block * 3, width, block * 4);
    }

    else
        FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

    return true;
}

// Draw staff
BOOL DrawStaff(HDC hdc, RECT rect)
{
    using Gdiplus::SmoothingModeAntiAlias;
    using Gdiplus::GraphicsPath;
    using Gdiplus::SolidBrush;
    using Gdiplus::Graphics;
    using Gdiplus::Matrix;
    using Gdiplus::PointF;
    using Gdiplus::Color;
    using Gdiplus::RectF;
    using Gdiplus::SizeF;
    using Gdiplus::Font;
    using Gdiplus::Pen;

    static HBITMAP bitmap;
    static SIZE size;
    static HDC hbdc;

    // Treble clef
    static const float tc[][2] =
        {
         {-6, 16}, {-8, 13},
         {-14, 19}, {-10, 35}, {2, 35},
         {8, 37}, {21, 30}, {21, 17},
         {21, 5}, {10, -1}, {0, -1},
         {-12, -1}, {-23, 5}, {-23, 22},
         {-23, 29}, {-22, 37}, {-7, 49},
         {10, 61}, {10, 68}, {10, 73},
         {10, 78}, {9, 82}, {7, 82},
         {2, 78}, {-2, 68}, {-2, 62},
         {-2, 25}, {10, 18}, {11, -8},
         {11, -18}, {5, -23}, {-4, -23},
         {-10, -23}, {-15, -18}, {-15, -13},
         {-15, -8}, {-12, -4}, {-7, -4},
         {3, -4}, {3, -20}, {-6, -17},
         {-5, -23}, {9, -20}, {9, -9},
         {7, 24}, {-5, 30}, {-5, 67},
         {-5, 78}, {-2, 87}, {7, 91},
         {13, 87}, {18, 80}, {17, 73},
         {17, 62}, {10, 54}, {1, 45},
         {-5, 38}, {-15, 33}, {-15, 19},
         {-15, 7}, {-8, 1}, {0, 1},
         {8, 1}, {15, 6}, {15, 14},
         {15, 23}, {7, 26}, {2, 26},
         {-5, 26}, {-9, 21}, {-6, 16}
        };

    // Bass clef
    static const float bc[][2] =
        {
         {-2.3,3},
         {6,7}, {10.5,12}, {10.5,16},
         {10.5,20.5}, {8.5,23.5}, {6.2,23.3},
         {5.2,23.5}, {2,23.5}, {0.5,19.5},
         {2,20}, {4,19.5}, {4,18},
         {4,17}, {3.5,16}, {2,16},
         {1,16}, {0,16.9}, {0,18.5},
         {0,21}, {2.1,24}, {6,24},
         {10,24}, {13.5,21.5}, {13.5,16.5},
         {13.5,11}, {7,5.5}, {-2.0,2.8},
         {14.9,21},
         {14.9,22.5}, {16.9,22.5}, {16.9,21},
         {16.9,19.5}, {14.9,19.5}, {14.9,21},
         {14.9,15},
         {14.9,16.5}, {16.9,16.5}, {16.9,15},
         {16.9,13.5}, {14.9,13.5}, {14.9,15}
        };

    // Note head
    static const float hd[][2] =
        {
         {8.0, 0.0},
         {8.0, 8.0}, {-8.0, 8.0}, {-8.0, 0.0},
         {-8.0, -8.0}, {8.0, -8.0}, {8.0, 0.0}
        };

    // Sharp symbol
    static const float sp[][2] =
        {
         {35, 35}, // 0
         {8, 22}, // 1
         {8, 46}, // 2
         {35, 59}, // 3
         {35, 101}, // 4
         {8, 88}, // 5
         {8, 111}, // 6
         {35, 125}, // 7
         {35, 160}, // 8
         {44, 160}, // 9
         {44, 129}, // 10
         {80, 147}, // 11
         {80, 183}, // 12
         {89, 183}, // 13
         {89, 151}, // 14
         {116, 165}, // 15
         {116, 141}, // 16
         {89, 127}, // 17
         {89, 86}, // 18
         {116, 100}, // 19
         {116, 75}, // 20
         {89, 62}, // 21
         {89, 19}, // 22
         {80, 19}, // 23
         {80, 57}, // 23
         {44, 39}, // 25
         {44, -1}, // 26
         {35, -1}, // 27
         {35, 35}, // 28
         {44, 64}, // 29
         {80, 81}, // 30
         {80, 123}, // 31
         {44, 105}, // 32
         {44, 64}, // 33
        };

    // Flat symbol
    static const float ft[][2] =
        {
         {20, 86}, // 0
         {28, 102.667}, {41.6667, 111}, {61, 111}, // 3
         {71.6667, 111}, {80.3333, 107.5}, {87, 100.5}, // 6
         {93.6667, 93.5}, {97, 83.6667}, {97, 71}, // 9
         {97, 53}, {89, 36.6667}, {73, 22}, // 12
         {57, 7.33333}, {35.3333, -1.33333}, {8, -4}, // 15
         {8, 195}, // 16
         {20, 195}, // 17
         {20, 86}, // 18
         {20, 7}, // 19
         {35.3333, 9}, {47.8333, 15.6667}, {57.5, 27}, // 22
         {67.1667, 38.3333}, {72, 51.6667}, {72, 67}, // 25
         {72, 75.6667}, {70.1667, 82.3333}, {66.5, 87}, // 28
         {62.8333, 91.6667}, {57.3333, 94}, {50, 94}, // 31
         {41.3333, 94}, {34.1667, 90.3333}, {28.5, 83}, // 34
         {22.8333, 75.6667}, {20, 64.6667}, {20, 50}, // 37
         {20, 7}, // 38
        };

    // Scale offsets
    static const int offset[] =
        {0, 0, 1, 2, 2, 3,
         3, 4, 5, 5, 6, 6};

    static const int sharps[] =
	{NATURAL, SHARP, NATURAL, FLAT, NATURAL, NATURAL,
	 SHARP, NATURAL, FLAT, NATURAL, FLAT, NATURAL};

    // Draw nice etched edge
    DrawEdge(hdc, &rect , EDGE_SUNKEN, BF_ADJUST | BF_RECT);

    // Calculate dimensions
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    int lineHeight = height / 14.0;
    int lineWidth = width / 16.0;
    int margin = width / 32.0;

    // Create DC
    if (hbdc == NULL)
    {
	// Create DC
	hbdc = CreateCompatibleDC(hdc);
    }

    // Create new bitmap if resized
    if (width != size.cx || height != size.cy)
    {
	// Delete old bitmap
	if (bitmap != NULL)
	    DeleteObject(bitmap);

	// Create new bitmap
	bitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hbdc, bitmap);

	size.cx = width;
	size.cy = height;
    }

    // Erase background
    SetViewportOrgEx(hbdc, 0, 0, NULL);
    RECT brct = {0, 0, width, height};
    FillRect(hbdc, &brct, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // Move origin
    SetViewportOrgEx(hbdc, 0, height / 2, NULL);

    // Draw staff
    for (int i = 1; i < 6; i++)
    {
	MoveToEx(hbdc, margin, i * lineHeight, NULL);
	LineTo(hbdc, width - margin, i * lineHeight);
	MoveToEx(hbdc, margin, -i * lineHeight, NULL);
	LineTo(hbdc, width - margin, -i * lineHeight);
    }

    // Draw leger lines
    MoveToEx(hbdc, width / 2 - lineWidth / 2, 0, NULL);
    LineTo(hbdc, width / 2 + lineWidth / 2, 0);
    MoveToEx(hbdc, width / 2 + lineWidth * 5.5, -lineHeight * 6, NULL);
    LineTo(hbdc, width / 2 + lineWidth * 6.5, -lineHeight * 6);
    MoveToEx(hbdc, width / 2 - lineWidth * 5.5, lineHeight * 6, NULL);
    LineTo(hbdc, width / 2 - lineWidth * 6.5, lineHeight * 6);

    // Graphics
    Graphics graphics(hbdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    // Draw treble clef
    GraphicsPath tclef;
    tclef.AddLine(tc[0][0], tc[0][1], tc[1][0], tc[1][1]);
    for (unsigned int i = 1; i < Length(tc) - 1; i += 3)
        tclef.AddBezier(tc[i][0], tc[i][1], tc[i + 1][0], tc[i + 1][1],
                        tc[i + 2][0], tc[i + 2][1], tc[i + 3][0], tc[i + 3][1]);
    Matrix matrix;
    RectF bounds;
    SizeF sizeF;
    Pen pen(Color(0, 0, 0));
    SolidBrush brush(Color(0, 0, 0));

    tclef.GetBounds(&bounds, &matrix, &pen);
    matrix.Translate(-(bounds.GetLeft() + bounds.GetRight()) / 2,
                     -(bounds.GetTop() + bounds.GetBottom()) / 2);
    tclef.Transform(&matrix);
    bounds.GetSize(&sizeF);
    float scale = (height / 2) / sizeF.Height;
    matrix.Reset();
    matrix.Scale(scale, -scale);
    tclef.Transform(&matrix);
    matrix.Reset();
    matrix.Translate(margin + lineWidth / 2, -lineHeight * 3);
    tclef.Transform(&matrix);
    graphics.FillPath(&brush, &tclef);

    // Draw bass clef
    GraphicsPath bclef;
    for (int i = 0; i < 27; i += 3)
        bclef.AddBezier(bc[i][0], bc[i][1], bc[i + 1][0], bc[i + 1][1],
                        bc[i + 2][0], bc[i + 2][1], bc[i + 3][0], bc[i + 3][1]);
    bclef.StartFigure();
    for (int i = 28; i < 34; i += 3)
        bclef.AddBezier(bc[i][0], bc[i][1], bc[i + 1][0], bc[i + 1][1],
                        bc[i + 2][0], bc[i + 2][1], bc[i + 3][0], bc[i + 3][1]);
    bclef.StartFigure();
    for (unsigned int i = 35; i < Length(bc) - 1; i += 3)
        bclef.AddBezier(bc[i][0], bc[i][1], bc[i + 1][0], bc[i + 1][1],
                        bc[i + 2][0], bc[i + 2][1], bc[i + 3][0], bc[i + 3][1]);

    bclef.GetBounds(&bounds, &matrix, &pen);
    matrix.Translate(-(bounds.GetLeft() + bounds.GetRight()) / 2,
                     -(bounds.GetTop() + bounds.GetBottom()) / 2);
    bclef.Transform(&matrix);
    bounds.GetSize(&sizeF);
    scale = (lineHeight * 4.5) / sizeF.Height;
    matrix.Reset();
    matrix.Scale(scale, -scale);
    bclef.Transform(&matrix);
    matrix.Reset();
    matrix.Translate(margin + lineWidth / 2, lineHeight * 2.8);
    bclef.Transform(&matrix);
    graphics.FillPath(&brush, &bclef);

    // Note head
    GraphicsPath head;
    for (unsigned int i = 0; i < Length(hd) - 1; i += 3)
        head.AddBezier(hd[i][0], hd[i][1], hd[i + 1][0], hd[i + 1][1],
                       hd[i + 2][0], hd[i + 2][1], hd[i + 3][0], hd[i + 3][1]);
    head.GetBounds(&bounds, &matrix, &pen);
    bounds.GetSize(&sizeF);
    scale = (lineHeight * 2) / sizeF.Height;
    matrix.Reset();
    matrix.Scale(scale, -scale);
    head.Transform(&matrix);

    // Sharp
    GraphicsPath sharp;
    for (unsigned int i = 0; i < 28; i++)
        sharp.AddLine(sp[i][0], sp[i][1], sp[i + 1][0], sp[i + 1][1]);
    sharp.StartFigure();
    for (int i = 29; i < 33; i++)
        sharp.AddLine(sp[i][0], sp[i][1], sp[i + 1][0], sp[i + 1][1]);
    matrix.Reset();
    sharp.GetBounds(&bounds, &matrix, &pen);
    matrix.Translate(-(bounds.GetLeft() + bounds.GetRight()) / 2,
                     -(bounds.GetTop() + bounds.GetBottom()) / 2);
    sharp.Transform(&matrix);
    matrix.Reset();
    bounds.GetSize(&sizeF);
    scale = (lineHeight * 3) / sizeF.Height;
    matrix.Scale(scale, -scale);
    sharp.Transform(&matrix);

    // Flat
    GraphicsPath flat;
    for (int i = 0; i < 15; i += 3)
        flat.AddBezier(ft[i][0], ft[i][1], ft[i + 1][0], ft[i + 1][1],
                       ft[i + 2][0], ft[i + 2][1], ft[i + 3][0], ft[i + 3][1]);
    for (int i = 15; i < 19; i++)
        flat.AddLine(ft[i][0], ft[i][1], ft[i + 1][0], ft[i + 1][1]);
    flat.StartFigure();
    for (int i = 19; i < 36; i += 3)
        flat.AddBezier(ft[i][0], ft[i][1], ft[i + 1][0], ft[i + 1][1],
                       ft[i + 2][0], ft[i + 2][1], ft[i + 3][0], ft[i + 3][1]);
    flat.AddLine(ft[37][0], ft[37][1], ft[38][0], ft[38][1]);
    matrix.Reset();
    flat.GetBounds(&bounds, &matrix, &pen);
    matrix.Translate(-(bounds.GetLeft() + bounds.GetRight()) / 2,
                     -(bounds.GetTop() + bounds.GetBottom()) / 2);
    flat.Transform(&matrix);
    matrix.Reset();
    bounds.GetSize(&sizeF);
    scale = (lineHeight * 3) / sizeF.Height;
    matrix.Scale(scale, -scale);
    flat.Transform(&matrix);

    // Calculate transform for note
    int xBase = lineWidth * 14;
    int yBase = lineHeight * 14;
    int note = staff.n - staff.transpose;
    int octave = note / OCTAVE;
    int index = (note + OCTAVE) % OCTAVE;

    // Wrap top two octaves
    if (octave >= 6)
        octave -= 2;

    // Wrap C0
    else if (octave == 0 && index <= 1)
        octave += 4;

    // Wrap bottom two octaves
    else if (octave <= 1 || (octave == 2 && index <= 1))
        octave += 2;

    float dx = (octave * lineWidth * 3.5) +
        (offset[index] * lineWidth / 2);
    float dy = (octave * lineHeight * 3.5) +
        (offset[index] * lineHeight / 2);

    // Draw note
    matrix.Reset();
    matrix.Translate(width / 2 - xBase + dx, yBase - dy);
    head.Transform(&matrix);
    graphics.FillPath(&brush, &head);

    // Accidentals
    switch (sharps[index])
    {
        // Natural
    case NATURAL:
        // Do nothing
        break;

        // Draw sharp
    case SHARP:
        matrix.Reset();
        matrix.Translate(width / 2 - xBase + dx - lineWidth / 2, yBase - dy);
        sharp.Transform(&matrix);
        graphics.FillPath(&brush, &sharp);
        break;

        // Draw flat
    case FLAT:
        matrix.Reset();
        matrix.Translate(width / 2 - xBase + dx - lineWidth / 2,
                         yBase - dy - lineHeight / 2);
        flat.Transform(&matrix);
        graphics.FillPath(&brush, &flat);
        break;
    }

    // Move origin back
    SetViewportOrgEx(hbdc, 0, 0, NULL);

    // Copy the bitmap
    BitBlt(hdc, rect.left, rect.top, width, height,
	   hbdc, 0, 0, SRCCOPY);

    return true;
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
	return false;
    }

    if (lock.hwnd != NULL)
	Button_SetCheck(lock.hwnd, display.lock? BST_CHECKED: BST_UNCHECKED);

    InvalidateRgn(display.hwnd, NULL, true);

    return true;
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
	return false;
    }

    return true;
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
	return false;
    }

    if (zoom.hwnd != NULL)
	Button_SetCheck(zoom.hwnd, spectrum.zoom? BST_CHECKED: BST_UNCHECKED);

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

    return true;
}

// Strobe clicked
BOOL StrobeClicked(WPARAM wParam, LPARAM lParam)
{

    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	strobe.enable = !strobe.enable;
        staff.enable = !strobe.enable;
	break;

    default:
	return false;
    }

    ShowWindow(strobe.hwnd, strobe.enable? SW_SHOW: SW_HIDE);
    ShowWindow(staff.hwnd, staff.enable? SW_SHOW: SW_HIDE);

    InvalidateRgn(strobe.hwnd, NULL, true);
    InvalidateRgn(staff.hwnd, NULL, true);

    if (enable.hwnd != NULL)
	Button_SetCheck(enable.hwnd, strobe.enable? BST_CHECKED: BST_UNCHECKED);

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

    return true;
}

// Staff clicked
BOOL StaffClicked(WPARAM wParam, LPARAM lParam)
{

    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
	staff.enable = !staff.enable;
        strobe.enable = !staff.enable;
	break;

    default:
	return false;
    }

    ShowWindow(staff.hwnd, staff.enable? SW_SHOW: SW_HIDE);
    ShowWindow(strobe.hwnd, strobe.enable? SW_SHOW: SW_HIDE);

    InvalidateRgn(staff.hwnd, NULL, true);
    InvalidateRgn(strobe.hwnd, NULL, true);

    if (enable.hwnd != NULL)
	Button_SetCheck(enable.hwnd, strobe.enable? BST_CHECKED: BST_UNCHECKED);

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

    return true;
}

// Meter clicked
BOOL MeterClicked(WPARAM wParam, LPARAM lParam)
{
    return DisplayClicked(wParam, lParam);
}

// Edit reference
BOOL EditReference(WPARAM wParam, LPARAM lParam)
{
    static TCHAR s[64];

    if (audio.reference == 0)
	return false;

    switch (HIWORD(wParam))
    {
    case EN_KILLFOCUS:
	GetWindowText(reference.hwnd, s, sizeof(s));
	audio.reference = atof(s);

	SendMessage(reference.hwnd, UDM_SETPOS32, 0,
		    audio.reference * 10);
	break;

    default:
	return false;
    }

    InvalidateRgn(display.hwnd, NULL, true);

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

    return true;
}

// Change reference
BOOL ChangeReference(WPARAM wParam, LPARAM lParam)
{
    static TCHAR s[64];

    long value = SendMessage(updown.hwnd, UDM_GETPOS32, 0, 0);
    audio.reference = (double)value / 10.0;

    sprintf(s, " %6.2lf", audio.reference);
    SetWindowText(reference.hwnd, s);

    InvalidateRgn(display.hwnd, NULL, true);

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

    return true;
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
    static double  values[MAXIMA];

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
	xf[i] = (i * fps + df * fps);

	// Calculate differences for finding maxima
	dx[i] = xa[i] - xa[i - 1];
    }

    // Downsample
    if (audio.down)
    {
	// x2 = xa << 2
	for (unsigned int i = 0; i < Length(x2); i++)
	{
	    x2[i] = 0.0;

	    for (int j = 0; j < 2; j++)
		x2[i] += xa[(i * 2) + j] / 2.0;
	}

	// x3 = xa << 3
	for (unsigned int i = 0; i < Length(x3); i++)
	{
	    x3[i] = 0.0;

	    for (int j = 0; j < 3; j++)
		x3[i] += xa[(i * 3) + j] / 3.0;
	}

	// x4 = xa << 4
	for (unsigned int i = 0; i < Length(x4); i++)
	{
	    x4[i] = 0.0;

	    for (int j = 0; j < 4; j++)
		x2[i] += xa[(i * 4) + j] / 4.0;
	}

	// x5 = xa << 5
	for (unsigned int i = 0; i < Length(x5); i++)
	{
	    x5[i] = 0.0;

	    for (int j = 0; j < 5; j++)
		x5[i] += xa[(i * 5) + j] / 5.0;
	}

	// Add downsamples
	for (unsigned int i = 1; i < Length(xa); i++)
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
        // Clear maxima
        maxima[count].f  = 0.0;
        maxima[count].fr = 0.0;
        maxima[count].n  = 0;

        values[count] = 0.0;

        // Cents relative to reference
        double cf = -12.0 * log2(audio.reference / xf[i]);

        // Note number
        int note = round(cf) + C5_OFFSET;

        // Don't use if negative
        if (note < 0)
            continue;

        // Fundamental filter
        if (audio.fund && (count > 0) &&
            ((note % OCTAVE) != (maxima[0].n % OCTAVE)))
            continue;

        // Note filter
        if (audio.note)
        {
            // Get note and octave
            int n = note % OCTAVE;
            int o = note / OCTAVE;

            // Ignore too high
            if (o >= Length(filter.octave))
                continue;

            // Ignore if filtered
            if (!filter.note[n] ||
                !filter.octave[o])
                continue;
        }

        // Find maximum value
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
            // Frequency
	    maxima[count].f = xf[i];

	    // Note number
	    maxima[count].n = note;

	    // Octave note number
	    int n = (note - audio.key + OCTAVE) % OCTAVE;
	    // A note number
	    int a = (A_OFFSET - audio.key + OCTAVE) % OCTAVE;

	    // Temperament ratio
	    double temperRatio = temperaments[audio.temperament][n] /
	      temperaments[audio.temperament][a];
	    // Equal ratio
	    double equalRatio = temperaments[EQUAL][n] /
	      temperaments[EQUAL][a];

	    // Temperament adjustment
	    double temperAdjust = temperRatio / equalRatio;

	    // Reference note
	    maxima[count].fr = audio.reference *
                pow(2.0, round(cf) / 12.0) * temperAdjust;

	    // Set limit to octave above
	    if (!audio.down && (limit > i * 2))
		limit = i * 2 - 1;

	    count++;
	}
    }

    // Reference note frequency and lower and upper limits
    double fr = 0.0;
    double fl = 0.0;
    double fh = 0.0;

    // Note number
    int note = 0;

    // Found flag and cents value
    BOOL found = false;
    double c = 0.0;

    // Do the note and cents calculations
    if (max > MIN)
    {
	found = true;

	// Frequency
	if (!audio.down)
	    f = maxima[0].f;

	// Cents relative to reference
	double cf =
	    -12.0 * log2(audio.reference / f);

        // Don't count silly values
        if (isnan(cf))
        {
            cf = 0.0;
            found = false;
        }

	// Note number
	note = round(cf) + C5_OFFSET;

	if (note < 0)
	    found = false;

	// Octave note number
	int n = (note - audio.key + OCTAVE) % OCTAVE;
	// A note number
	int a = (A_OFFSET - audio.key + OCTAVE) % OCTAVE;

	// Temperament ratio
	double temperRatio = temperaments[audio.temperament][n] /
            temperaments[audio.temperament][a];
	// Equal ratio
	double equalRatio = temperaments[EQUAL][n] /
            temperaments[EQUAL][a];

        // Temperament adjustment
        double temperAdjust = temperRatio / equalRatio;

	// Reference note
	fr = audio.reference * pow(2.0, round(cf) / 12.0) * temperAdjust;

	// Lower and upper freq
	fl = audio.reference * pow(2.0, (round(cf) - 0.55) /
                                   12.0) * temperAdjust;
	fh = audio.reference * pow(2.0, (round(cf) + 0.55) /
                                   12.0) * temperAdjust;

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
	    found = false;
    }

    // If display not locked
    if (!display.lock)
    {
	// Update scope window
	scope.data = data;
	InvalidateRgn(scope.hwnd, NULL, true);

	// Update spectrum window
	for (int i = 0; i < count; i++)
	    values[i] = maxima[i].f / fps;

	spectrum.count = count;

	if (found)
	{
	    spectrum.f = f  / fps;
	    spectrum.r = fr / fps;
	    spectrum.l = fl / fps;
	    spectrum.h = fh / fps;
	}

	InvalidateRgn(spectrum.hwnd, NULL, true);
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
	    display.n = note;
	    display.count = count;

	    // Update strobe
	    strobe.c = c;

	    // Update staff
	    staff.n = note;

	    // Update meter
	    meter.c = c;
	}

	// Update display and staff
	InvalidateRgn(display.hwnd, NULL, true);
	InvalidateRgn(staff.hwnd, NULL, true);

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

		// Update strobe
		strobe.c = 0.0;

		// Update staff
		staff.n = 0.0;

		// Update meter
		meter.c = 0.0;

		// Update spectrum
		spectrum.f = 0.0;
		spectrum.r = 0.0;
		spectrum.l = 0.0;
		spectrum.h = 0.0;
	    }

	    // Update display and staff
	    InvalidateRgn(display.hwnd, NULL, true);
	    InvalidateRgn(staff.hwnd, NULL, true);
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
