// SimpleGrid.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SimpleGrid.h"
#include "Windowsx.h"

#define MAX_LOADSTRING 100

#define COL_SPACING 100

// Global Variables:
HINSTANCE hInst;								// current instance
HWND hWnd;
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
HPEN gridlinesPen;

HBRUSH solidWhiteBrush;

HDC offscreenDC;
HBITMAP offscreenBitmap;
RECT totalArea;

void DrawGridElements(HDC hdc, RECT client);

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

	INITCOMMONCONTROLSEX icex;

    // Ensure that the common control DLL is loaded. 
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_COOL_CLASSES | ICC_DATE_CLASSES | ICC_TAB_CLASSES;

	InitCommonControlsEx(&icex);


	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIMPLEGRID));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg) && !IsDialogMessage(hWnd, &msg))
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

	delegate = new ReferenceDelegate();

	for(int i=0; i<delegate->totalColumns(); i++){
		addColumn(delegate->headerContent(i), delegate->columnWidth(i));
		totalWidth += delegate->columnWidth(i);
	}

	totalHeight = (delegate->totalRows() + 1) * delegate->rowHeight() + delegate->totalRows();

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
	gridlinesPen = CreatePen(PS_SOLID, 1, RGB(210, 210, 210));
	solidWhiteBrush = CreateSolidBrush(RGB(255,255,255));
	offscreenDC = CreateCompatibleDC(GetDC(hWnd));
	//TODO: This attempts to draw the entire grid into memory.  This could be a problem for large grids.
	offscreenBitmap = CreateCompatibleBitmap(GetDC(hWnd), totalWidth, totalHeight);
	SelectObject(offscreenDC, offscreenBitmap);


	totalArea.left = 0;
	totalArea.right = totalWidth;
	totalArea.bottom = totalHeight;
	totalArea.top = 0;
	
	DrawGridElements(offscreenDC, totalArea);

	//SetScroll(hWnd);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


void DrawHeaderDragGuideline(HDC hdc, RECT client)
{
	if ( draggingHeader == true ){
		SelectObject(hdc, headerPen);	
	
		MoveToEx(hdc, draggedXPos, 0, NULL);
		LineTo(hdc, draggedXPos, client.bottom);
	}
}

void DrawHeader(HDC hdc, RECT client){
	if ( delegate->stickyHeaders() ){
		POINT origin;
		GetWindowOrgEx(hdc, &origin);
		SetWindowOrgEx(hdc, origin.x , origin.y - scrollOffsetY, 0);
		OffsetRect(&client, 0, -scrollOffsetY);
	}

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
		MoveToEx(hdc, left+i-1, 0, NULL);
		LineTo(hdc, left+i-1, delegate->rowHeight());
		TRIVERTEX vertex[2] ;
		vertex[0].x     = left;
		vertex[0].y     = 1;
		vertex[0].Red   = 0xf000;
		vertex[0].Green = 0xf000;
		vertex[0].Blue  = 0xf000;
		vertex[0].Alpha = 0x0000;

		vertex[1].x     = left+i-1;
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
		SetWindowOrgEx(hdc, origin.x, origin.y + scrollOffsetY, 0);
		OffsetRect(&client, 0, scrollOffsetY);
	}

}

void DrawVerticalGridlines(HDC hdc, RECT client)
{
	SelectObject(hdc, gridlinesPen);

	if ( delegate->drawVerticalGridlines() ){
		int left = 0;
		for(int i=0; i<numColumns; i++){
			GridColumn* col = columns[i];
			MoveToEx(hdc, left + col->getWidth()-1, delegate->rowHeight()+1, NULL);
			LineTo(hdc, left + col->getWidth()-1, (delegate->totalRows()+1)*delegate->rowHeight());
			left += col->getWidth();
		}
	} else {
		MoveToEx(hdc, totalWidth, 0, NULL);
		LineTo(hdc, totalWidth, totalHeight);
	}

}

