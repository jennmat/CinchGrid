// SimpleGrid.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Windowsx.h"
#include <sstream>

#define MAX_LOADSTRING 100

#define DEFAULT_COLUMN_WIDTH 120

#define MAX_OFFSCREEN_WIDTH 400
#define MAX_OFFSCREEN_HEIGHT 1000

TCHAR szClassName[] = _T("CinchGrid");
 
void RegisterCinchGrid()
{
    WNDCLASSEX wc;
	wc.cbSize         = sizeof(wc);
    wc.lpszClassName  = szClassName;
    wc.hInstance      = GetModuleHandle(0);
	wc.lpfnWndProc    = CinchGrid::WndProc;
    wc.hCursor        = NULL;
    wc.hIcon          = 0;
    wc.lpszMenuName   = 0;
	wc.hbrBackground  = (HBRUSH)CreateSolidBrush(RGB(255,255,255));
    wc.style          = CS_DBLCLKS;
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
	mouseHighlighting = false;
	mouseHighlightingStartPos = -1;

	highlightActiveCell = false;
	highlightRangeLow = -1;
	highlightRangeHigh= -1;

	activelyDraggedColumn = -1;

	delegate = d;

	data = nullptr;
	page_table = nullptr;
	rows_loaded = 0;
	
	isCaretVisible = false;
	isCaretCreated = false;

	sizeColumnCursor = LoadCursor(NULL, IDC_SIZEWE);
	iBeamCursor = LoadCursor(NULL, IDC_IBEAM);
	arrowCursor = LoadCursor(NULL, IDC_ARROW);

	initialize();

  
	headerPen = CreatePen(PS_SOLID, 1, RGB(137, 140, 149));
	gridlinesPen = CreatePen(PS_SOLID, 1, RGB(205, 205, 205));
	borderlinesPen = CreatePen(PS_SOLID, 1, RGB(137, 140, 149));
	sortIndicatorPen = CreatePen(PS_SOLID, 1, RGB(165,172,181));
	LOGBRUSH lb = {BS_SOLID, RGB(100, 100, 100), 0}; 
	activeRowPen = ExtCreatePen(PS_COSMETIC | PS_ALTERNATE | PS_ENDCAP_SQUARE | PS_JOIN_ROUND, 1, &lb, 0, NULL);
	//activeCellPen = CreatePen(PS_SOLID, 3, RGB(65,65,65));
	LOGBRUSH darkb = {BS_SOLID, RGB(65, 65, 65), 0}; 
	activeCellPen = ExtCreatePen(PS_GEOMETRIC | PS_ENDCAP_ROUND | PS_JOIN_ROUND, 1, &darkb, 0, NULL);
	transparencyColor = RGB(255, 20, 147);
	solidWhiteBrush = CreateSolidBrush(RGB(255,255,255));
	inactiveBackgroundBrush = CreateSolidBrush(RGB(255,255,255));
	solidHotPinkBrush = CreateSolidBrush(transparencyColor);
	activeRowBrush = CreateSolidBrush(RGB(167,205,240));
	sortIndicatorBrush = CreateSolidBrush(RGB(165,172,181));
		
	offscreenDC = CreateCompatibleDC(GetDC(hWnd));
	offscreenActiveDC = CreateCompatibleDC(GetDC(hWnd));

	/*WORD		wBits[] = { 0x00 };
	HBITMAP     bmpBrush;
	bmpBrush  = CreateBitmap(32, 32, 1, 1, wBits);
    inactiveBackgroundBrush = CreatePatternBrush(bmpBrush);*/
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

void CinchGrid::initializeLayout(){
	setupColumns();
}

void CinchGrid::initializeData(){
	RECT client;
	GetClientRect(hWnd, &client);
	
	clientWidth = client.right;

	if ( data != nullptr ){
		CleanupData();
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
		memset(data[i], NULL, numColumns*sizeof(wchar_t*));
	}
	page_table = new int[PAGESIZE];
	memset(page_table, 0xffff, PAGESIZE);
}

void CinchGrid::initialize(){
	initializeLayout();
	initializeData();
	
}

void CinchGrid::CleanupData(){
	if ( data != nullptr ){
		delegate->CleanupSegment(rows_loaded, cols_loaded, data);
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

void CinchGrid::reloadLayout(){
	initializeLayout();
}

void CinchGrid::reloadData(){
	delegate->willReloadData();
	
	activeRow = -1;
	initializeData();

	SetScroll(hWnd);

	SetupAndDrawOffscreenBitmap();
	InvalidateRect(hWnd, NULL, true);

	delegate->didReloadData();

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

	offscreenActiveElements = CreateCompatibleBitmap(GetDC(hWnd), offscreenWidth, offscreenHeight);
	SelectObject(offscreenActiveDC, offscreenActiveElements);

	totalArea.left = 0;
	totalArea.right = offscreenWidth;
	totalArea.bottom = offscreenHeight;
	totalArea.top = 0;
	
	int l = FillRect(offscreenDC, &totalArea, inactiveBackgroundBrush);
	int rc = FillRect(offscreenActiveDC, &totalArea, solidHotPinkBrush);
	int err = GetLastError();

	SetupWindowOffset();
	DrawGridElements(offscreenDC, totalArea);
	ClearWindowOffset();
}

void CinchGrid::DrawHeaderDragGuideline(HDC hdc, RECT client )
{
	if ( draggingHeader == true ){
		SelectObject(hdc, gridlinesPen);	
	
		MoveToEx(hdc, draggedXPos, delegate->rowHeight(), NULL);
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
	if ( delegate->allowNewRows() ){
		bottom = totalArea.bottom;
	} else {
		bottom = delegate->rowHeight() * (delegate->totalRows()+1);
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
			bottom = min(offscreenHeight, delegate->rowHeight() * (delegate->totalRows() + 1)) + 1;
		} else {
			bottom = offscreenHeight + 1;
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
	for(int col = 0; col<delegate->totalColumns(); col++){
		DrawTextForCell(hdc, row, col);
	}
}

void CinchGrid::DrawTextForCell(HDC hdc, int row, int col){
	int left = 0;
	int top = delegate->rowHeight() * (row+1) + 1;
	for(int i=0; i<numColumns && i<col; i++){
		left += columns[i]->getWidth();
	}
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
	bool highlighting = false;
	if ( row == activeRow && col == activeCol ){
		if ( highlightActiveCell ){
			highlighting = true;
		}
	}
	left += width;
	if ( row == activeRow && delegate->rowSelection() == true ){
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
			delegate->CleanupSegment(rows_loaded, cols_loaded, data);
		}
		delegate->LoadSegment(row, len, data, &rows_loaded, &cols_loaded);
		int j = row;
		for(int i=row; i<row+PAGESIZE; i++){
			page_table[i%PAGESIZE] = j++;
		}
	}

	if ( highlighting ) {
		RECT beforeHighlight, highlighted, afterHighlight;
		beforeHighlight.left = textRect.left; beforeHighlight.top = textRect.top+1;
		beforeHighlight.right = textRect.left; beforeHighlight.bottom = textRect.bottom+1;
		DrawText(hdc, data[row%PAGESIZE][col], highlightRangeLow, &beforeHighlight, DT_CALCRECT);
		
		highlighted.left = beforeHighlight.right;
		highlighted.right = beforeHighlight.right;
		highlighted.top = beforeHighlight.top;
		highlighted.bottom = beforeHighlight.bottom;
		
		
		DrawText(hdc, data[row%PAGESIZE][col]+highlightRangeLow, highlightRangeHigh - highlightRangeLow, &highlighted, DT_CALCRECT);
		
		RECT highlightedText;
		highlightedText.left = highlighted.left;
		highlightedText.right = highlighted.right;
		highlightedText.top = highlighted.top;
		highlightedText.bottom = highlighted.bottom;

		highlighted.top -= 1;
		highlighted.bottom -= 1;


		afterHighlight.left = highlightedText.right;
		afterHighlight.right = highlightedText.right;
		afterHighlight.top = highlightedText.top;
		afterHighlight.bottom = highlightedText.bottom;
		int len = wcslen(data[row%PAGESIZE][col]);
		DrawText(hdc, data[row%PAGESIZE][col]+highlightRangeHigh, len - highlightRangeHigh, &afterHighlight, DT_CALCRECT);
		

		SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
		DrawText(hdc, data[row%PAGESIZE][col], highlightRangeLow, &beforeHighlight, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
		DrawText(hdc, data[row%PAGESIZE][col]+highlightRangeHigh, len - highlightRangeHigh, &afterHighlight, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
		
		
		SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
		HBRUSH hbrush = GetSysColorBrush(COLOR_HIGHLIGHT);
		highlighted.top += 2;
		highlighted.bottom += 1;
		FillRect(hdc, &highlighted, hbrush);
		DrawText(hdc, data[row%PAGESIZE][col]+highlightRangeLow, highlightRangeHigh - highlightRangeLow, &highlightedText, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
		
	} else {
		SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
		DrawText(hdc, data[row%PAGESIZE][col], -1, &textRect, DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
	}
	
}

void CinchGrid::ClearActiveRow(int row, HDC hdc, RECT client)
{
	RECT rect;
	rect.left = 0;
	if ( autosized ) rect.right = clientWidth;
	else  rect.right = totalWidth;
	rect.top = (row+1) * delegate->rowHeight() ;
	rect.bottom = rect.top + delegate->rowHeight()+1;
	FillRect(hdc, &rect, solidWhiteBrush);
	//Gridlines
	if ( delegate->drawHorizontalGridlines()){
		DrawHorizontalGridlines(hdc, client);
	}
	if ( delegate->drawVerticalGridlines()){
		DrawVerticalGridlines(hdc, client);
	}
	DrawTextForRow(hdc, client, row);
}

void CinchGrid::DrawActiveRow(HDC hdc, RECT client)
{
	if( delegate->rowSelection() == false ){
		return;
	}
	if ( activeRow >= 0){
			
		SelectObject(hdc, activeRowPen);
		SelectObject(hdc,GetStockObject(NULL_BRUSH));
		RECT row;
		row.left = 0;
		if ( autosized ) row.right = clientWidth;
		else  row.right = totalWidth;
		row.top = (activeRow+1) * delegate->rowHeight();
		row.bottom = row.top + delegate->rowHeight()+1;
		FillRect(hdc, &row, activeRowBrush);
		Rectangle(hdc, row.left, row.top, row.right, row.bottom);

		HFONT hFont = delegate->getFont();
		SelectObject(hdc, hFont);
		SetBkMode(hdc, TRANSPARENT);
	
		DrawTextForRow(hdc, client, activeRow);
	}
}

void CinchGrid::DrawRows(HDC hdc, RECT client)
{
	SetBkMode(hdc, TRANSPARENT);

	HFONT hFont = delegate->getFont();
	SelectObject(hdc, hFont);
		
	int j = 0;
	int left = 0;
	if ( windowOffsetY < 0 ){
		int a = 1;
	}
	for(int row = windowOffsetY / delegate->rowHeight(); row<delegate->totalRows() && row*delegate->rowHeight() < offscreenHeight; row++){
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
		case VK_DELETE:
			{
				return OnChar(wParam, lParam);
			}
		case VK_RETURN:
			if ( GetKeyState(VK_SHIFT) & 0x8000) {
				SetActiveCell(activeRow-1, activeCol);
			} else {
				SetActiveCell(activeRow+1,activeCol);
			}
			break;
		case VK_TAB:
			if ( GetKeyState(VK_SHIFT) & 0x8000 ){
				if ( activeCol -1 < 0) {
					SetActiveCell(activeRow-1, delegate->totalColumns() - 1);
				} else {
					SetActiveCell(activeRow, activeCol-1);
				}

			} else {
				if ( activeCol + 1 >= delegate->totalColumns() ) {
					SetActiveCell(activeRow+1, 0);
				} else {
					SetActiveCell(activeRow, activeCol+1);
				}
			}
			break;
		case VK_LEFT:
			if ( activeCol - 1 >= 0 ){
				SetActiveCell(activeRow, activeCol-1);
			}
			break;
		case VK_RIGHT:
			if ( activeCol + 1 < delegate->totalColumns() ){
				SetActiveCell(activeRow, activeCol+1);
			}
			break;
		case VK_UP:
			if( activeRow > 0 ){
				if ( delegate->rowSelection() ){
					SetActiveRow(activeRow-1);
					ScrollRowIntoViewFromBeneath(activeRow+1);
				} else {
					SetActiveCell(activeRow-1, activeCol);
				}
			}
			break;
		case VK_DOWN:
			if ( activeRow + 1 <= delegate->totalRows() ){
				if ( delegate->rowSelection() == true ){
					SetActiveRow(activeRow+1);					
					ScrollRowIntoView(activeRow+1);	
				} else {
					SetActiveCell(activeRow+1, activeCol);
				}
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
		case 'c':
		case 'C':
			if ( GetKeyState(VK_CONTROL) & 0x8000 ) {
				Copy();
				return 0;
			}
			break;
		case 'v':
		case 'V':
			if ( GetKeyState(VK_CONTROL) & 0x8000 ){
				Paste();
				return 0;
			}
			break;
		}
		
		
	}
	SetFocus(hWnd);
	return MA_ACTIVATE;
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
			
		totalWidth = 0;
		for(int i=0; i<numColumns; i++){
			totalWidth += columns[i]->getWidth();
		}
		draggingHeader = false;
	
		SetupAndDrawOffscreenBitmap();

		InvalidateRect(hWnd, NULL, true);

		delegate->didChangeColumnWidth(activelyDraggedColumn, mouseXPos-accum);
	}

	mouseHighlighting = false;
	mouseHighlightingStartPos = -1;

	return MA_ACTIVATE;
}

LRESULT CinchGrid::OnLButtonDoubleClick(WPARAM wParam, LPARAM lParam){	
	if ( delegate->allowEditing(activeCol) ){
		int len = wcslen(data[activeRow][activeCol]);
		if ( len > 0 ){
			highlightActiveCell = true;
			highlightRangeLow = 0;
			highlightRangeHigh = len;
			caretPos = highlightRangeHigh;
			DrawTextForCell(offscreenDC, activeRow, activeCol);
			RepositionCursor(activeRow, activeCol, highlightRangeHigh);
			RECT rect;
			GetCellRect(activeRow, activeCol, &rect);
			InvalidateRect(hWnd, &rect, true);
		}
	}
	return TRUE;
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

		if ( !draggingHeader && delegate->allowSorting(clickedColumn) == true ){
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
		}
	} else {
		int clickedCol = -1;

		int accum = 0;
		for(int i=0; i<numColumns; i++){
			accum += columns[i]->getWidth();
			if ( accum > xPos ){
				clickedCol = i;
				break;
			}
		}
		int clickedRow = (yPos + scrollOffsetY) / delegate->rowHeight();
		//To account for header space
		clickedRow--;

		if ( delegate->allowNewRows() && clickedRow >= delegate->totalRows() ){
			int newRows = clickedRow - delegate->totalRows() + 1;
			for(int i=0; i<newRows; i++){
				delegate->prepareNewRow(clickedRow);
			}
			totalHeight = (delegate->totalRows()) * delegate->rowHeight();
		}

		//To account for the header space.
		caretPos = 0;
			
		if ( clickedRow == activeRow && clickedCol == activeCol ){
			/* Clicking inside the already active cell.  This means we want to re-position the cursor based on the click */
			wchar_t* content = data[activeRow][activeCol];
			if ( content != nullptr && wcslen(content) > 0 ){
				/* Since the font may be variable-width, use a binary search to find the closest insertion point */
				int minimizedXPos;
				RECT rect;
				GetCellRect(activeRow, activeCol, &rect);
				FindInsertionPoint(xPos, &caretPos, &minimizedXPos);
				SetCaretPos(minimizedXPos, rect.top + 4);

				if ( highlightRangeHigh > 0 ){
					//Turn off the highlighting
					highlightActiveCell = false;
					highlightRangeHigh = -1;
					highlightRangeLow = -1;
					RedrawCell(activeRow, activeCol);
				}

				mouseHighlighting = true;
				mouseHighlightingStartPos = caretPos;
			}
		} else {
			if ( clickedRow <= delegate->totalRows() ){
				SetActiveRow(clickedRow);
			}

			if ( clickedCol >= 0 ){
				SetActiveCell(clickedRow, clickedCol );
			}
		}

		if ( delegate->allowEditing(clickedCol) == false ){
			//InvalidateRect(hWnd, NULL, true);
		}


		
	}

	return MA_ACTIVATE;
}

void CinchGrid::FindInsertionPoint(int mouseClickXPos, int* pos, int* xPos ){
	wchar_t* content = data[activeRow][activeCol];
	RECT rect;
	GetCellRect(activeRow, activeCol, &rect);
	int cellRelativeXPos = mouseClickXPos - rect.left;
	RECT textRect;
	int len = wcslen(content);
	int minimumDist = INT_MAX;
	int minimizedXPos = 0;
	for(int i=0; i<=len; i++){
		textRect.left = 0; textRect.right = 0;
		textRect.top = 0; textRect.bottom = 0;
		int rc = DrawText(offscreenDC, content, i, &textRect, DT_CALCRECT);
		if ( abs(cellRelativeXPos - textRect.right) < minimumDist ){
			minimizedXPos = textRect.right + 4;
			minimumDist = abs(cellRelativeXPos - textRect.right);
			caretPos = i;
		}
	}

	*xPos = rect.left + minimizedXPos;
	*pos = caretPos;

}

LRESULT CinchGrid::OnMouseMove(WPARAM wParam, LPARAM lParam){
	int mouseXPos = GET_X_LPARAM(lParam); 
	int mouseYPos = GET_Y_LPARAM(lParam); 
		
	if ( draggingHeader ){
		SetCursor(sizeColumnCursor);

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
	} else if ( mouseYPos < delegate->rowHeight() ){
		int accum = 0;
		bool hoverColDivide = false;
		for(int i=0; i<numColumns; i++){
			accum += columns[i]->getWidth();
			if ( abs(accum-mouseXPos) < 10 ){
				hoverColDivide = true;
			}
		}
		if ( hoverColDivide == true ){
			SetCursor(sizeColumnCursor);
		} else {
			SetCursor(arrowCursor);
		}
	} else if ( delegate->allowEditing(activeCol) ){
		RECT rect;
		GetCellRect(activeRow, activeCol, &rect);
		if ( mouseXPos > rect.left && mouseXPos < rect.right &&
			mouseYPos > rect.top && mouseYPos < rect.bottom ){
			SetCursor(iBeamCursor);
		} else {
			SetCursor(arrowCursor);
		}
	
		if ( mouseHighlighting) {
			int pos;
			int xpos;
			FindInsertionPoint(mouseXPos, &pos, &xpos);
			highlightActiveCell = true;
			highlightRangeLow = mouseHighlightingStartPos;
			highlightRangeHigh = pos;
			if ( highlightRangeLow > highlightRangeHigh ){
				swap(highlightRangeHigh, highlightRangeLow);
			}
			RedrawCell(activeRow, activeCol);
			RepositionCursor(activeRow,activeCol, highlightRangeHigh);
		}
	} else {
		SetCursor(arrowCursor);
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
	
	RECT rect;
	SelectObject(hdc, borderlinesPen);
	GetClientRect(hWnd, &rect);
	MoveToEx(hdc, 0, 0, NULL);
	LineTo(hdc, rect.right-1, 0);
	LineTo(hdc, rect.right-1, rect.bottom-1);
	LineTo(hdc, 0, rect.bottom-1);
	LineTo(hdc, 0, 0);

	TransparentBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, offscreenActiveDC, ps.rcPaint.left + scrollOffsetX, ps.rcPaint.top + scrollOffsetY - windowOffsetY, ps.rcPaint.right, ps.rcPaint.bottom, transparencyColor);
	
	//BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, offscreenActiveDC, ps.rcPaint.left + scrollOffsetX, ps.rcPaint.top + scrollOffsetY - windowOffsetY, SRCCOPY);
	
	if( draggingHeader ){
		DrawHeaderDragGuideline(hdc, ps.rcPaint);
	}

	if ( delegate->stickyHeaders() ){
		DrawHeader(hdc, ps.rcPaint, true);
	}

	
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
		AdjustWindow();
		InvalidateRect(hWnd, NULL, true);
	} else if ( cmd == SB_PAGERIGHT ){
		RECT client;
		GetClientRect(hWnd, &client);
		scrollOffsetX += client.right;
		if( scrollOffsetX > totalWidth - client.right ){
			scrollOffsetX = totalWidth - client.right;
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
		}
		SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
			
		AdjustWindow();
		InvalidateRect(hWnd, NULL, true);
	} else if ( cmd == SB_LINEUP ){
		scrollOffsetY -= delegate->rowHeight();
		if( scrollOffsetY < 0 ){
			scrollOffsetY = 0;
		} else {
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
		
		SetScrollPos(hWnd, SB_VERT, scrollOffsetY, true);
		AdjustWindow();
		InvalidateRect(hWnd, NULL, true);
	}
	return 0;
}

void CinchGrid::GetCellForMousePos(int mouseX, int mouseY, int* row, int *cell){

}

void CinchGrid::GetStartCursorPos(int row, int col, int* x, int *y){
	*x=0;
	for(int i=0; i<numColumns && i<col; i++){
		*x += columns[i]->getWidth();
	}
	*x += 3;
	*y = delegate->rowHeight() * (row+1) + 4;
}

void CinchGrid::RepositionCursor(int row, int col, int pos){
	/* Measure the text so I can move the cursor */
	RECT textRect;
	textRect.left = 0; textRect.right = 0;
	textRect.top = 0; textRect.bottom = 0;
	int rc = DrawText(offscreenDC, data[row][col], pos, &textRect, DT_CALCRECT);
	int x, y;
	if ( !isCaretCreated ){
		TEXTMETRIC m;
		GetTextMetrics(offscreenDC, &m);
		CreateCaret(hWnd, (HBITMAP) NULL, 1, m.tmHeight); 
		isCaretCreated = true;
	}

	GetStartCursorPos(row, col, &x, &y);
	SetCaretPos(x+textRect.right, y);

	if ( !isCaretVisible ){
		ShowCaret(hWnd);
		isCaretVisible = true;
	}
}

void CinchGrid::Cut(){
}

void CinchGrid::Paste(){
	HGLOBAL hglb = nullptr;
    LPTSTR lptstr; 

	if (!IsClipboardFormatAvailable(CF_TEXT)) 
		return; 
	if (!OpenClipboard(hWnd)) 
		return; 


	lptstr = (LPTSTR)GetClipboardData(CF_TEXT); 
    if (hglb != nullptr) 
    { 
        lptstr = (LPTSTR)GlobalLock(hglb); 
        if (lptstr != NULL) 
        { 
            // Call the application-defined ReplaceSelection 
            // function to insert the text and repaint the 
            // window. 
		
            //ReplaceSelection(hwndSelected, pbox, lptstr); 
            GlobalUnlock(hglb); 
        } 
    } 
    CloseClipboard(); 
 
}

void CinchGrid::Copy(){
	HGLOBAL hglbCopy; 
	LPTSTR  lptstrCopy; 

	int len = highlightRangeHigh - highlightRangeLow;

	if ( len <= 0 ){
		return;
	}

	
	if ( !OpenClipboard(hWnd) ){
		return;
	}

	EmptyClipboard();

	hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(TCHAR)); 
    if (hglbCopy == NULL) { 
        CloseClipboard(); 
        return; 
    }


	 lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 
	 memcpy(lptstrCopy, data[activeRow][activeCol]+highlightRangeLow, len * sizeof(TCHAR)); 
     lptstrCopy[len] = (TCHAR) 0;    // null character 
     GlobalUnlock(hglbCopy); 
 
	 // Place the handle on the clipboard. 
 
	HANDLE rc = SetClipboardData(CF_TEXT, hglbCopy); 
	CloseClipboard();	
	
}

void CinchGrid::EraseHighlightedRange(){
	wchar_t* content = data[activeRow][activeCol];
	wchar_t* replace = new wchar_t[200];
	int len = wcslen(content);
	memset(replace, 0, 200*sizeof(wchar_t));
	if ( highlightRangeLow > 0 ){
		wcsncpy_s(replace, 200, content, highlightRangeLow);
	}
	if ( len - highlightRangeHigh > 0 ){
		wcsncat_s(replace, 200, content+highlightRangeHigh, len - highlightRangeHigh);
	}
	delete content;
	data[activeRow][activeCol] = replace;
	caretPos = highlightRangeLow;
	highlightActiveCell = false;
	highlightRangeHigh = -1;
	highlightRangeLow = -1;
	RepositionCursor(activeRow, activeCol, caretPos);
	RedrawCell(activeRow, activeCol);		
}

LRESULT CinchGrid::OnChar(WPARAM wParam, LPARAM lParam){
	switch (wParam) 
    { 
        case 0x0A: 
            break; 
        case 0x1B: 
            break; 
        case 0x09: 
            break; 
        case 0x0D: 
            break; 
		case VK_DELETE:
		case VK_BACK:
			{
				if ( highlightRangeLow >= 0 ){
					EraseHighlightedRange();
				} else {
					wchar_t* content = data[activeRow][activeCol];
					int len = wcslen(content) + 1;
					if ( len > 1 ){
						int i;
						if ( wParam == VK_BACK ) i = caretPos-1;
						if ( wParam == VK_DELETE ) i = caretPos;
						wchar_t tmp;
						while ( i < len ){
							tmp = content[i+1];
							content[i+1] = content[i];
							content[i] = tmp;
							i++;
						}
						content[len-1] = '\0';
						if ( wParam == VK_BACK ) caretPos--;
						RepositionCursor(activeRow, activeCol, caretPos);
						RedrawCell(activeRow, activeCol);		
					}
				}

				
			}
			break;
		default:
			{
			
			if ( highlightRangeHigh > 0 ){
				EraseHighlightedRange();
			}
			wchar_t c = (wchar_t)wParam;
	
			if( data[activeRow][activeCol] == NULL ){
				data[activeRow][activeCol] = new wchar_t[100];
				memset(data[activeRow][activeCol], 0, 100);
				data[activeRow][activeCol][0] = c;
				caretPos = 1;
				RepositionCursor(activeRow, activeCol, caretPos);
			} else {
				wchar_t* content = data[activeRow][activeCol];
				int i = wcslen(content) + 1;
				wchar_t tmp;
				while ( i > caretPos ){
					tmp = content[i];
					content[i] = content[i-1];
					content[i-1] = tmp;
					i--;
				}
				content[caretPos] = c;
				caretPos++;
				RepositionCursor(activeRow, activeCol, caretPos);
			}

			RedrawCell(activeRow, activeCol);
			}
			break;
	}
	
	
	return 0;
}

void CinchGrid::RedrawCell(int row, int col){
	SetupWindowOffset();
	DrawTextForCell(offscreenDC, row, col);
	ClearWindowOffset();
	RECT rect;
	GetCellRect(row, col, &rect);
	InvalidateRect(hWnd, &rect, true);
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
		DeleteObject(grid->activeRowPen);
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
	case WM_CHAR:
		return grid->OnChar(wParam, lParam);
	case WM_GETDLGCODE:
		return DLGC_WANTARROWS;
	case WM_MOUSEACTIVATE:
		SetFocus(hWnd);
		return MA_ACTIVATE;
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
	case WM_LBUTTONDBLCLK:
		return grid->OnLButtonDoubleClick(wParam, lParam);
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
	case WM_KILLFOCUS:
		if ( grid->isCaretVisible ){
			HideCaret(hWnd);
			grid->isCaretVisible = false;
		}
		break;
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




void CinchGrid::DrawGridElements(HDC hdc, RECT client)
{
	clientWidth = client.right;
	FillRect(offscreenDC, &totalArea, inactiveBackgroundBrush);
	int height = 0;
	if ( delegate->allowNewRows() ){
		height = offscreenHeight;
	} else {
		height = delegate->rowHeight() * (delegate->totalRows() + 1);
	}

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = totalWidth;
	rect.bottom = height;
	FillRect(hdc, &rect, solidWhiteBrush);
	
	DrawHorizontalGridlines(hdc, client);
	DrawVerticalGridlines(hdc, client);
	DrawRows(hdc, client);

	DrawActiveRow(hdc, client);
	DrawActiveCol();
	DrawHeaderDragGuideline(hdc, client);

	DrawHeader(hdc, client, false);
}



int CinchGrid::GetActiveRow(){
	return activeRow;
}

void CinchGrid::DrawActiveCol(){
	if ( activeRow > -1 && delegate->rowSelection() == false ){
		FillRect(offscreenActiveDC, &totalArea, solidHotPinkBrush);
		SelectObject(offscreenActiveDC, activeCellPen);
		HBRUSH transparent = CreateSolidBrush(transparencyColor);

		SetBkMode(offscreenActiveDC, TRANSPARENT);
		SelectObject(offscreenActiveDC, transparent);
		RECT rect;
		GetCellRect(activeRow, activeCol, &rect);
		Rectangle(offscreenActiveDC, rect.left, rect.top, rect.right-1, rect.bottom);
	}
}

void CinchGrid::SetActiveCell(int row, int col){
	highlightActiveCell = false;
	highlightRangeHigh = -1;
	highlightRangeLow = -1;

	RECT rect;
	GetCellRect(activeRow, activeCol, &rect);
	

	if ( delegate->allowEditing(col) ){
		activeCol = col;
		activeRow = row;
		RECT client;
		GetClientRect(hWnd, &client);
		if ( row >= 0 ){
			int len = 0;
			if ( data[row][col] != nullptr ){
				int len = wcslen(data[row][col]);
			}
			RepositionCursor(row, col, len);
			caretPos = len;

		}
 		SetupWindowOffset();
		DrawActiveCol();
		
		ExpandRectToIncludeCell(activeRow, activeCol, &rect);
		InvalidateRect(hWnd, &rect, true);

		SetCursor(iBeamCursor);
		ClearWindowOffset();
	}
}

void CinchGrid::GetCellRect(int row, int col, LPRECT rect){
	int i = 0;
	rect->left = 0;
	rect->top = 0;
	for(int i=0; i<numColumns && i<col; i++){
		rect->left += columns[i]->getWidth();
	}
	rect->right = rect->left + columns[i]->getWidth() + 1;
	rect->left -= 1;
	if ( rect->left < 0 ) rect->left = 0;
	rect->top = (row+1) * delegate->rowHeight();
	rect->bottom = rect->top + delegate->rowHeight() + 1;
}

void CinchGrid::ExpandRectToIncludeCell(int row, int col, LPRECT rect) {
	RECT o;
	GetCellRect(row, col, &o);
	rect->left = min(o.left, rect->left);
	rect->right = max(o.right, rect->right);
	rect->top = min(o.top, rect->top);
	rect->bottom = max(o.bottom, rect->bottom);
}

void CinchGrid::SetActiveRow(int row, bool silent){
	int previousActiveRow = activeRow;
	activeRow = row;

	RECT client;
	GetClientRect(hWnd, &client);
	
	RECT rect;
	if ( previousActiveRow >= 0 ){
		GetCellRect(previousActiveRow, 0, &rect);
	} else {
		GetCellRect(activeRow, 0, &rect);
	}
	ExpandRectToIncludeCell(activeRow, delegate->totalColumns(), &rect);

	if( delegate->rowSelection() ){
	
		SetupWindowOffset();
		if (previousActiveRow >= 0 ){
			ClearActiveRow(previousActiveRow, offscreenDC, client);
		}
		DrawActiveRow(offscreenDC, client);
		ClearWindowOffset();
				
		if ( !silent ){
			PostMessage(GetParent(hWnd), CINCHGRID_ROW_SELECTED, 0, 0);
			delegate->didSelectRow(GetActiveRow());
		}
	}

	if( previousActiveRow != -1 ){
		SetupWindowOffset();
		DrawTextForRow(offscreenDC, client, previousActiveRow); 
		ClearWindowOffset();
	}

	Repaint(&rect);



}

void CinchGrid::Repaint(LPRECT rect){
	int off = scrollOffsetY;
	rect->top -= off;
	rect->bottom -= off;
	InvalidateRect(hWnd, rect, true);
	rect->top += off;
	rect->bottom += off;
}

void CinchGrid::setDelegate(GridDelegate* d){
	delegate = d;
	setupColumns();
	initialize();
}

GridDelegate* CinchGrid::getDelegate(){
	return delegate;
}