// SimpleGrid.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Windowsx.h"

#define MAX_LOADSTRING 100

#define DEFAULT_COLUMN_WIDTH 120

#define MAX_OFFSCREEN_WIDTH 400
#define MAX_OFFSCREEN_HEIGHT 8000

TCHAR szClassName[] = _T("CinchGrid");
 
void RegisterCinchGrid()
{
    WNDCLASSEX wc;
	wc.cbSize         = sizeof(wc);
    wc.lpszClassName  = szClassName;
    wc.hInstance      = GetModuleHandle(0);
	wc.lpfnWndProc    = CinchGrid::WndProc;
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon          = 0;
    wc.lpszMenuName   = 0;
	wc.hbrBackground  = (HBRUSH)CreateSolidBrush(RGB(255,255,255));
    wc.style          = 0;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hIconSm        = 0;
 
    RegisterClassEx(&wc);
}


HWND CinchGrid::CreateCinchGrid(HWND parent, GridDelegate* delegate)
{
	RegisterCinchGrid();

	HWND hWnd = CreateWindowEx(0,
		szClassName,
		_T("Cinch Grid"),
		WS_CHILD,
		0, 0, 0, 0,
		parent,
		NULL, GetModuleHandle(0), delegate);


	
	return hWnd;
}


CinchGrid::CinchGrid(HWND h, HINSTANCE inst, GridDelegate * d){

	numColumns = 0;

	headerEditor = NULL;

	hWnd = h;
	hInst = inst;
	activeRow = -1;
	activelyEditingRow = -1;
	activeCol = -1;

	editingHeader = -1;

	scrollOffsetX = 0;
	scrollOffsetY = 0;

	windowOffsetY = 0;

	sortedColumn = -1;

	overflowX = false;
	overflowY = false;

	totalWidth = 0;
	totalHeight = 0;

	draggingHeader = false;
	editingInitialized = false;
	draggedXPos = 0;

	activelyDraggedColumn = -1;

	delegate = d;

	data = nullptr;
	page_table = nullptr;
	rows_loaded = 0;

	initialize();

  
	headerPen = CreatePen(PS_SOLID, 1, RGB(137, 140, 149));
	gridlinesPen = CreatePen(PS_SOLID, 1, RGB(205, 205, 205));
	borderlinesPen = CreatePen(PS_SOLID, 1, RGB(137, 140, 149));
	sortIndicatorPen = CreatePen(PS_SOLID, 1, RGB(165,172,181));
	LOGBRUSH lb = {BS_SOLID, RGB(100, 100, 100), 0}; 
	activeCellPen = ExtCreatePen(PS_COSMETIC | PS_ALTERNATE | PS_ENDCAP_SQUARE | PS_JOIN_ROUND, 1, &lb, 0, NULL);
	
	solidWhiteBrush = CreateSolidBrush(RGB(255,255,255));
	activeRowBrush = CreateSolidBrush(RGB(167,205,240));
	sortIndicatorBrush = CreateSolidBrush(RGB(165,172,181));

	offscreenDC = CreateCompatibleDC(GetDC(hWnd));
}

void CinchGrid::setupColumns(){
	for(int i=0; i<numColumns; i++){
		delete columns[i];
		columns[i] = NULL;
	}

	numColumns = 0;
	totalWidth = 0;
	numAutosizedCols = 0;
	totalWidthFixedCols = 0;
	autosized = false;

	for(int i=0; i<delegate->totalColumns(); i++){
		if ( delegate->columnWidth(i) == CINCH_GRID_MAXIMIZE_WIDTH ){
			autosized = true;
			numAutosizedCols++;	
		}
	}

	for(int i=0; i<delegate->totalColumns(); i++){
		wstring headerContent;
		delegate->headerContent(i, headerContent);
		int width = delegate->columnWidth(i);

		addColumn(headerContent, width);
		if ( !autosized ) totalWidth += width;
		
		if ( width != CINCH_GRID_MAXIMIZE_WIDTH ){
			totalWidthFixedCols += width;
		}
	}
}

void CinchGrid::initialize(){
	
	RECT client;
	GetClientRect(hWnd, &client);
	
	clientWidth = client.right;

	if ( data != nullptr ){
		CleanupData();
	}


	if ( numColumns == 0 ){
		setupColumns();
	}
	
	numRows = delegate->totalRows();

	totalHeight = (delegate->totalRows()) * delegate->rowHeight();
	
	
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
	
	
	data = new wchar_t**[PAGESIZE];
	for(int i=0; i<PAGESIZE; i++){
		data[i] = new wchar_t*[numColumns];
		memset(data[i], 0, numColumns*sizeof(wchar_t*));
	}
	page_table = new int[PAGESIZE];
	memset(page_table, 0xffff, PAGESIZE);
}

void CinchGrid::CleanupData(){
	if ( data != nullptr ){
		delegate->CleanupSegment(rows_loaded, data);
	}

	for(int i=0; i<PAGESIZE; i++){
		delete data[i];
	}
	delete data;
	delete page_table;
	
	data = nullptr;
	page_table = nullptr;
}

CinchGrid::~CinchGrid(){
	for(int i=0; i<numColumns; i++){
		delete columns[i];
	}

	CleanupData();
}

void CinchGrid::reloadData(){
	stopEditing();

	delegate->willReloadData();
	
	if (editingInitialized == true ){
		for(int i=0; i<numColumns; i++){
			if( delegate->allowEditing(i) ){
				if ( columns[i]->getEditor() != NULL ){
					delegate->setupEditorForCell(columns[i]->getEditor(), activelyEditingRow, i, data);
				}
			}
		}
	}
	

	activeRow = -1;
	initialize();

	SetScroll(hWnd);

	delegate->didReloadData();

	SetupAndDrawOffscreenBitmap();
	InvalidateRect(hWnd, NULL, true);

}


