// SimpleGrid.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Windowsx.h"

#define MAX_LOADSTRING 100


TCHAR szClassName[] = _T("CinchGrid");
 
void RegisterCinchGrid()
{
    WNDCLASSEX wc;
 
    wc.cbSize         = sizeof(wc);
    wc.lpszClassName  = szClassName;
    wc.hInstance      = GetModuleHandle(0);
	wc.lpfnWndProc    = CinchGrid::WndProc;
    wc.hCursor        = LoadCursor (NULL, IDC_ARROW);
    wc.hIcon          = 0;
    wc.lpszMenuName   = 0;
	wc.hbrBackground  = (HBRUSH)CreateSolidBrush(RGB(255,255,255));
    wc.style          = 0;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hIconSm        = 0;
 
    RegisterClassEx(&wc);
}


HWND CinchGrid::CreateCinchGrid(HWND parent)
{
	RegisterCinchGrid();

	HWND hWnd = CreateWindowEx(0,
		szClassName,
		_T("Cinch Grid"),
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL,
		0, 0, 500, 500,
		parent,
		NULL, GetModuleHandle(0), NULL);

	return hWnd;
}


CinchGrid::CinchGrid(HWND h, HINSTANCE inst){

	numColumns = 0;

	hWnd = h;
	hInst = inst;
	activeRow = -1;
	activeCol = -1;

	scrollOffsetX = 0;
	scrollOffsetY = 0;

	overflowX = 0;
	overflowY = 0;

	totalWidth = 0;
	totalHeight = 0;

	draggingHeader = false;
	editingInitialized = false;
	draggedXPos = 0;

	activelyDraggedColumn = -1;

	delegate = new ReferenceDelegate();

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
	gridlinesPen = CreatePen(PS_SOLID, 1, RGB(210, 210, 210));
	solidWhiteBrush = CreateSolidBrush(RGB(255,255,255));
	activeRowBrush = CreateSolidBrush(RGB(222,235,250));
		
	offscreenDC = CreateCompatibleDC(GetDC(hWnd));
	//TODO: This attempts to draw the entire grid into memory.  This could be a problem for large grids.
	SetupAndDrawOffscreenBitmap();
}


void CinchGrid::addColumn(wchar_t * header, int width) {
	columns[numColumns] = new GridColumn(header, width);
	numColumns++;
}

void CinchGrid::SetScroll(HWND hWnd)
{
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
			
}


void CinchGrid::SetupAndDrawOffscreenBitmap(){
	RECT client;
 	GetClientRect(hWnd, &client);
	offscreenBitmap = CreateCompatibleBitmap(GetDC(hWnd), max(client.right, totalWidth), max(client.bottom, totalHeight));
	SelectObject(offscreenDC, offscreenBitmap);


	totalArea.left = 0;
	totalArea.right = max(client.right, totalWidth);
	totalArea.bottom = max(client.bottom, totalHeight);
	totalArea.top = 0;
	
	DrawGridElements(offscreenDC, totalArea);
}

void CinchGrid::DrawHeaderDragGuideline(HDC hdc, RECT client)
{
	if ( draggingHeader == true ){
		SelectObject(hdc, headerPen);	
	
		MoveToEx(hdc, draggedXPos, 0, NULL);
		LineTo(hdc, draggedXPos, client.bottom);
	}
}

void CinchGrid::DrawHeader(HDC hdc, RECT client){
	

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
}

void CinchGrid::DrawVerticalGridlines(HDC hdc, RECT client)
{
	SelectObject(hdc, gridlinesPen);
	
	if ( delegate->drawVerticalGridlines() ){
		int left = 0;
		int bottom = client.bottom;
		if ( !delegate->allowNewRows() ){
			bottom = totalHeight;
		}
		for(int i=0; i<numColumns; i++){
			GridColumn* col = columns[i];
			MoveToEx(hdc, left + col->getWidth()-1, delegate->rowHeight()+1, NULL);
			LineTo(hdc, left + col->getWidth()-1, bottom );
			left += col->getWidth();
		}
	} else {
		MoveToEx(hdc, totalWidth, 0, NULL);
		LineTo(hdc, totalWidth, totalHeight);
	}

}