void DrawHorizontalGridlines(HDC hdc, RECT client)
{
	SelectObject(hdc, gridlinesPen);
	if( delegate->drawHorizontalGridlines() ){
		for(int i=1; i<delegate->totalRows()+2; i+=1){
			MoveToEx(hdc, 0, i*delegate->rowHeight(), NULL);
			LineTo(hdc, totalWidth, i*delegate->rowHeight());
		}
	} else {
		//Always draw bottom gridline
		MoveToEx(hdc, 0, totalHeight-1, NULL);
		LineTo(hdc, totalWidth, totalHeight-1);
	}
}

void DrawTextForRow(HDC hdc, RECT client, int row){
	int left = 0;
	int top = delegate->rowHeight() * (row+1) + 1;
	for(int col = 0; col<delegate->totalColumns(); col++){
		GridColumn* c = columns[col];
		RECT textRect;
		textRect.left = left+LEFT_MARGIN;
		textRect.right = left + c->getWidth()-1;
		textRect.top = top+1;
		textRect.bottom = top + delegate->rowHeight()-1;
		left += c->getWidth();
		FillRect(hdc, &textRect, solidWhiteBrush); 
		DrawText(hdc, delegate->cellContent(row, col), -1, &textRect, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	}
}

void ClearActiveRow(int row, HDC hdc, RECT client)
{
	RECT rect;
	rect.left = 0;
	rect.right = totalWidth;
	rect.top = row * delegate->rowHeight();
	rect.bottom = rect.top + delegate->rowHeight();
	FillRect(hdc, &rect, solidWhiteBrush);
	//Gridlines
	if ( delegate->drawHorizontalGridlines()){
		DrawHorizontalGridlines(hdc, client);
	}
	if ( delegate->drawVerticalGridlines()){
		DrawVerticalGridlines(hdc, client);
	}
	DrawTextForRow(hdc, client, row-1);
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
	SetBkMode(hdc, TRANSPARENT);

	HFONT hFont = delegate->getFont();
	SelectObject(hdc, hFont);
		
	int j = 0;
	int left = 0;
	for(int row = 0; row<delegate->totalRows(); row++){
		left = 0;
		DrawTextForRow(hdc, client, row);
	}
}

LRESULT CALLBACK DetailWndProc(HWND hWnd, UINT message, WPARAM wParam,
                               LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if ( message == WM_KILLFOCUS ){
		delegate->editingFinished(hWnd, activeRow, uIdSubclass);
	}
	return DefSubclassProc(hWnd, message, wParam, lParam);
}

void startEditing(int previous, int row){
	int left = 0;
	HWND previousWindow = HWND_TOP;
	for(int i=0; i<numColumns; i++){
		if( delegate->allowEditing(i) ){
			if ( columns[i]->getEditor() == NULL ){
				HWND editor = delegate->editorForColumn(i, hWnd, hInst);
				SendMessage(editor, WM_SETFONT, (WPARAM)delegate->getEditFont(), 0);
				SetWindowSubclass(editor, DetailWndProc, i, 0);
				columns[i]->setEditor(editor);
			} else {
				delegate->editingFinished(columns[i]->getEditor(), previous, i); 
			}
			HWND editor = columns[i]->getEditor();
			delegate->setupEditorForCell(editor, row, i);
			SetWindowPos(editor, previousWindow, left-scrollOffsetX, (row+1) * delegate->rowHeight() - scrollOffsetY, columns[i]->getWidth(), delegate->rowHeight(), 0);
			ShowWindow(editor, SW_SHOW);
			previousWindow = editor;
		}
		left += columns[i]->getWidth();
	}
}



void scrollEditors(int offsetX, int offsetY){
	for(int i=0; i<numColumns; i++){
		if ( columns[i]->getEditor() != NULL ){
			RECT editor;
			GetWindowRect(columns[i]->getEditor(), &editor);
			MapWindowPoints(NULL, hWnd, (LPPOINT)&editor, 2);
			SetWindowPos(columns[i]->getEditor(), HWND_TOP, editor.left - offsetX, editor.top - offsetY, 0, 0, SWP_NOSIZE);
		}
	}
}


void DrawGridElements(HDC hdc, RECT client)
{
	FillRect(offscreenDC, &totalArea, solidWhiteBrush);

	DrawVerticalGridlines(hdc, client);
	DrawHorizontalGridlines(hdc, client);
	DrawCellText(hdc, client);

	DrawActiveRow(hdc, client);
	DrawHeaderDragGuideline(hdc, client);

		
	DrawHeader(hdc, client);
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
	case WM_KILLFOCUS:
		OutputDebugString(L"Lost focus\n");
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

				RECT invalidate1;
				invalidate1.left = draggedXPos - 1;
				invalidate1.right = draggedXPos + 1;
				invalidate1.top = 0;
				invalidate1.bottom = client.bottom;


				draggedXPos = mouseXPos;
				RECT invalidate;
				invalidate.left = mouseXPos - 1;
				invalidate.right = mouseXPos + 1;
				invalidate.top = 0;
				invalidate.bottom = client.bottom;

				InvalidateRect(hWnd, &invalidate1, true);
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
			totalWidth = 0;
			for(int i=0; i<activelyDraggedColumn; i++){
				accum += columns[i]->getWidth();
			}

			columns[activelyDraggedColumn]->setWidth(mouseXPos - accum);	
			
			totalWidth = 0;
			for(int i=0; i<numColumns; i++){
				totalWidth += columns[i]->getWidth();
			}

			totalArea.right = totalWidth;
			offscreenBitmap = CreateCompatibleBitmap(GetDC(hWnd), totalWidth, totalHeight);
			SelectObject(offscreenDC, offscreenBitmap);

			DrawGridElements(offscreenDC, totalArea);

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
	
				if( delegate->rowSelection() ){
	
				
					RECT r2;
					r2.left = 0;
					r2.right = client.right;
					r2.top = previousActiveRow * delegate->rowHeight() - 2;
					r2.bottom = repaint.top + delegate->rowHeight() + 4;
					//InvalidateRect(hWnd, NULL, true);

					repaint.left = 0;
					repaint.right = client.right;
					repaint.top = activeRow * delegate->rowHeight() - 2;
					repaint.bottom = repaint.top + delegate->rowHeight() + 4;
					//InvalidateRect(hWnd, &repaint, true);
					ClearActiveRow(previousActiveRow, offscreenDC, client);
					DrawActiveRow(offscreenDC, client);
					//InvalidateRect(hWnd, &repaint, true);
					//InvalidateRect(hWnd, &r2, true);
					InvalidateRect(hWnd, NULL, true);

				}
				startEditing(previousActiveRow-1, activeRow-1);

				if( previousActiveRow != -1 ){
					DrawTextForRow(offscreenDC, client, previousActiveRow-1); 
					InvalidateRect(hWnd, NULL, true);
				}
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
		} else {
			overflowX = false;
		}
		if ( totalHeight > client.bottom ){
			overflowY = true;
		} else {
			overflowY = false;
		}
		if ( overflowX){
			ShowScrollBar(hWnd, SB_HORZ, true);
		} else {
			ShowScrollBar(hWnd, SB_HORZ, false);
		}
		if ( overflowY ){
			ShowScrollBar(hWnd, SB_VERT, true);
		} else {
			ShowScrollBar(hWnd, SB_VERT, false);
		}

		SetScroll(hWnd);
		break;
	case WM_PAINT:
		{
		
		hdc = BeginPaint(hWnd, &ps);
		
		//POINT origin;
		//GetWindowOrgEx(hdc, &origin);
		//SetWindowOrgEx(hdc, origin.x + scrollOffsetX, origin.y + scrollOffsetY, 0);

		//OffsetRect(&ps.rcPaint, scrollOffsetX, scrollOffsetY);


		BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, offscreenDC, ps.rcPaint.left + scrollOffsetX, ps.rcPaint.top + scrollOffsetY, SRCCOPY);
		
		if( draggingHeader ){
			DrawHeaderDragGuideline(hdc, ps.rcPaint);
		}

		EndPaint(hWnd, &ps);

		//SetWindowOrgEx(hdc, origin.x, origin.y, 0);
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
			if ( scrollOffsetY < ypos) {
				scrollEditors(0, scrollOffsetY + ypos);
			}else{
				scrollEditors(0, scrollOffsetY - ypos);
			}
			scrollOffsetY = ypos;
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_PAGEDOWN ){
			scrollOffsetY += client.bottom;
			if( scrollOffsetY > totalHeight - client.bottom ){
				scrollOffsetY = totalHeight - client.bottom;
			} else {
				scrollEditors(0, client.bottom);				
			}
			SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_PAGEUP ){
			scrollOffsetY -= client.bottom;
			if( scrollOffsetY < 0 ) {
				scrollOffsetY = 0;
			} else {
				scrollEditors(0, 0 - client.bottom);
			}
			SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_LINEDOWN ){
			scrollOffsetY += delegate->rowHeight();
			if( scrollOffsetY > totalHeight - client.bottom ){
				scrollOffsetY = totalHeight - client.bottom;
			}	
			SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
			scrollEditors(0, delegate->rowHeight());
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_LINEUP ){
			scrollOffsetY -= delegate->rowHeight();
			if( scrollOffsetY < 0 ){
				scrollOffsetY = 0;
			} else {
				scrollEditors(0, 0 - delegate->rowHeight());
				SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
				InvalidateRect(hWnd, NULL, true);
			}
			
		}
		return 0;
		}
	case WM_HSCROLL:
		{
		int xpos = HIWORD(wParam);
		int cmd = LOWORD(wParam);
		if ( cmd == SB_THUMBPOSITION ){
			SetScrollPos(hWnd, SB_HORZ, xpos, true);
			if ( scrollOffsetX < xpos) {
				scrollEditors(scrollOffsetX - xpos, 0);
			}else{
				scrollEditors(scrollOffsetX + xpos, 0);
			}
			scrollOffsetX = xpos;
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_PAGERIGHT ){
			RECT client;
			GetClientRect(hWnd, &client);
			scrollOffsetX += client.right;
			if( scrollOffsetX > totalWidth - client.right ){
				scrollOffsetX = totalWidth - client.right;
			} else {
				scrollEditors(client.right, 0);
			}
			SetScrollPos(hWnd, SB_HORZ, scrollOffsetX, true);
			InvalidateRect(hWnd, NULL, true);
		
		} else if ( cmd == SB_PAGELEFT ){
			RECT client;
			GetClientRect(hWnd, &client);
			scrollOffsetX -= client.right;
			if( scrollOffsetX < 0 ) { 
				scrollOffsetX = 0;
			} else {
				scrollEditors(-client.right, 0);
			}
			SetScrollPos(hWnd, SB_HORZ, scrollOffsetX, true);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_LINELEFT ) {
			RECT client;
			GetClientRect(hWnd, &client);
			scrollOffsetX -= 10;
			if ( scrollOffsetX < 0 ) {
				scrollOffsetX = 0;
			} else {
				scrollEditors(-10, 0);
			}
			SetScrollPos(hWnd, SB_HORZ, scrollOffsetX, true);
			InvalidateRect(hWnd, NULL, true);

		} else if ( cmd == SB_LINERIGHT ){

			RECT client;
			GetClientRect(hWnd, &client);
			scrollOffsetX += 10;
			if( scrollOffsetX > totalWidth - client.right ){
				scrollOffsetX = totalWidth - client.right;
			} else {
				scrollEditors(10, 0);
			}
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


