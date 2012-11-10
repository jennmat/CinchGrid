// SimpleGrid.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SimpleGrid.h"
#include "Windowsx.h"

#define MAX_LOADSTRING 100

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
int totalHeight = 0;

bool draggingHeader = false;
int draggedXPos = 0;

int activelyDraggedColumn = -1;

HPEN headerPen;

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
	si.nMax   = delegate->totalRows() * delegate->rowHeight();
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
		totalWidth += delegate->columnWidth(i);
	}

	totalHeight = (delegate->totalRows() + 1) * delegate->rowHeight();

	RECT client;
	GetClientRect(hWnd, &client);

	overflowY = false;
	
	overflowX = false;
	for(int i=1; i<delegate->totalRows()+2; i+=1){
		if ( i * delegate->rowHeight() > client.bottom ){
			//overflow
			overflowY = true;
			break;
		}
	}

	int left = 0;
	for(int i=0; i<numColumns; i++){
		GridColumn* col = columns[i];
		left += col->getWidth();
	}
		 
	if ( left > client.right ){
		overflowX = true;
	}

	headerPen = CreatePen(PS_SOLID, 1, RGB(210, 210, 210));
	

   //SetScroll(hWnd);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   
   return TRUE;
}

void DrawHeaderDragGuideline(HDC hdc, RECT client)
{
	if ( draggingHeader == true ){
		HPEN headerPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
		SelectObject(hdc, headerPen);	
	
		MoveToEx(hdc, draggedXPos, 0, NULL);
		LineTo(hdc, draggedXPos, client.bottom);
	}
}