void CinchGrid::DrawHorizontalGridlines(HDC hdc, RECT client)
{
	SelectObject(hdc, gridlinesPen);
	if( delegate->drawHorizontalGridlines() ){
		int i = 0;
		int bottom = client.bottom;
		if (  delegate->allowNewRows() == false ){
			bottom = totalHeight+1;
		}
		while (i * delegate->rowHeight() < bottom ){
			MoveToEx(hdc, 0, i*delegate->rowHeight(), NULL);
			LineTo(hdc, totalWidth, i*delegate->rowHeight());
			i+= 1;
		}
	} else {
		//Always draw bottom gridline
		MoveToEx(hdc, 0, totalHeight-1, NULL);
		LineTo(hdc, totalWidth, totalHeight-1);
	}
}

void CinchGrid::DrawTextForRow(HDC hdc, RECT client, int row){
	int left = 0;
	int top = delegate->rowHeight() * (row+1) + 1;
	for(int col = 0; col<delegate->totalColumns(); col++){
		GridColumn* c = columns[col];
		RECT textRect;
		textRect.left = left+LEFT_MARGIN;
		textRect.right = left + c->getWidth()-1;
		textRect.top = top+1;
		textRect.bottom = top + delegate->rowHeight()-2;
		left += c->getWidth();
		if ( row == activeRow - 1 && delegate->rowSelection() == true ){
			FillRect(hdc, &textRect, activeRowBrush); 
		} else {
			FillRect(hdc, &textRect, solidWhiteBrush); 
		}
		DrawText(hdc, delegate->cellContent(row, col), -1, &textRect, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
	}
}

void CinchGrid::ClearActiveRow(int row, HDC hdc, RECT client)
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

void CinchGrid::DrawActiveRow(HDC hdc, RECT client)
{
	if( delegate->rowSelection() == false ){
		return;
	}
	if ( activeRow >= 1){
			
		LOGBRUSH lb = {BS_SOLID, RGB(100, 100, 100), 0}; 
		HPEN activeCellPen = ExtCreatePen(PS_COSMETIC | PS_ALTERNATE | PS_ENDCAP_SQUARE | PS_JOIN_ROUND, 1, &lb, 0, NULL);

		SelectObject(hdc, activeCellPen);
		SelectObject(hdc,GetStockObject(NULL_BRUSH));
		RECT row;
		row.left = 0;
		row.right = totalWidth;
		row.top = activeRow * delegate->rowHeight();
		row.bottom = row.top + delegate->rowHeight();
		FillRect(hdc, &row, activeRowBrush);
		Rectangle(hdc, row.left, row.top, row.right, row.bottom);
		DrawTextForRow(hdc, client, activeRow-1);
	}
}

void CinchGrid::DrawCellText(HDC hdc, RECT client)
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

LRESULT CALLBACK CinchGrid::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	CinchGrid* self = (CinchGrid *)GetWindowLong(hWnd, GWL_USERDATA);

	
	switch (message)
	{
	case WM_NCCREATE:
		{
		CinchGrid * grid = new CinchGrid(hWnd, GetModuleHandle(0));
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)grid);
		return TRUE;
		}
		break;
	//case WM_MOUSEACTIVATE:
		//SetFocus(hWnd);
		//return MA_ACTIVATE;
	case WM_MOUSEMOVE:	
		{
		int mouseXPos = GET_X_LPARAM(lParam); 
		int mouseYPos = GET_Y_LPARAM(lParam); 
		
		if ( self->draggingHeader ){
			int accum = 0;
			for(int i=0; i<self->activelyDraggedColumn; i++){
				accum += self->columns[i]->getWidth();
			}
			if ( mouseXPos - accum > 10 ){
				RECT client;
				GetClientRect(hWnd, &client);

				RECT invalidate1;
				invalidate1.left = self->draggedXPos - 1;
				invalidate1.right = self->draggedXPos + 1;
				invalidate1.top = 0;
				invalidate1.bottom = client.bottom;


				self->draggedXPos = mouseXPos;
				RECT invalidate;
				invalidate.left = mouseXPos - 1;
				invalidate.right = mouseXPos + 1;
				invalidate.top = 0;
				invalidate.bottom = client.bottom;

				InvalidateRect(hWnd, &invalidate1, true);
				InvalidateRect(hWnd, &invalidate, true);
			}
		}
		if ( mouseYPos < self->delegate->rowHeight() ){
			int accum = 0;
			for(int i=0; i<self->numColumns; i++){
				accum += self->columns[i]->getWidth();
				if ( abs(accum-mouseXPos) < 10 ){
					SetCursor(LoadCursor(NULL, IDC_SIZEWE));
				}
			}
		
		}
		break;
		}
	case WM_LBUTTONUP:
		if ( self->draggingHeader == true ){
			int accum = 0;
			int mouseXPos = GET_X_LPARAM(lParam); 
			self->totalWidth = 0;
			for(int i=0; i<self->activelyDraggedColumn; i++){
				accum += self->columns[i]->getWidth();
			}

			self->columns[self->activelyDraggedColumn]->setWidth(mouseXPos - accum);	
			
			self->totalWidth = 0;
			for(int i=0; i<self->numColumns; i++){
				self->totalWidth += self->columns[i]->getWidth();
			}

			self->totalArea.right = self->totalWidth;
			self->offscreenBitmap = CreateCompatibleBitmap(GetDC(hWnd), self->totalWidth, self->totalHeight+1);
			SelectObject(self->offscreenDC, self->offscreenBitmap);

			self->DrawGridElements(self->offscreenDC, self->totalArea);

			InvalidateRect(hWnd, NULL, true);
		}
		self->draggingHeader = false;

		break;
	case WM_LBUTTONDOWN:
		{
		RECT repaint;
		repaint.left = self->activeCol * COL_SPACING - 2;
		repaint.right = repaint.left + COL_SPACING + 4;
		repaint.top = self->activeRow * self->delegate->rowHeight() - 2;
		repaint.bottom = repaint.top + self->delegate->rowHeight() + 4;
		//InvalidateRect(hWnd, NULL, true);

		int xPos = GET_X_LPARAM(lParam); 
		int yPos = GET_Y_LPARAM(lParam); 

		if ( yPos < self->delegate->rowHeight() ){
			int accum = 0;
			for(int i=0; i<self->numColumns; i++){
				accum += self->columns[i]->getWidth();
				if ( abs(accum-xPos) < 10 ){
					self->draggingHeader = true;
					self->activelyDraggedColumn = i;
				}
			}
		} else {

	
			int accum = 0;
			for(int i=0; i<self->numColumns; i++){
				accum += self->columns[i]->getWidth();
				if ( accum > xPos ){
					self->activeCol = i;
					break;
				}
			}
			int clickedRow = (yPos + self->scrollOffsetY) / self->delegate->rowHeight() - 1;

			if ( self->delegate->allowNewRows() && clickedRow >= self->delegate->totalRows() ){
				self->delegate->prepareNewRow(clickedRow);
				self->totalHeight = (self->delegate->totalRows() + 1) * self->delegate->rowHeight();
			}

			if ( clickedRow < self->delegate->totalRows() ){
				int previousActiveRow = self->activeRow;
				self->activeRow = (yPos + self->scrollOffsetY) / self->delegate->rowHeight();
				
				RECT client;
				GetClientRect(hWnd, &client);
	
				if( self->delegate->rowSelection() ){
	
				
					RECT r2;
					r2.left = 0;
					r2.right = client.right;
					r2.top = previousActiveRow * self->delegate->rowHeight() - 2;
					r2.bottom = repaint.top + self->delegate->rowHeight() + 4;
					//InvalidateRect(hWnd, NULL, true);

					repaint.left = 0;
					repaint.right = client.right;
					repaint.top = self->activeRow * self->delegate->rowHeight() - 2;
					repaint.bottom = repaint.top + self->delegate->rowHeight() + 4;
					//InvalidateRect(hWnd, &repaint, true);
					if (previousActiveRow > 0 ){
						self->ClearActiveRow(previousActiveRow, self->offscreenDC, client);
					}
					self->DrawActiveRow(self->offscreenDC, client);
					//InvalidateRect(hWnd, &repaint, true);
					//InvalidateRect(hWnd, &r2, true);
					InvalidateRect(hWnd, NULL, true);

				}

				self->startEditing(previousActiveRow-1, self->activeRow-1);

				if( previousActiveRow != -1 ){
					self->DrawTextForRow(self->offscreenDC, client, previousActiveRow-1); 
					InvalidateRect(hWnd, NULL, true);
				}

			}
		}
		}
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
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
		if ( self != NULL ){
			self->SetupAndDrawOffscreenBitmap();
			InvalidateRect(hWnd, NULL, true);

			self->SetScroll(hWnd);
		}
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		{
		hdc = BeginPaint(hWnd, &ps);
		
		//POINT origin;
		//GetWindowOrgEx(hdc, &origin);
		//SetWindowOrgEx(hdc, origin.x + scrollOffsetX, origin.y + scrollOffsetY, 0);

		//OffsetRect(&ps.rcPaint, scrollOffsetX, scrollOffsetY);
		

		if( self != NULL ){
			BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, self->offscreenDC, ps.rcPaint.left + self->scrollOffsetX, ps.rcPaint.top + self->scrollOffsetY, SRCCOPY);
		
			if( self->draggingHeader ){
				self->DrawHeaderDragGuideline(hdc, ps.rcPaint);
			}
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
		wchar_t d[90];
		
		if ( cmd == SB_THUMBPOSITION || cmd == SB_THUMBTRACK ){
			SetScrollPos(hWnd, SB_VERT, ypos, true);
			int lastY = self->scrollOffsetY;
			self->scrollOffsetY = ypos;
			self->scrollEditors(0, self->scrollOffsetY - lastY);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_PAGEDOWN ){
			self->scrollOffsetY += client.bottom;
			if( self->scrollOffsetY > self->totalHeight - client.bottom ){
				self->scrollOffsetY = self->totalHeight - client.bottom;
			} else {
				self->scrollEditors(0, client.bottom);				
			}
			SetScrollPos(hWnd, SB_VERT, self->scrollOffsetY, true);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_PAGEUP ){
			self->scrollOffsetY -= client.bottom;
			if( self->scrollOffsetY < 0 ) {
				self->scrollOffsetY = 0;
			} else {
				self->scrollEditors(0, 0 - client.bottom);
			}
			SetScrollPos(hWnd, SB_VERT, self->scrollOffsetY, true);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_LINEDOWN ){
			self->scrollOffsetY += self->delegate->rowHeight();
			if( self->scrollOffsetY > self->totalHeight - client.bottom ){
				self->scrollOffsetY = self->totalHeight - client.bottom;
			}	
			SetScrollPos(hWnd, SB_VERT, self->scrollOffsetY, true);
			self->scrollEditors(0, self->delegate->rowHeight());
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_LINEUP ){
			self->scrollOffsetY -= self->delegate->rowHeight();
			if( self->scrollOffsetY < 0 ){
				self->scrollOffsetY = 0;
			} else {
				self->scrollEditors(0, 0 - self->delegate->rowHeight());
				SetScrollPos(hWnd, SB_VERT, self->scrollOffsetY, true);
				InvalidateRect(hWnd, NULL, true);
			}
			
		}
		return 0;
		}
	case WM_HSCROLL:
		{
		int xpos = HIWORD(wParam);
		int cmd = LOWORD(wParam);
		if ( cmd == SB_THUMBPOSITION || cmd == SB_THUMBTRACK ){
			SetScrollPos(hWnd, SB_HORZ, xpos, true);
			int lastX = self->scrollOffsetX;
			self->scrollOffsetX = xpos;
			self->scrollEditors(self->scrollOffsetX - lastX, 0);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_PAGERIGHT ){
			RECT client;
			GetClientRect(hWnd, &client);
			self->scrollOffsetX += client.right;
			if( self->scrollOffsetX > self->totalWidth - client.right ){
				self->scrollOffsetX = self->totalWidth - client.right;
			} else {
				self->scrollEditors(client.right, 0);
			}
			SetScrollPos(hWnd, SB_HORZ, self->scrollOffsetX, true);
			InvalidateRect(hWnd, NULL, true);
		
		} else if ( cmd == SB_PAGELEFT ){
			RECT client;
			GetClientRect(hWnd, &client);
			self->scrollOffsetX -= client.right;
			if( self->scrollOffsetX < 0 ) { 
				self->scrollOffsetX = 0;
			} else {
				self->scrollEditors(-client.right, 0);
			}
			SetScrollPos(hWnd, SB_HORZ, self->scrollOffsetX, true);
			InvalidateRect(hWnd, NULL, true);
		} else if ( cmd == SB_LINELEFT ) {
			RECT client;
			GetClientRect(hWnd, &client);
			self->scrollOffsetX -= 10;
			if ( self->scrollOffsetX < 0 ) {
				self->scrollOffsetX = 0;
			} else {
				self->scrollEditors(-10, 0);
			}
			SetScrollPos(hWnd, SB_HORZ, self->scrollOffsetX, true);
			InvalidateRect(hWnd, NULL, true);

		} else if ( cmd == SB_LINERIGHT ){

			RECT client;
			GetClientRect(hWnd, &client);
			self->scrollOffsetX += 10;
			if( self->scrollOffsetX > self->totalWidth - client.right ){
				self->scrollOffsetX = self->totalWidth - client.right;
			} else {
				self->scrollEditors(10, 0);
			}
			SetScrollPos(hWnd, SB_HORZ, self->scrollOffsetX, true);
			InvalidateRect(hWnd, NULL, true);
		}
		return 0;
		}
	case WM_MOUSEWHEEL:
		{
		if( self->overflowY ){
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			int prevOffsetY = self->scrollOffsetY;
			self->scrollOffsetY += 0 - zDelta;
			RECT client;
			GetClientRect(self->hWnd, &client);
			if( self->scrollOffsetY > self->totalHeight - client.bottom ){
				self->scrollOffsetY = self->totalHeight - client.bottom;
			
			} else if ( self->scrollOffsetY < 0 ){
				self->scrollOffsetY = 0;
			}
		
			self->scrollEditors(0, self->scrollOffsetY - prevOffsetY);
		
			SetScrollPos(hWnd, SB_VERT, self->scrollOffsetY, true);
			InvalidateRect(hWnd, NULL, true);
			}
		}
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


