#pragma once

#include "resource.h"
#include "stdafx.h"

#define MAX_GRID_COLUMNS 1000

#define MAX_SCROLL_RANGE 10000

#define LEFT_MARGIN 3

#define COL_SPACING 100

#define TAB_CAPTURE_CLASS -100
#define REVERSE_TAB_CAPTURE_CLASS -200

#define HEADER_EDITOR 998

#define CINCHGRID_ROW_SELECTED (WM_APP + 0x0102)

#define PAGESIZE 200

class CinchGrid {
private:
	
	int numColumns;
	int numRows;
	GridColumn* columns[MAX_GRID_COLUMNS];
	GridDelegate* delegate;

	wchar_t ***data;
	int* page_table;
	int rows_loaded;
	int cols_loaded;

	HWND hWnd;
	HINSTANCE hInst;

	HWND headerEditor;

	int activeRow;// = -1;
	int activelyEditingRow;
	int activeCol;// = -1;

	bool highlightActiveCell;
	int highlightRangeLow;
	int highlightRangeHigh;

	int sortedColumn;

	int scrollOffsetX;// = 0;
	int scrollOffsetY;// = 0;

	int offscreenWidth;
	int offscreenHeight;

	long windowOffsetY;

	bool overflowX;// = 0;
	bool overflowY;// = 0;

	/* Width of the grid content */
	int totalWidth;// = 0;
	int totalHeight;// = 0;

	/*Width of the window we are living in*/
	int clientWidth;

	/* True if at least one column is autosized (i.e. set to max width)*/
	bool autosized;
	int numAutosizedCols;
	int totalWidthFixedCols;

	bool draggingHeader;// = false;
	bool editingInitialized;// = false;
	int draggedXPos;// = 0;
	bool mouseHighlighting;
	int mouseHighlightingStartPos;

	int editingHeader;
	bool isCaretVisible;
	bool isCaretCreated;
	int caretPos;

	int activelyDraggedColumn;// = -1;

	HPEN headerPen;
	HPEN gridlinesPen;
	HPEN borderlinesPen;
	HPEN sortIndicatorPen;
	HPEN activeRowPen;
	HPEN activeCellPen;

	COLORREF transparencyColor;
	HBRUSH solidWhiteBrush;
	HBRUSH inactiveBackgroundBrush;
	HBRUSH activeRowBrush;
	HBRUSH sortIndicatorBrush;
	HBRUSH solidHotPinkBrush;


	HDC offscreenDC;
	HDC offscreenActiveDC;
	HBITMAP offscreenBitmap;
	HBITMAP offscreenActiveElements;
	RECT totalArea;

	HWND firstFocusedEditor;
	HWND lastFocusedEditor;

	HCURSOR sizeColumnCursor;
	HCURSOR iBeamCursor;
	HCURSOR arrowCursor;
	
	/*When the tab hits this control, move to the next row */
	HWND tabCapture;
	/* Move to the previous row when the tab hits this control */
	HWND reverseTabCapture;

	void addColumn(wstring header, int width);

	void initialize();
	void initializeLayout();
	void initializeData();
	void CleanupData();
	void setupColumns();

	/*Editing */
	void SetScroll(HWND hWnd);

	void PageDown();
	void PageUp();

	void ScrollRowIntoViewFromBeneath(int row);

	/* Painting */
	void AdjustWindow();
	void SetupWindowOffset();
	void ClearWindowOffset();
	void SetupAndDrawOffscreenBitmap();
	void DrawGridElements(HDC hdc, RECT client);
	void DrawHeaderDragGuideline(HDC hdc, RECT client);
	void DrawHeader(HDC hdc, RECT client, bool fromPaintRoutine );
	void DrawVerticalGridlines(HDC hdc, RECT client);
	void DrawHorizontalGridlines(HDC hdc, RECT client);
	void DrawTextForRow(HDC hdc, RECT client, int row);
	void DrawTextForCell(HDC hdc, int row, int col);
	void DrawRows(HDC hdc, RECT client);
	void DrawActiveRow(HDC hdc, RECT client);
	void ClearActiveRow(int row, HDC hdc, RECT client);
	void RedrawCell(int row, int col);
	void FindInsertionPoint(int mouseClickXPos, int* pos, int* xPos);
	void EraseHighlightedRange();
	void DrawActiveCol();
	
	void Cut();
	void Copy();
	void Paste();

	void SetActiveCell(int row, int col);

	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDoubleClick(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnVScroll(WPARAM wParam, LPARAM lParam);
	LRESULT OnHScroll(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnChar(WPARAM wParam, LPARAM lParam);

	void GetStartCursorPos(int row, int col, int* x, int* y);
	void GetCellForMousePos(int mouseX, int mouseY, int* row, int *col);
	void GetCellRect(int row, int col, LPRECT rect);
	void ExpandRectToIncludeCell(int row, int col, LPRECT rect);
	void RepositionCursor(int row, int col, int pos);
	void Repaint(LPRECT rect);
public:

	

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK DetailWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	void setDelegate(GridDelegate* d);
	GridDelegate* getDelegate();
	void reloadData();
	void reloadLayout();
	int GetActiveRow();
	void SetActiveRow(int row, bool silent=false);
	void ScrollRowIntoView(int row);

	
	CinchGrid(HWND hWnd, HINSTANCE hInst, GridDelegate * delegate);
	~CinchGrid();

	static HWND CreateCinchGrid(HWND parent, GridDelegate * delegate);
};


