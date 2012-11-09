// SimpleGrid.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SimpleGrid.h"
#include "Windowsx.h"

#define MAX_LOADSTRING 100

#define ROW_SPACING 25
#define COL_SPACING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int activeRow = -1;
int activeCol = -1;

int scrollOffsetX = 0;
int scrollOffsetY = 0;

bool overflowX = 0;
bool overflowY = 0;

int totalWidth = 0;

bool draggingHeader = false;
int activelyDraggedColumn = -1;

void addColumn(wchar_t * header, int width) {
	columns[numColumns] = new GridColumn(header, width);
	numColumns++;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SIMPLEGRID, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIMPLEGRID));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMPLEGRID));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SIMPLEGRID);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}



void SetScroll(HWND hWnd)
{
	RECT client;
	GetClientRect(hWnd, &client);
		
		
	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE;
    si.nMin   = 0;
	si.nMax   = delegate->totalRows() * ROW_SPACING;
	si.nPage  = client.bottom;
	SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

	SCROLLINFO hsi;
	ZeroMemory(&si, sizeof(hsi));
    hsi.cbSize = sizeof(si);
    hsi.fMask  = SIF_RANGE | SIF_PAGE;
    hsi.nMin   = 0;
	hsi.nMax   = totalWidth;
	hsi.nPage  = client.right;
	SetScrollInfo(hWnd, SB_HORZ, &hsi, TRUE);

	//ShowScrollBar(hWnd, SB_VERT, true);
	//ShowScrollBar(hWnd, SB_HORZ, true);
			
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
	
   //SetScrollRange(hWnd, SB_VERT, 0, 1, true);
	//SetScrollRange(hWnd, SB_HORZ, 0, 1, true);
		

	ShowScrollBar(hWnd, SB_BOTH, false);

   delegate = new HardCodedDelegate();

   for(int i=0; i<delegate->totalColumns(); i++){
	   addColumn(delegate->headerContent(i), delegate->columnWidth(i));
   }

   //SetScroll(hWnd);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   
   return TRUE;
}