void CinchGrid::addColumn(wstring header, int width) {
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
		overflowX = (overflowX && scrollOffsetX > 0) || false;
	}
	if ( totalHeight + 50 > client.bottom ){
		overflowY = true;
	} else {
		overflowY = (overflowY && scrollOffsetY > 0) || false;
	}


	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE;
    si.nMin   = 0;
	si.nMax   = (delegate->totalRows() * delegate->rowHeight()) + client.bottom;
	si.nPage  = client.bottom;
	
	SCROLLINFO hsi;
	ZeroMemory(&hsi, sizeof(hsi));
    hsi.cbSize = sizeof(hsi);
    hsi.fMask  = SIF_RANGE | SIF_PAGE;
    hsi.nMin   = 0;
	hsi.nMax   = totalWidth;
	hsi.nPage  = client.right;

	if ( overflowX){
		SetScrollInfo(hWnd, SB_HORZ, &hsi, TRUE);
		ShowScrollBar(hWnd, SB_HORZ, true);
	} else {
		ShowScrollBar(hWnd, SB_HORZ, false);
	}
	if ( overflowY ){
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		ShowScrollBar(hWnd, SB_VERT, true);
	} else {
		ShowScrollBar(hWnd, SB_VERT, false);
	}
			
}
void CinchGrid::AdjustWindow(){

	//totalHeight
	//windowheight

	RECT client;
	GetClientRect(hWnd, &client);

	if ( scrollOffsetY + client.bottom > windowOffsetY + offscreenHeight ){
		windowOffsetY =  max(0, scrollOffsetY - ((offscreenHeight - client.bottom) / 2));
		
		SetupAndDrawOffscreenBitmap();
	} else if ( scrollOffsetY < windowOffsetY ){
		windowOffsetY =  max(0,scrollOffsetY - ((offscreenHeight - client.bottom) / 2));
		
		SetupAndDrawOffscreenBitmap();
	}
}

void CinchGrid::SetupWindowOffset()
{
	SetWindowOrgEx(offscreenDC, 0, windowOffsetY, 0);
}

void CinchGrid::ClearWindowOffset()
{	
	SetWindowOrgEx(offscreenDC, 0, 0, 0);
}

void CinchGrid::SetupAndDrawOffscreenBitmap(){
	RECT client;
 	GetClientRect(hWnd, &client);
	
	offscreenWidth = max(client.right, min(totalWidth, MAX_OFFSCREEN_WIDTH));
	offscreenHeight = max(1, min(totalHeight+client.bottom, MAX_OFFSCREEN_HEIGHT));

	offscreenBitmap = CreateCompatibleBitmap(GetDC(hWnd), offscreenWidth, offscreenHeight);
	SelectObject(offscreenDC, offscreenBitmap);

	totalArea.left = 0;
	totalArea.right = offscreenWidth;
	totalArea.bottom = offscreenHeight;
	totalArea.top = 0;
	
	FillRect(offscreenDC, &totalArea, solidWhiteBrush);
	
	SetupWindowOffset();
	DrawGridElements(offscreenDC, totalArea);
	ClearWindowOffset();
}

void CinchGrid::DrawHeaderDragGuideline(HDC hdc, RECT client )
{
	if ( draggingHeader == true ){
		SelectObject(hdc, gridlinesPen);	
	
		MoveToEx(hdc, draggedXPos, 0, NULL);
		LineTo(hdc, draggedXPos, client.bottom);
	}
}