LRESULT CALLBACK CinchGrid::DetailWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	CinchGrid * self = (CinchGrid *)dwRefData;

	switch(message){

	case WM_NCCALCSIZE:
		wchar_t name[80];
		GetClassName(hWnd, name, 80);
		if ( wcscmp(name, TEXT("Edit")) == 0 ){
			HDC dc = GetDC(hWnd);
			SelectObject(dc, self->delegate->getEditFont());
			TEXTMETRIC metrics;
			GetTextMetrics(dc, &metrics);
			const int cyBorder = (self->delegate->rowHeight() - metrics.tmHeight) / 2 - 3;
			InflateRect((LPRECT)lParam, 0, -cyBorder);
		}
		break;
	
	case WM_KILLFOCUS:
		self->delegate->editingFinished(hWnd, self->activeRow-1, uIdSubclass);
		break;
	case WM_SETFOCUS:
		if ( uIdSubclass == TAB_CAPTURE_CLASS || uIdSubclass == REVERSE_TAB_CAPTURE_CLASS ){
			//delegate->editingFinished(hWnd, activeRow, uIdSubclass);
			int previousActiveRow = self->activeRow;
			if ( uIdSubclass == TAB_CAPTURE_CLASS ){
				SetFocus(self->firstFocusedEditor);
				self->activeRow++;
				
				if ( self->activeRow > self->delegate->totalRows() ){
					if ( self->delegate->allowNewRows() ){
						self->delegate->prepareNewRow(self->activeRow-1);
						self->totalHeight = (self->delegate->totalRows() + 1) * self->delegate->rowHeight();
					} else {
						self->activeRow = 1;
					}
				}
			} else if ( uIdSubclass == REVERSE_TAB_CAPTURE_CLASS ){ 
				SetFocus(self->lastFocusedEditor);
				self->activeRow--;

			
				if ( self->activeRow < 1 ){
					self->activeRow = self->delegate->totalRows();
				}
			}

			self->startEditing(previousActiveRow-1, self->activeRow-1);
	
			if( previousActiveRow != -1 ){
				RECT client;
				GetClientRect(hWnd, &client);
				self->DrawTextForRow(self->offscreenDC, client, previousActiveRow-1); 
				InvalidateRect(hWnd, NULL, true);
			}

		}
		break;

	default:
		break;
	}

	return DefSubclassProc(hWnd, message, wParam, lParam);
	
}