void DrawHeader(HDC hdc, RECT client){

	
	if ( delegate->stickyHeaders() ){
		POINT origin;
		GetWindowOrgEx(hdc, &origin);
		SetWindowOrgEx(hdc, origin.x - scrollOffsetX, origin.y - scrollOffsetY, 0);
		OffsetRect(&client, -scrollOffsetX, -scrollOffsetY);
	}


	HPEN headerPen = CreatePen(PS_SOLID, 1, RGB(165, 172, 181));
	SelectObject(hdc, headerPen);	
		

	MoveToEx(hdc, 0, 0, NULL);
	LineTo(hdc, totalWidth, 0);
	MoveToEx(hdc, 0, delegate->rowHeight(), NULL);
	LineTo(hdc, totalWidth, delegate->rowHeight());

	int j = 0;
	int left = 0;
	for(int l=0; l<numColumns; l++){
		GridColumn* col = columns[l];
		int i = col->getWidth();
		//Rectangle(hdc, i, 0, i + COL_SPACING, delegate->rowHeight());
		MoveToEx(hdc, left+i, 0, NULL);
		LineTo(hdc, left+i, delegate->rowHeight());
		TRIVERTEX vertex[2] ;
		vertex[0].x     = left+1;
		vertex[0].y     = 1;
		vertex[0].Red   = 0xf000;
		vertex[0].Green = 0xf000;
		vertex[0].Blue  = 0xf000;
		vertex[0].Alpha = 0x0000;

		vertex[1].x     = left+i;
		vertex[1].y     = delegate->rowHeight(); 
		vertex[1].Red   = 0xe000;
		vertex[1].Green = 0xe000;
		vertex[1].Blue  = 0xe000;
		vertex[1].Alpha = 0x0000;

		GRADIENT_RECT r;
		r.UpperLeft = 0;
		r.LowerRight = 1;

		GradientFill(hdc, vertex, 2, &r, 1, GRADIENT_FILL_RECT_V);

		RECT headerText;
		headerText.left = left + 6;
		headerText.top = 1;
		headerText.bottom = delegate->rowHeight();
		headerText.right = left + i;

		DrawText(hdc, col->getHeader(), -1, &headerText, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			
		left += i;
		j++;
	}

	if ( delegate->stickyHeaders() ){
		POINT origin;
		GetWindowOrgEx(hdc, &origin);
		SetWindowOrgEx(hdc, origin.x + scrollOffsetX, origin.y + scrollOffsetY, 0);
		OffsetRect(&client, scrollOffsetX, scrollOffsetY);
	}

}

void DrawVerticalGridlines(HDC hdc, RECT client)
{
	SelectObject(hdc, headerPen);

	if ( delegate->drawVerticalGridlines() ){
		int left = 0;
		for(int i=0; i<numColumns; i++){
			GridColumn* col = columns[i];
			MoveToEx(hdc, left + col->getWidth(), delegate->rowHeight()+1, NULL);
			LineTo(hdc, left + col->getWidth(), (delegate->totalRows()+1)*delegate->rowHeight());
			left += col->getWidth();
		}
	} else {
		MoveToEx(hdc, totalWidth, 0, NULL);
		LineTo(hdc, totalWidth, totalHeight);
	}

}

void DrawHorizontalGridlines(HDC hdc, RECT client)
{
	if( delegate->drawHorizontalGridlines() ){
		for(int i=1; i<delegate->totalRows()+2; i+=1){
			MoveToEx(hdc, 0, i*delegate->rowHeight(), NULL);
			LineTo(hdc, totalWidth, i*delegate->rowHeight());
		}
	} else {
		//Always draw bottom gridline
		MoveToEx(hdc, 0, totalHeight, NULL);
		LineTo(hdc, totalWidth, totalHeight);
	}
}

void DrawTextForRow(HDC hdc, RECT client, int row){
	int left = 0;
	int top = delegate->rowHeight() * (row+1) + 1;
	if ( top < client.bottom ){
		for(int col = 0; col<delegate->totalColumns(); col++){
			GridColumn* c = columns[col];
			RECT textRect;
			textRect.left = left+7;
			textRect.right = left + c->getWidth();
			textRect.top = top;
			textRect.bottom = top + delegate->rowHeight();
			left += c->getWidth();
			
			if ( textRect.left < client.right ) {
				DrawText(hdc, delegate->cellContent(row, col), -1, &textRect, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			}
		}
	}

}

void DrawActiveRow(HDC hdc, RECT client)
{
	if ( activeRow >= 1){
			
		LOGBRUSH lb = {BS_SOLID, RGB(100, 100, 100), 0}; 
		HPEN activeCellPen = ExtCreatePen(PS_COSMETIC | PS_ALTERNATE | PS_ENDCAP_SQUARE | PS_JOIN_ROUND, 1, &lb, 0, NULL);

		SelectObject(hdc, activeCellPen);
		SelectObject(hdc,GetStockObject(NULL_BRUSH));
		HBRUSH brush = CreateSolidBrush(RGB(167,205,240));
		RECT row;
		row.left = 0;
		row.right = totalWidth;
		row.top = activeRow * delegate->rowHeight();
		row.bottom = row.top + delegate->rowHeight();
		FillRect(hdc, &row, brush);
		Rectangle(hdc, row.left, row.top, row.right, row.bottom);
		DrawTextForRow(hdc, client, activeRow-1);
	}
}

void DrawCellText(HDC hdc, RECT client)
{
	int j = 0;
	int left = 0;
	int top = delegate->rowHeight();
	for(int row = 0; row<delegate->totalRows(); row++){
		left = 0;
		DrawTextForRow(hdc, client, row);
		top += delegate->rowHeight();
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
				//InvalidateRect(hWnd, NULL, true);
				RECT client;
				GetClientRect(hWnd, &client);

				RECT invalidate;
				invalidate.left = draggedXPos - 1;
				invalidate.right = draggedXPos + 1;
				invalidate.top = 0;
				invalidate.bottom = client.bottom;

				InvalidateRect(hWnd, &invalidate, true);

				draggedXPos = mouseXPos;
				
				invalidate.left = mouseXPos - 1;
				invalidate.right = mouseXPos + 1;
				invalidate.top = 0;
				invalidate.bottom = client.bottom;

				InvalidateRect(hWnd, &invalidate, true);
			}
		}
		if ( mouseYPos < delegate->rowHeight() ){
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
			int accum = 0;
			int mouseXPos = GET_X_LPARAM(lParam); 
			for(int i=0; i<activelyDraggedColumn; i++){
				accum += columns[i]->getWidth();
			}

			columns[activelyDraggedColumn]->setWidth(mouseXPos - accum);	
			InvalidateRect(hWnd, NULL, true);
		}
		draggingHeader = false;

		break;
	case WM_LBUTTONDOWN:
		{
		RECT repaint;
		repaint.left = activeCol * COL_SPACING - 2;
		repaint.right = repaint.left + COL_SPACING + 4;
		repaint.top = activeRow * delegate->rowHeight() - 2;
		repaint.bottom = repaint.top + delegate->rowHeight() + 4;
		//InvalidateRect(hWnd, NULL, true);

		int xPos = GET_X_LPARAM(lParam); 
		int yPos = GET_Y_LPARAM(lParam); 

		if ( yPos < delegate->rowHeight() ){
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
			if ( (yPos + scrollOffsetY) / delegate->rowHeight() < delegate->totalRows() + 1 ){
				int previousActiveRow = activeRow;
				activeRow = (yPos + scrollOffsetY) / delegate->rowHeight();
				RECT client;
				GetClientRect(hWnd, &client);

				RECT r2;
				r2.left = 0;
				r2.right = client.right;
				r2.top = previousActiveRow * delegate->rowHeight() - 2 - scrollOffsetY;
				r2.bottom = repaint.top + delegate->rowHeight() + 4 - scrollOffsetY;
				//InvalidateRect(hWnd, &r2, true);
				//InvalidateRect(hWnd, NULL, true);

				repaint.left = 0;
				repaint.right = client.right;
				repaint.top = activeRow * delegate->rowHeight() - 2 - scrollOffsetY;
				repaint.bottom = repaint.top + delegate->rowHeight() + 4  - scrollOffsetY;
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
		RECT client;
		GetClientRect(hWnd, &client);
		if( totalWidth > client.right ){
			overflowX = true;
		}
		if ( totalHeight > client.bottom ){
			overflowY = true;
		}
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

		SetScroll(hWnd);
		break;
	case WM_PAINT:
		{
		
		hdc = BeginPaint(hWnd, &ps);
		
		POINT origin;
		GetWindowOrgEx(hdc, &origin);
		SetWindowOrgEx(hdc, origin.x + scrollOffsetX, origin.y + scrollOffsetY, 0);

		OffsetRect(&ps.rcPaint, scrollOffsetX, scrollOffsetY);

		HFONT hFont = delegate->getFont();
		SelectObject(hdc, hFont);
		SetBkMode(hdc, TRANSPARENT);

		
		DrawVerticalGridlines(hdc, ps.rcPaint);
		DrawHorizontalGridlines(hdc, ps.rcPaint);
		DrawCellText(hdc, ps.rcPaint);

		DrawActiveRow(hdc, ps.rcPaint);
		DrawHeaderDragGuideline(hdc, ps.rcPaint);

		
		DrawHeader(hdc, ps.rcPaint);	
		
		//BitBlt(h, 0, 0, window.right-window.left, window.bottom-window.top, hdc, 0, 0, SRCCOPY);
		
		EndPaint(hWnd, &ps);

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
			scrollOffsetY += delegate->rowHeight();
			SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_LINEUP ){
			scrollOffsetY -= delegate->rowHeight();
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