void DrawColumnHeader(HDC hdc, int x, int width, int height, LPWSTR text)
{
	MoveToEx(hdc, x+width-1, 0, NULL);
	LineTo(hdc, x+width-1, height);
	TRIVERTEX vertex[2] ;
	vertex[0].x     = x+1;
	vertex[0].y     = 1;
	vertex[0].Red   = 0xf200;
	vertex[0].Green = 0xf200;
	vertex[0].Blue  = 0xf200;
	vertex[0].Alpha = 0x0000;

	vertex[1].x     = x+width-1;
	vertex[1].y     = height; 
	vertex[1].Red   = 0xe200;
	vertex[1].Green = 0xe200;
	vertex[1].Blue  = 0xe200;
	vertex[1].Alpha = 0x0000;

	GRADIENT_RECT r;
	r.UpperLeft = 0;
	r.LowerRight = 1;

	GradientFill(hdc, vertex, 2, &r, 1, GRADIENT_FILL_RECT_V);

	RECT headerText;
	headerText.left = x + 6;
	headerText.top = 1;
	headerText.bottom = height;
	headerText.right = x + width;

	DrawText(hdc, text, -1, &headerText, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
		
}

void CinchGrid::DrawHeader(HDC hdc, RECT client, bool fromPaintRoutine){
	
	if ( delegate->stickyHeaders() == true && !fromPaintRoutine ){
		//This will be drawn directly in the paint routine.
		return;
	}
	SetBkMode(hdc, TRANSPARENT);

	HFONT hFont = delegate->getFont();
	SelectObject(hdc, hFont);
	

	SelectObject(hdc, headerPen);
	int right;
	if ( autosized ) right = clientWidth;
	else right = totalArea.right;
	
	MoveToEx(hdc, 0, 0, NULL);
	if ( !delegate->allowNewColumns() && !autosized){
		right = totalWidth;
	}
	
	int j = 0;
	int left = 0;

	MoveToEx(hdc, 0, 0, NULL);
	LineTo(hdc, 0, delegate->rowHeight());

	MoveToEx(hdc, 0, delegate->rowHeight(), NULL);
	LineTo(hdc, right, delegate->rowHeight());


	for(int l=0; l<numColumns; l++){
		GridColumn* col = columns[l];
		int width = col->getWidth();
		if ( width == CINCH_GRID_MAXIMIZE_WIDTH ){
			width = ( clientWidth - totalWidthFixedCols ) / numAutosizedCols;
		}
		//Rectangle(hdc, i, 0, i + COL_SPACING, delegate->rowHeight());
		DrawColumnHeader(hdc, left, width, delegate->rowHeight(), (LPWSTR)col->getHeader().c_str());
	
		if ( col->sorted == true ){
			POINT points[3];
				
			if ( col->descending == true ){
				LONG sx = left+width-15;
				LONG sy = 14;
			
				LONG d = 4;

				points[0].x = sx;
				points[0].y = sy;

				points[1].x = sx+d;
				points[1].y = sy-d;

				points[2].x = sx-d;
				points[2].y = sy-d;
			
			} else {
				LONG sx = left+width-15;
				LONG sy = 10;
			
				LONG d = 4;

				
				points[0].x = sx;
				points[0].y = sy;

				points[1].x = sx+d;
				points[1].y = sy+d;

				points[2].x = sx-d;
				points[2].y = sy+d;
			

			}

			SelectObject(hdc, sortIndicatorPen);
			SelectObject(hdc, sortIndicatorBrush);
			
				
			Polygon(hdc, points, 3);

			SelectObject(hdc, headerPen);
		}
		
		left += width;

		j++;
	}

	if( delegate->allowNewColumns() ){
		while (left < offscreenWidth ){
			DrawColumnHeader(hdc, left, DEFAULT_COLUMN_WIDTH, delegate->rowHeight(), L"");
			left += DEFAULT_COLUMN_WIDTH;
		}
	}
}

void CinchGrid::DrawVerticalGridlines(HDC hdc, RECT client)
{
	
	int bottom = offscreenHeight;
	if ( !delegate->allowNewRows() ){
		bottom = totalArea.bottom;
	}

	//SelectObject(hdc, borderlinesPen);

	//MoveToEx(hdc, 0, client.top+delegate->rowHeight(), NULL);
	//LineTo(hdc, 0, bottom + windowOffsetY );

	SelectObject(hdc, gridlinesPen);
	
	int left = 0;
		
	if ( delegate->drawVerticalGridlines() ){
		

		for(int i=0; i<numColumns; i++){
			GridColumn* col = columns[i];
			int width = col->getWidth();
			if ( width == CINCH_GRID_MAXIMIZE_WIDTH ) width = clientWidth;
			MoveToEx(hdc, left + width-1, client.top+delegate->rowHeight()+1, NULL);
			LineTo(hdc, left + width-1, bottom + windowOffsetY );
			left += width;
		}
		if ( delegate->allowNewColumns() ){
			while ( left < offscreenWidth ){
				MoveToEx(hdc, left + DEFAULT_COLUMN_WIDTH-1, windowOffsetY, NULL);
				LineTo(hdc, left + DEFAULT_COLUMN_WIDTH-1, bottom );
			
				left += DEFAULT_COLUMN_WIDTH;
			}
		}
	
	}

	//SelectObject(hdc, borderlinesPen);
	
	//GridColumn* col = columns[numColumns-1];
	//MoveToEx(hdc, left + col->getWidth()-1, client.top+delegate->rowHeight(), NULL);
	//LineTo(hdc, left + col->getWidth()-1, bottom + windowOffsetY );
			
	
}

void CinchGrid::DrawHorizontalGridlines(HDC hdc, RECT client)
{
	SelectObject(hdc, gridlinesPen);
	if( delegate->drawHorizontalGridlines() ){
		int i = windowOffsetY / client.bottom + 2;  //+2 is to skip the header, it draws it's own darker gridlines
		int bottom = offscreenHeight;
		if (  delegate->allowNewRows() == false ){
			bottom = bottom+1;
		}
		int width = totalWidth;
		if ( autosized == true ){
			width = clientWidth;
		}
		if( delegate->allowNewColumns() ){
			width = offscreenWidth;
		}
		while (i * delegate->rowHeight() < bottom + windowOffsetY ){
			if( i * delegate->rowHeight() >= 0 ){
				MoveToEx(hdc, 0, i*delegate->rowHeight()+client.top, NULL);
				LineTo(hdc, width, i*delegate->rowHeight() + client.top);
			}
			i+= 1;
		}
	}

	//Always draw bottom gridline
	//SelectObject(hdc, borderlinesPen);
	//MoveToEx(hdc, 0, client.bottom-1, NULL);
	//LineTo(hdc, totalWidth, client.bottom-1);
	
	SelectObject(hdc, headerPen);	

	MoveToEx(hdc, 0, 0, NULL);
	int right = totalArea.right;
	if ( !delegate->allowNewColumns() ){
		right = totalWidth;
	}
	//LineTo(hdc, right, 0);
	MoveToEx(hdc, 0, delegate->rowHeight(), NULL);
	LineTo(hdc, right, delegate->rowHeight());
}

void CinchGrid::DrawTextForRow(HDC hdc, RECT client, int row){
	int left = 0;
	int top = delegate->rowHeight() * (row+1) + 1;
	for(int col = 0; col<delegate->totalColumns(); col++){
		GridColumn* c = columns[col];
		RECT textRect;
		textRect.left = left+LEFT_MARGIN;
		int width = c->getWidth();
		if ( width == CINCH_GRID_MAXIMIZE_WIDTH ){
			width = ( clientWidth - totalWidthFixedCols ) / numAutosizedCols;
		}
		textRect.right = left + width-1;
		textRect.top = top+1;
		textRect.bottom = top + delegate->rowHeight()-2;
		left += width;
		if ( row == activeRow - 1 && delegate->rowSelection() == true ){
			FillRect(hdc, &textRect, activeRowBrush); 
		} else {
			FillRect(hdc, &textRect, solidWhiteBrush); 
		}

		//wstring content;
		//delegate->cellContent(row, col, content);
		
		int len = PAGESIZE;
		if ( numRows < PAGESIZE ){
			len = numRows;
		}

		if ( page_table[row % PAGESIZE] != row ){
			if ( data != nullptr ){
				delegate->CleanupSegment(rows_loaded, data);
			}
			rows_loaded = delegate->LoadSegment(row, len, data);
			for(int i=row; i<row+PAGESIZE; i++){
				page_table[i%PAGESIZE] = i+row;
			}
		}

		int rc = DrawText(hdc, data[row%PAGESIZE][col], -1, &textRect, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
	}
}

void CinchGrid::ClearActiveRow(int row, HDC hdc, RECT client)
{
	RECT rect;
	rect.left = 0;
	if ( autosized ) rect.right = clientWidth;
	else  rect.right = totalWidth;
	rect.top = (row) * delegate->rowHeight() ;
	rect.bottom = rect.top + delegate->rowHeight()+1;
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
			
		SelectObject(hdc, activeCellPen);
		SelectObject(hdc,GetStockObject(NULL_BRUSH));
		RECT row;
		row.left = 0;
		if ( autosized ) row.right = clientWidth;
		else  row.right = totalWidth;
		row.top = activeRow * delegate->rowHeight();
		row.bottom = row.top + delegate->rowHeight()+1;
		FillRect(hdc, &row, activeRowBrush);
		Rectangle(hdc, row.left, row.top, row.right, row.bottom);

		HFONT hFont = delegate->getFont();
		SelectObject(hdc, hFont);
		SetBkMode(hdc, TRANSPARENT);
	
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
	if ( windowOffsetY < 0 ){
		int a = 1;
	}
	for(int row = windowOffsetY / delegate->rowHeight(); row<delegate->totalRows() && row*delegate->rowHeight() < client.bottom + windowOffsetY; row++){
		left = 0;
		DrawTextForRow(hdc, client, row);
	}
}



LRESULT CinchGrid::OnKeyDown(WPARAM wParam, LPARAM lParam){
	
	RECT client;
	GetClientRect(hWnd, &client);
	int pageSize = client.bottom / delegate->rowHeight();

	if ( activeRow > -1 ){
		switch(wParam){
		case VK_UP:
			if( activeRow - 1 > 0 ){
				SetActiveRow(activeRow-1, true);
				ScrollRowIntoViewFromBeneath(activeRow+1);
			}
			break;
		case VK_DOWN:
			if ( activeRow + 1 <= delegate->totalRows() ){
				SetActiveRow(activeRow+1, true);
				ScrollRowIntoView(activeRow+1);	
			}
			break;
		case VK_NEXT:
			if( activeRow + pageSize > delegate->totalRows() ){
				SetActiveRow(delegate->totalRows(), true);
			} else {
				SetActiveRow(activeRow+pageSize, true);
			}
			ScrollRowIntoView(activeRow);	
			break;
		case VK_PRIOR:
			if( activeRow - pageSize < 1 ){
				SetActiveRow(1, true);
			} else {
				SetActiveRow(activeRow - pageSize, true);
			}
			ScrollRowIntoViewFromBeneath(activeRow);	
			break;
		}

		
	}
	SetFocus(hWnd);
	return MA_ACTIVATEANDEAT;
}

LRESULT CinchGrid::OnKeyUp(WPARAM wParam, LPARAM lParam){
	switch(wParam){
		case VK_UP:
		case VK_DOWN:
		case VK_NEXT:
		case VK_PRIOR:
			PostMessage(GetParent(hWnd), CINCHGRID_ROW_SELECTED, 0, 0);
			delegate->didSelectRow(GetActiveRow());
			break;
		}
	
	SetFocus(hWnd);
	return MA_ACTIVATEANDEAT;
}


void CinchGrid::ScrollRowIntoViewFromBeneath(int row){
	RECT client;
	GetClientRect(hWnd, &client);
	int windowPos = row * delegate->rowHeight() - windowOffsetY;
	bool offScreen = (windowPos < (scrollOffsetY-delegate->rowHeight()+100) );
	if  (offScreen ) {
		scrollOffsetY = (row * delegate->rowHeight()) - ( delegate->rowHeight() * 3 );
		if ( scrollOffsetY < 0 ) scrollOffsetY = 0 ;
		SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
		AdjustWindow();
		InvalidateRect(hWnd, NULL, true);
	}
	if ( windowPos < delegate->rowHeight() ){
		scrollOffsetY = 0;
		SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
		AdjustWindow();
		InvalidateRect(hWnd, NULL, true);
	}
}


void CinchGrid::ScrollRowIntoView(int row){
	RECT client;
	GetClientRect(hWnd, &client);
	int windowPos = row * delegate->rowHeight() - windowOffsetY;
	bool offScreen = ( windowPos > (client.bottom - delegate->rowHeight()) ) || (windowPos < scrollOffsetY );
	if  (offScreen ) {
		scrollOffsetY = row * delegate->rowHeight() - ( client.bottom - delegate->rowHeight());
		if ( scrollOffsetY < 0 ) scrollOffsetY = 0 ;
		SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
		AdjustWindow();
		InvalidateRect(hWnd, NULL, true);
	}
	if ( windowPos < delegate->rowHeight() ){
		scrollOffsetY = 0;
		SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
		AdjustWindow();
		InvalidateRect(hWnd, NULL, true);
	}
}


LRESULT CinchGrid::OnRButtonUp(WPARAM wParam, LPARAM lParam){
	int mouseXPos = GET_X_LPARAM(lParam); 
	int mouseYPos = GET_Y_LPARAM(lParam);

	if ( delegate->stickyHeaders() && mouseYPos < delegate->rowHeight() ){
		delegate->headerContextClick(hWnd, mouseXPos, mouseYPos);
	}
	if ( !delegate->stickyHeaders() && mouseYPos + scrollOffsetY < delegate->rowHeight() ){
		delegate->headerContextClick(hWnd, mouseXPos, mouseYPos);
	}

	return MA_ACTIVATE;
}

LRESULT CinchGrid::OnLButtonUp(WPARAM wParam, LPARAM lParam){
	if ( draggingHeader == true ){
		int accum = 0;
		int mouseXPos = GET_X_LPARAM(lParam); 
		totalWidth = 0;
		for(int i=0; i<activelyDraggedColumn; i++){
			accum += columns[i]->getWidth();
		}

		columns[activelyDraggedColumn]->setWidth(mouseXPos - accum);	
			
		adjustEditors();

		totalWidth = 0;
		for(int i=0; i<numColumns; i++){
			totalWidth += columns[i]->getWidth();
		}
		draggingHeader = false;
	
		SetupAndDrawOffscreenBitmap();

		InvalidateRect(hWnd, NULL, true);

		delegate->didChangeColumnWidth(activelyDraggedColumn, mouseXPos-accum);
	}

	return MA_ACTIVATE;
}

LRESULT CinchGrid::OnLButtonDown(WPARAM wParam, LPARAM lParam){
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
		int clickedColumn = 0;
		for(int i=0; i<numColumns; i++){
			int width = columns[i]->getWidth();
			if ( width == CINCH_GRID_MAXIMIZE_WIDTH ){
				width = ( clientWidth - totalWidthFixedCols ) / numAutosizedCols;
			}
			accum += width;
			if ( abs(accum-xPos) < 10 ){
				draggingHeader = true;
				activelyDraggedColumn = i;
			}
			if ( accum > xPos ){
				clickedColumn = i;
				break;
			}
		}

		if ( delegate->allowSorting(clickedColumn) == true ){
			if ( sortedColumn >= 0 && sortedColumn != clickedColumn ){
				delegate->sortOff(sortedColumn);
				columns[sortedColumn]->sorted = false;
			}
			if ( columns[clickedColumn]->sorted == true ){
				if ( columns[clickedColumn]->descending == true ){
					columns[clickedColumn]->descending = false;
					delegate->sortAscending(clickedColumn);
					sortedColumn = clickedColumn;
					reloadData();
				} else {
					columns[clickedColumn]->descending = true;
					delegate->sortDescending(clickedColumn);
					sortedColumn = clickedColumn;
					reloadData();
				}
			} else {
				columns[clickedColumn]->sorted = true;
				columns[clickedColumn]->descending = false;
				delegate->sortAscending(clickedColumn);
				sortedColumn = clickedColumn;
				reloadData();
			}
		}	

		if ( !draggingHeader ) {
			if ( delegate->allowHeaderTitleEditing(clickedColumn) ){
				startHeaderTitleEditing(activeCol);
				InvalidateRect(hWnd, NULL, true);
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
		int clickedRow = (yPos + scrollOffsetY) / delegate->rowHeight() - 1;
			

		if ( delegate->allowNewRows() && clickedRow >= delegate->totalRows() ){
			int newRows = clickedRow + 1 - delegate->totalRows();
			for(int i=0; i<newRows; i++){
				delegate->prepareNewRow(clickedRow);
			}
			totalHeight = (delegate->totalRows()) * delegate->rowHeight();
		}

		//To account for the header space.
		clickedRow++;

		if ( clickedRow <= delegate->totalRows() ){

			SetActiveRow(clickedRow);

			
		}
	}

	if ( !delegate->allowEditing(activeCol)){
		SetFocus(hWnd);
	}
	return MA_ACTIVATE;
}

LRESULT CinchGrid::OnMouseMove(WPARAM wParam, LPARAM lParam){
	int mouseXPos = GET_X_LPARAM(lParam); 
	int mouseYPos = GET_Y_LPARAM(lParam); 
		
	if ( draggingHeader ){
		int accum = 0;
		for(int i=0; i<activelyDraggedColumn; i++){
			accum += columns[i]->getWidth();
		}
		if ( mouseXPos - accum > 10 ){
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

	return 0;
}

LRESULT CinchGrid::OnSize(WPARAM wParam, LPARAM lParam){
	RECT client;
	GetClientRect(hWnd, &client);
		
	clientWidth = client.right;

	SetupAndDrawOffscreenBitmap();
	InvalidateRect(hWnd, NULL, true);

	SetScroll(hWnd);
	return 0;
}

LRESULT CinchGrid::OnPaint(WPARAM wParam, LPARAM lParam){
	
	HDC          hdc;
    PAINTSTRUCT  ps;

	hdc = BeginPaint(hWnd, &ps);
	
	
	
	BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, offscreenDC, ps.rcPaint.left + scrollOffsetX, ps.rcPaint.top + scrollOffsetY - windowOffsetY, SRCCOPY);
	
	
	if( draggingHeader ){
		DrawHeaderDragGuideline(hdc, ps.rcPaint);
	}

	if ( delegate->stickyHeaders() ){
		DrawHeader(hdc, ps.rcPaint, true);
	}


	RECT rect;
	SelectObject(hdc, borderlinesPen);
	GetClientRect(hWnd, &rect);
	MoveToEx(hdc, 0, 0, NULL);
	LineTo(hdc, rect.right-1, 0);
	LineTo(hdc, rect.right-1, rect.bottom-1);
	LineTo(hdc, 0, rect.bottom-1);
	LineTo(hdc, 0, 0);


	
	/* Draws a visualization of the back buffers */
	//HPEN red = CreatePen(PS_SOLID, 1, RGB(255,0,0));
	//HPEN green = Create
	//Pen(PS_SOLID, 1, RGB(0,255,0));
	//HPEN blue = CreatePen(PS_SOLID, 1, RGB(0,0,255));
	//SelectObject(hdc, red);
	//int xfac = 8;
	//int yfac = totalHeight / 400;
			
	//Rectangle(hdc, 600, 20, 600+(totalWidth/xfac), 20+(totalHeight/yfac));

	//RECT client;
	//GetWindowRect(hWnd, &client);

	//SelectObject(hdc, blue);
	//Rectangle(hdc, 600, 20+(windowOffsetY/yfac), 600+(totalWidth/xfac), 20+((windowOffsetY + offscreenHeight)/yfac));

	//SelectObject(hdc, green);
	//Rectangle(hdc, 600, 20+(scrollOffsetY/yfac), 600+(totalWidth/xfac), 20+((client.bottom+scrollOffsetY)/yfac));
	
	EndPaint(hWnd, &ps);

	return 0;
}

LRESULT CinchGrid::OnHScroll(WPARAM wParam, LPARAM lParam){
	int xpos = HIWORD(wParam);
	int cmd = LOWORD(wParam);
	if ( cmd == SB_THUMBPOSITION || cmd == SB_THUMBTRACK ){
		SetScrollPos(hWnd, SB_HORZ, xpos, true);
		int lastX = scrollOffsetX;
		scrollOffsetX = xpos;
		scrollEditors(scrollOffsetX - lastX, 0);
		AdjustWindow();
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
		AdjustWindow();
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
		AdjustWindow();
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
		AdjustWindow();
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
		AdjustWindow();
		InvalidateRect(hWnd, NULL, true);
	}
	return 0;
}

void CinchGrid::PageDown(){
	RECT client;
	GetClientRect(hWnd, &client);
	int lastY = scrollOffsetY;
	
	scrollOffsetY += client.bottom;
	if( scrollOffsetY > totalHeight + client.bottom ){
		scrollOffsetY = totalHeight + client.bottom;
		scrollEditors(0, scrollOffsetY - lastY);
	} else {
		scrollEditors(0, client.bottom);				
	}
	SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
	AdjustWindow();
	InvalidateRect(hWnd, NULL, true);
}

void CinchGrid::PageUp(){
	RECT client;
	GetClientRect(hWnd, &client);
	int lastY = scrollOffsetY;
	
	scrollOffsetY -= client.bottom;
	if( scrollOffsetY < 0 ) {
		scrollOffsetY = 0;
		scrollEditors(0, scrollOffsetY - lastY);

	} else {
		scrollEditors(0, 0 - client.bottom);
	}
	SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
	AdjustWindow();
	InvalidateRect(hWnd, NULL, true);
}

LRESULT CinchGrid::OnVScroll(WPARAM wParam, LPARAM lParam){
	int ypos = HIWORD(wParam);
	int cmd = LOWORD(wParam); 
	RECT client;
	GetClientRect(hWnd, &client);
	int lastY = scrollOffsetY;
			
	if ( cmd == SB_THUMBPOSITION || cmd == SB_THUMBTRACK ){
		SetScrollPos(hWnd, SB_VERT, ypos, true);
		scrollOffsetY = ypos;
		scrollEditors(0, scrollOffsetY - lastY);
		AdjustWindow();
		InvalidateRect(hWnd, NULL, true);
	} else if ( cmd == SB_PAGEDOWN ){
		PageDown();
	} else if ( cmd == SB_PAGEUP ){
		PageUp();
	} else if ( cmd == SB_LINEDOWN ){
		scrollOffsetY += delegate->rowHeight();
		if( scrollOffsetY > totalHeight + client.bottom ){
			scrollOffsetY = totalHeight + client.bottom;
		} else {
			scrollEditors(0, delegate->rowHeight());
		}
		SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
			
		AdjustWindow();
		InvalidateRect(hWnd, NULL, true);
	} else if ( cmd == SB_LINEUP ){
		scrollOffsetY -= delegate->rowHeight();
		if( scrollOffsetY < 0 ){
			scrollOffsetY = 0;
		} else {
			scrollEditors(0, 0 - delegate->rowHeight());
			SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
			AdjustWindow();
			InvalidateRect(hWnd, NULL, true);
		}
	}

	return 0;
}

LRESULT CinchGrid::OnMouseWheel(WPARAM wParam, LPARAM lParam){
	if( overflowY ){
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		int prevOffsetY = scrollOffsetY;
		scrollOffsetY += 0 - zDelta;
		RECT client;
		GetClientRect(hWnd, &client);
		if( scrollOffsetY > totalHeight + client.bottom ){
			scrollOffsetY = totalHeight + client.bottom;
			
		} else if ( scrollOffsetY < 0 ){
			scrollOffsetY = 0;
		}
		
		scrollEditors(0, scrollOffsetY - prevOffsetY);
		
		SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
		AdjustWindow();
		InvalidateRect(hWnd, NULL, true);
	}
	return 0;
}


LRESULT CALLBACK CinchGrid::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	
	CinchGrid* grid = (CinchGrid *)GetWindowLong(hWnd, GWL_USERDATA);
	CREATESTRUCT* c;

	switch (message)
	{
	case WM_NCCREATE:
		{
		c = (CREATESTRUCT*)lParam;
		GridDelegate* d = (GridDelegate *)c->lpCreateParams;
		grid = new CinchGrid(hWnd, GetModuleHandle(0), d);
		d->setGrid(grid);
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)grid);
		return TRUE;
		}
		break;
	case WM_DESTROY:
		DeleteObject(grid->activeRowBrush);
		DeleteObject(grid->headerPen);
		DeleteObject(grid->gridlinesPen);
		DeleteObject(grid->borderlinesPen);
		DeleteObject(grid->sortIndicatorPen);
		DeleteObject(grid->activeCellPen);
		DeleteObject(grid->solidWhiteBrush);
		DeleteObject(grid->activeRowBrush);
		DeleteObject(grid->sortIndicatorBrush);
		DeleteObject(grid->offscreenDC);
		DeleteObject(grid->offscreenBitmap);
		break;
	case WM_NCDESTROY:
		delete grid;
		break;
	case WM_ERASEBKGND:
		/*{
			RECT rect;
			GetClientRect(hWnd, &rect);
			HDC dc = (HDC)wParam;
			FillRect(dc, &rect, CreateSolidBrush(RGB(245,245,245)));
		}*/
		return 1;
	case WM_GETDLGCODE:
		return DLGC_WANTARROWS;
	case WM_MOUSEACTIVATE:
		SetFocus(hWnd);
		return MA_ACTIVATE;
	case WM_SETFOCUS:
		return 1;
	case WM_KILLFOCUS:
		return 0;
	case WM_KEYDOWN:
		return grid->OnKeyDown(wParam, lParam);
	case WM_KEYUP:
		return grid->OnKeyUp(wParam, lParam);
	case WM_MOUSEMOVE:
		return grid->OnMouseMove(wParam, lParam);
	case WM_PAINT:
		return grid->OnPaint(wParam, lParam);
	case WM_LBUTTONDOWN:
		return grid->OnLButtonDown(wParam, lParam);
	case WM_LBUTTONUP:
		return grid->OnLButtonUp(wParam, lParam);
	case WM_RBUTTONUP:
		return grid->OnRButtonUp(wParam, lParam);
	case WM_SIZE:
		return grid->OnSize(wParam, lParam);
	case WM_MOUSEWHEEL:
		return grid->OnMouseWheel(wParam, lParam);
	case WM_HSCROLL:
		return grid->OnHScroll(wParam, lParam);
	case WM_VSCROLL:
		return grid->OnVScroll(wParam, lParam);
	
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

/*

LRESULT CALLBACK CinchGrid::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	

	
	switch (message)
	{
	case WM_MOUSEMOVE:	
		{
		
		break;
		}
	case WM_LBUTTONUP:
		
	case WM_LBUTTONDOWN:
		{
		
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
		
	case WM_PAINT:
		{
		
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_VSCROLL:
		{
		
		return 0;
		}
	case WM_HSCROLL:
		{
		
		}
	case WM_MOUSEWHEEL:
		{
		
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
	
	
}

*/


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
	case WM_KEYUP:
		if ( uIdSubclass == HEADER_EDITOR ){
			if ( self->editingHeader >= 0 ) {
				wchar_t * newText = (wchar_t*)malloc(50*sizeof(wchar_t));
				memset(newText, 0, 50);
				GetWindowText(self->headerEditor, newText, 50);
				self->columns[self->editingHeader]->setHeader(newText);

			}
		}
		if ( wParam == VK_RETURN ){
			int previous = self->activeRow;
			self->delegate->editingFinished(hWnd, previous-1, uIdSubclass, self->data);
			self->rows_loaded = max(self->rows_loaded, previous);
			self->page_table[previous-1 % PAGESIZE] = previous-1;

			self->activeRow++;
			if ( self->activeRow > self->delegate->totalRows() ){
				if ( self->delegate->allowNewRows() ) {
					self->delegate->prepareNewRow(self->activeRow-1);
					self->totalHeight = (self->delegate->totalRows()) * self->delegate->rowHeight();
				} else {
					self->activeRow = 1;
				}
			}

			self->startEditing(previous-1, self->activeRow-1, 0);

			if( previous != -1 ){
				self->SetupWindowOffset();
				RECT client;
				GetClientRect(hWnd, &client);
				self->DrawTextForRow(self->offscreenDC, client, previous-1); 
				self->ClearWindowOffset();
				InvalidateRect(hWnd, NULL, true);
			}

		}
		break;
	case WM_KILLFOCUS:
		if ( uIdSubclass == HEADER_EDITOR ){
			self->stopHeaderTitleEditing();
		} else {
			if ( uIdSubclass != TAB_CAPTURE_CLASS && uIdSubclass != REVERSE_TAB_CAPTURE_CLASS ){
				self->delegate->editingFinished(hWnd, self->activelyEditingRow, uIdSubclass, self->data);
				self->rows_loaded = max(self->rows_loaded, self->activelyEditingRow+1);
				self->page_table[self->activelyEditingRow % PAGESIZE] = self->activelyEditingRow;
			}
			self->delegate->willLoseFocus();

			
			/*HWND dest = (HWND)wParam;
			bool focusMovingOutsideGrid = true;
			HWND parent = GetParent(dest);
			while ( parent != HWND_TOP ){
				if ( parent == self->hWnd ) {
					focusMovingOutsideGrid = false;
					break;
				}
				parent = GetParent(parent);
			}
			
			if ( focusMovingOutsideGrid ) {
				self->stopEditing();
			}
			self->delegate->editingFinished(hWnd, self->activeRow-1, uIdSubclass);
			if( focusMovingOutsideGrid ) {
				self->delegate->willLoseFocus();
			}*/
		}
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

			self->startEditing(previousActiveRow-1, self->activeRow-1, -1);
	
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

void CinchGrid::stopHeaderTitleEditing(){
	ShowWindow(headerEditor, SW_HIDE);
	SetupWindowOffset();
	RECT client;
	GetClientRect(hWnd, &client);
	DrawHeader(offscreenDC, client, false);
	InvalidateRect(hWnd, NULL, true);
}

void CinchGrid::startHeaderTitleEditing(int col){
	if ( headerEditor == NULL ){
		headerEditor = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			0, 0, 0, 0, hWnd, NULL, hInst, NULL);
		
		SetWindowSubclass(headerEditor, DetailWndProc, HEADER_EDITOR, (DWORD_PTR)this);
		
		SendMessage(headerEditor, WM_SETFONT, (WPARAM)delegate->getEditFont(), 0);
	}

	SendMessage(headerEditor, WM_SETTEXT, (WPARAM)0, (LPARAM)columns[col]->getHeader().c_str());

	int x = 0;
	int i = 0;
	while( i < col ){
		x+= columns[i]->getWidth();
		i++;
	}

	editingHeader = col;
	SetWindowPos(headerEditor, HWND_TOP, x, 0, columns[col]->getWidth(), delegate->rowHeight(), 0);
	ShowWindow(headerEditor, SW_SHOW);
	SetFocus(headerEditor);
}

void CinchGrid::adjustEditors(){
	int left = 0;
	HWND previousWindow = HWND_TOP;
	for(int i=0; i<numColumns; i++){
		HWND editor = columns[i]->getEditor();
		int width = columns[i]->getWidth();
		SetWindowPos(editor, previousWindow, left-scrollOffsetX, (activeRow) * delegate->rowHeight() - scrollOffsetY, columns[i]->getWidth(), delegate->rowHeight(), 0);
		ShowWindow(editor, SW_SHOW);
		previousWindow = editor;
		left += columns[i]->getWidth();
	}
}

void CinchGrid::startEditing(int previous, int row, int col){
	int left = 0;
	int firstEditableColumn = -1;
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
			if ( firstEditableColumn < 0 ) firstEditableColumn = i;
			if ( columns[i]->getEditor() == NULL ){
				HWND editor = delegate->editorForColumn(i, hWnd, hInst);
				SendMessage(editor, WM_SETFONT, (WPARAM)delegate->getEditFont(), 0);
				SetWindowSubclass(editor, DetailWndProc, i, (DWORD_PTR)this);
				columns[i]->setEditor(editor);
			} else {
				delegate->editingFinished(columns[i]->getEditor(), previous, i, data); 
				rows_loaded = max(rows_loaded, previous+1);
				page_table[previous % PAGESIZE] = previous;
			
			}
			HWND editor = columns[i]->getEditor();
			delegate->setupEditorForCell(editor, row, i, data);
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

	if ( firstEditableColumn >= 0 ){
		SetFocus(columns[firstEditableColumn]->getEditor());
	}
	
	SendMessage(GetFocus(), EM_SETSEL, 0, -1);
	
	activelyEditingRow = row;
	editingInitialized = true;
}

void CinchGrid::stopEditing(){
	for(int i=0; i<numColumns; i++){
		if ( columns[i]->getEditor() != NULL ){
			ShowWindow(columns[i]->getEditor(), SW_HIDE);
		}
	}
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
	clientWidth = client.right;
	FillRect(offscreenDC, &totalArea, solidWhiteBrush);

	DrawHorizontalGridlines(hdc, client);
	DrawVerticalGridlines(hdc, client);
	DrawCellText(hdc, client);

	DrawActiveRow(hdc, client);
	DrawHeaderDragGuideline(hdc, client);

	DrawHeader(hdc, client, false);
}



int CinchGrid::GetActiveRow(){
	return activeRow - 1;
}

void CinchGrid::SetActiveRow(int row, bool silent){
	int previousActiveRow = activeRow;
	activeRow = row;

	RECT client;
	GetClientRect(hWnd, &client);
	
	if( delegate->rowSelection() ){
	
		SetupWindowOffset();
		if (previousActiveRow > 0 ){
			ClearActiveRow(previousActiveRow, offscreenDC, client);
		}
		DrawActiveRow(offscreenDC, client);
		ClearWindowOffset();
		InvalidateRect(hWnd, NULL, true);
				
		if ( !silent ){
			PostMessage(GetParent(hWnd), CINCHGRID_ROW_SELECTED, 0, 0);
			delegate->didSelectRow(GetActiveRow());
		}
	}

	startEditing(previousActiveRow-1, activeRow-1, activeCol);

	if( previousActiveRow != -1 ){
		SetupWindowOffset();
		DrawTextForRow(offscreenDC, client, previousActiveRow-1); 
		ClearWindowOffset();
		InvalidateRect(hWnd, NULL, true);
	}

}

void CinchGrid::setDelegate(GridDelegate* d){
	delegate = d;
	stopEditing(); 
	setupColumns();
	initialize();
}

GridDelegate* CinchGrid::getDelegate(){
	return delegate;
}