void CinchGrid::startEditing(int previous, int row){
	int left = 0;
	HWND previousWindow = HWND_TOP;
	if( editingInitialized == false ){
		reverseTabCapture = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", L"", WS_CHILD | WS_TABSTOP,
			0, 0, 0, 0, hWnd, NULL, hInst, NULL);
		SetWindowSubclass(reverseTabCapture, DetailWndProc, REVERSE_TAB_CAPTURE_CLASS, (DWORD_PTR)this);
				
		SetWindowPos(reverseTabCapture, HWND_TOP, 0, 0, 0, 0, 0);
		ShowWindow(reverseTabCapture, SW_SHOW);

		previousWindow = reverseTabCapture;
	} else {
		previousWindow = reverseTabCapture;
	}
	for(int i=0; i<numColumns; i++){
		if( delegate->allowEditing(i) ){
			if ( columns[i]->getEditor() == NULL ){
				HWND editor = delegate->editorForColumn(i, hWnd, hInst);
				SendMessage(editor, WM_SETFONT, (WPARAM)delegate->getEditFont(), 0);
				SetWindowSubclass(editor, DetailWndProc, i, (DWORD_PTR)this);
				columns[i]->setEditor(editor);
			} else {
				delegate->editingFinished(columns[i]->getEditor(), previous, i); 
			}
			HWND editor = columns[i]->getEditor();
			delegate->setupEditorForCell(editor, row, i);
			if( editingInitialized == false && previousWindow == reverseTabCapture ){
				firstFocusedEditor = editor;
			}
			int a = delegate->rowHeight();
			int width = columns[i]->getWidth();
			SetWindowPos(editor, previousWindow, left-scrollOffsetX, (row+1) * delegate->rowHeight() - scrollOffsetY, columns[i]->getWidth(), delegate->rowHeight(), 0);
			ShowWindow(editor, SW_SHOW);
			previousWindow = editor;
		}
		left += columns[i]->getWidth();
	}

	lastFocusedEditor = previousWindow;

	if ( editingInitialized == false ){
		tabCapture = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", L"", WS_CHILD | WS_TABSTOP,
			0, 0, 0, 0, hWnd, NULL, hInst, NULL);
		
		SetWindowSubclass(tabCapture, DetailWndProc, TAB_CAPTURE_CLASS, (DWORD_PTR)this);
				
		SetWindowPos(tabCapture, previousWindow, 0, 0, 0, 0, 0);
		ShowWindow(tabCapture, SW_SHOW);
	}

	SendMessage(GetFocus(), EM_SETSEL, 0, -1);
		
	editingInitialized = true;
}




void CinchGrid::scrollEditors(int offsetX, int offsetY){
	for(int i=0; i<numColumns; i++){
		if ( columns[i]->getEditor() != NULL ){
			RECT editor;
			GetWindowRect(columns[i]->getEditor(), &editor);
			MapWindowPoints(NULL, hWnd, (LPPOINT)&editor, 2);
			SetWindowPos(columns[i]->getEditor(), HWND_TOP, editor.left - offsetX, editor.top - offsetY, 0, 0, SWP_NOSIZE);
		}
	}
}



void CinchGrid::DrawGridElements(HDC hdc, RECT client)
{
	FillRect(offscreenDC, &totalArea, solidWhiteBrush);

	DrawVerticalGridlines(hdc, client);
	DrawHorizontalGridlines(hdc, client);
	DrawCellText(hdc, client);

	DrawActiveRow(hdc, client);
	DrawHeaderDragGuideline(hdc, client);

	DrawHeader(hdc, client);
}