void DrawTextForRow(HDC hdc, RECT client, int row){
	int left = 0;
	int top = ROW_SPACING * (row+1) + 1;
	if ( top < client.bottom ){
		for(int col = 0; col<delegate->totalColumns(); col++){
			GridColumn* c = columns[col];
			RECT textRect;
			textRect.left = left+7;
			textRect.right = left + c->getWidth();
			textRect.top = top;
			textRect.bottom = top + ROW_SPACING;
			left += c->getWidth();
			
			if ( textRect.left < client.right ) {
				DrawText(hdc, delegate->cellContent(row, col), -1, &textRect, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			}
		}
	}

}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;


	switch (message)
	{
	case WM_ACTIVATE:
		break;
	case WM_MOUSEMOVE:
		{
		int mouseXPos = GET_X_LPARAM(lParam); 
		int mouseYPos = GET_Y_LPARAM(lParam); 
		
		if ( draggingHeader ){
			int accum = 0;
			for(int i=0; i<activelyDraggedColumn; i++){
				accum += columns[i]->getWidth();
			}
			if ( mouseXPos - accum > 10 ){
				columns[activelyDraggedColumn]->setWidth(mouseXPos - accum);	
				//InvalidateRect(hWnd, NULL, true);

			}
		}
		if ( mouseYPos < ROW_SPACING ){
			int accum = 0;
			for(int i=0; i<numColumns; i++){
				accum += columns[i]->getWidth();
				if ( abs(accum-mouseXPos) < 10 ){
					SetCursor(LoadCursor(NULL, IDC_SIZEWE));
				}
			}
		
		}
		break;
		}
	case WM_LBUTTONUP:
		if ( draggingHeader == true ){
			InvalidateRect(hWnd, NULL, true);
		}
		draggingHeader = false;

		break;
	case WM_LBUTTONDOWN:
		{
		RECT repaint;
		repaint.left = activeCol * COL_SPACING - 2;
		repaint.right = repaint.left + COL_SPACING + 4;
		repaint.top = activeRow * ROW_SPACING - 2;
		repaint.bottom = repaint.top + ROW_SPACING + 4;
		//InvalidateRect(hWnd, NULL, true);

		int xPos = GET_X_LPARAM(lParam); 
		int yPos = GET_Y_LPARAM(lParam); 

		if ( yPos < ROW_SPACING ){
			int accum = 0;
			for(int i=0; i<numColumns; i++){
				accum += columns[i]->getWidth();
				if ( abs(accum-xPos) < 10 ){
					draggingHeader = true;
					activelyDraggedColumn = i;
				}
			}
		} else {

			int accum = 0;
			for(int i=0; i<numColumns; i++){
				accum += columns[i]->getWidth();
				if ( accum > xPos ){
					activeCol = i;
					break;
				}
			}
			if ( (yPos + scrollOffsetY) / ROW_SPACING < delegate->totalRows() + 1 ){
				int previousActiveRow = activeRow;
				activeRow = (yPos + scrollOffsetY) / ROW_SPACING;
				RECT client;
				GetClientRect(hWnd, &client);

				RECT r2;
				r2.left = 0;
				r2.right = client.right;
				r2.top = previousActiveRow * ROW_SPACING - 2 - scrollOffsetY;
				r2.bottom = repaint.top + ROW_SPACING + 4 - scrollOffsetY;
				//InvalidateRect(hWnd, &r2, true);
				//InvalidateRect(hWnd, NULL, true);

				repaint.left = 0;
				repaint.right = client.right;
				repaint.top = activeRow * ROW_SPACING - 2 - scrollOffsetY;
				repaint.bottom = repaint.top + ROW_SPACING + 4  - scrollOffsetY;
				//InvalidateRect(hWnd, &repaint, true);
				InvalidateRect(hWnd, NULL, true);


			}
			
		}
		}
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_SIZE:
		SetScroll(hWnd);
		break;
	case WM_PAINT:
		{
		RECT window;
		GetWindowRect(hWnd, &window);
		RECT client;
		GetClientRect(hWnd, &client);
		
		hdc = BeginPaint(hWnd, &ps);
		
		
		//HDC hdc;
		//HBITMAP bitmap;
		//hdc = CreateCompatibleDC(h);
		//bitmap = CreateCompatibleBitmap(hdc, window.right-window.left, window.bottom-window.top);
		//SelectObject(hdc, bitmap);
		
		POINT origin;
		GetWindowOrgEx(hdc, &origin);
		SetWindowOrgEx(hdc, origin.x + scrollOffsetX, origin.y + scrollOffsetY, 0);

		OffsetRect(&ps.rcPaint, scrollOffsetX, scrollOffsetY);


		RECT rect;
		GetClientRect(hWnd, &rect);
		FillRect(hdc, &rect, CreateSolidBrush(RGB(255,255,255)));
		// TODO: Add any drawing code here...
		
		//RECT rect;
		//rect.left = 0;
		//rect.right = window.right - window.left;
		//rect.top = 0;
		//rect.bottom = ROW_SPACING;
		//FillRect(hdc, &rect, CreateSolidBrush(RGB(120,120,120)));
		
		

		HFONT hFont = delegate->getFont();
		SelectObject(hdc, hFont);
		SetBkMode(hdc, TRANSPARENT);

		
		HPEN headerPen = CreatePen(PS_SOLID, 1, RGB(165, 172, 181));
		SelectObject(hdc, headerPen);	
		
		totalWidth = 0;
		for(int i=0; i<numColumns; i++){
			totalWidth += columns[i]->getWidth();
		}

		MoveToEx(hdc, 0, 0, NULL);
		LineTo(hdc, totalWidth, 0);
		MoveToEx(hdc, 0, ROW_SPACING, NULL);
		LineTo(hdc, totalWidth, ROW_SPACING);

		int j = 0;
		int left = 0;
		for(int l=0; l<numColumns; l++){
			GridColumn* col = columns[l];
			int i = col->getWidth();
			//Rectangle(hdc, i, 0, i + COL_SPACING, ROW_SPACING);
			MoveToEx(hdc, left+i, 0, NULL);
			LineTo(hdc, left+i, ROW_SPACING);
			TRIVERTEX vertex[2] ;
			vertex[0].x     = left+1;
			vertex[0].y     = 1;
			vertex[0].Red   = 0xf400;
			vertex[0].Green = 0xf700;
			vertex[0].Blue  = 0xf900;
			vertex[0].Alpha = 0x0000;

			vertex[1].x     = left+i;
			vertex[1].y     = ROW_SPACING; 
			vertex[1].Red   = 0xef00;
			vertex[1].Green = 0xf200;
			vertex[1].Blue  = 0xf600;
			vertex[1].Alpha = 0x0000;

			GRADIENT_RECT r;
			r.UpperLeft = 0;
			r.LowerRight = 1;

			GradientFill(hdc, vertex, 2, &r, 1, GRADIENT_FILL_RECT_V);

			RECT headerText;
			headerText.left = left + 6;
			headerText.top = 1;
			headerText.bottom = ROW_SPACING;
			headerText.right = left + i;

			DrawText(hdc, col->getHeader(), -1, &headerText, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			
			left += i;
			j++;
		}

		int totalColumnWidth = left;

		HPEN hBluePen = CreatePen(PS_SOLID, 1, RGB(210, 210, 210));
		SelectObject(hdc, hBluePen);

		overflowY = false;
		overflowX = false;
		for(int i=1; i<delegate->totalRows()+2; i+=1){
			MoveToEx(hdc, 0, i*ROW_SPACING, NULL);
			LineTo(hdc, totalWidth, i*ROW_SPACING);
			if ( i * ROW_SPACING > ps.rcPaint.bottom ){
				//overflow
				overflowY = true;
				break;
			}
		}

		left = 0;
		for(int i=0; i<numColumns; i++){
			GridColumn* col = columns[i];
			MoveToEx(hdc, left + col->getWidth(), ROW_SPACING+1, NULL);
			LineTo(hdc, left + col->getWidth(), (delegate->totalRows()+1)*ROW_SPACING);
			left += col->getWidth();
		}

		if ( left > client.right - client.left ){
			overflowX = true;
		}

		j = 0;
		left = 0;
		int top = ROW_SPACING;
		for(int row = 0; row<delegate->totalRows(); row++){
			left = 0;
			DrawTextForRow(hdc, ps.rcPaint, row);
			top += ROW_SPACING;

		}

		
		if ( activeRow >= 1){
			
			LOGBRUSH lb = {BS_SOLID, RGB(100, 100, 100), 0}; 
			HPEN activeCellPen = ExtCreatePen(PS_COSMETIC | PS_ALTERNATE | PS_ENDCAP_SQUARE | PS_JOIN_ROUND, 1, &lb, 0, NULL);

			SelectObject(hdc, activeCellPen);
			SelectObject(hdc,GetStockObject(NULL_BRUSH));
			/*
			int accum = 0;
			for(int i=0; i<activeCol; i++){
				accum += columns[i]->getWidth();
			}
			int cx = accum + 1 + columns[activeCol]->getWidth();
			Rectangle(hdc, accum + 1 , activeRow * ROW_SPACING +1 , cx, (activeRow + 1) * ROW_SPACING);*/
			HBRUSH brush = CreateSolidBrush(RGB(167,205,240));
			RECT row;
			row.left = 0;
			row.right = totalColumnWidth;
			row.top = activeRow * ROW_SPACING;
			row.bottom = row.top + ROW_SPACING;
			FillRect(hdc, &row, brush);
			Rectangle(hdc, row.left, row.top, row.right, row.bottom);
			DrawTextForRow(hdc, ps.rcPaint, activeRow-1);
		}

		//BitBlt(h, 0, 0, window.right-window.left, window.bottom-window.top, hdc, 0, 0, SRCCOPY);

		EndPaint(hWnd, &ps);

		if ( overflowX || scrollOffsetX > 0){
			ShowScrollBar(hWnd, SB_HORZ, true);
		} else {
			ShowScrollBar(hWnd, SB_HORZ, false);
		}
		if ( overflowY || scrollOffsetY > 0 ){
			ShowScrollBar(hWnd, SB_VERT, true);
		} else {
			ShowScrollBar(hWnd, SB_VERT, false);
		}

		SetWindowOrgEx(hdc, origin.x, origin.y, 0);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_VSCROLL:
		{
		int ypos = HIWORD(wParam);
		int cmd = LOWORD(wParam);
		RECT client;
		GetClientRect(hWnd, &client);
			
		if ( cmd == SB_THUMBPOSITION ){
			SetScrollPos(hWnd, SB_VERT, ypos, true);
			scrollOffsetY = ypos;
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_PAGEDOWN ){
			scrollOffsetY += client.bottom;
			SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_PAGEUP ){
			scrollOffsetY -= client.bottom;
			if( scrollOffsetY < 0 ) scrollOffsetY = 0;
			SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_LINEDOWN ){
			scrollOffsetY += ROW_SPACING;
			SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_LINEUP ){
			scrollOffsetY -= ROW_SPACING;
			if( scrollOffsetY < 0 ) scrollOffsetY = 0;
			SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
			InvalidateRect(hWnd, NULL, true);
		
		}
		return 0;
		}
	case WM_HSCROLL:
		{
		int xpos = HIWORD(wParam);
		int cmd = LOWORD(wParam);
		if ( cmd == SB_THUMBPOSITION ){
			SetScrollPos(hWnd, SB_HORZ, xpos, true);
			scrollOffsetX = xpos;
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_PAGERIGHT ){
			RECT client;
			GetClientRect(hWnd, &client);
			scrollOffsetX += client.right;
			SetScrollPos(hWnd, SB_HORZ, scrollOffsetX, true);
			InvalidateRect(hWnd, NULL, true);
		
		} else if ( cmd == SB_PAGELEFT ){
			RECT client;
			GetClientRect(hWnd, &client);
			scrollOffsetX -= client.right;
			SetScrollPos(hWnd, SB_HORZ, scrollOffsetX, true);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_LINELEFT ) {
			RECT client;
			GetClientRect(hWnd, &client);
			scrollOffsetX -= 10;
			SetScrollPos(hWnd, SB_HORZ, scrollOffsetX, true);
			InvalidateRect(hWnd, NULL, true);

		} else if ( cmd == SB_LINERIGHT ){

			RECT client;
			GetClientRect(hWnd, &client);
			scrollOffsetX += 10;
			SetScrollPos(hWnd, SB_HORZ, scrollOffsetX, true);
			InvalidateRect(hWnd, NULL, true);
		}
		return 0;
		}
	case WM_MOUSEWHEEL:
		{
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		scrollOffsetY += 0 - zDelta;
		if ( scrollOffsetY >= 0 )  {
			SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
			InvalidateRect(hWnd, NULL, true);
		} else {
			scrollOffsetY = 0;
		}
		}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


