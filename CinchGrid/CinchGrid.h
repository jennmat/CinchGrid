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

class CinchGrid {
private:
	
	int numColumns;
	GridColumn* columns[MAX_GRID_COLUMNS];
	GridDelegate* delegate;

	HWND hWnd;
	HINSTANCE hInst;

	HWND headerEditor;

	int activeRow;// = -1;
	int activelyEditingRow;
	int activeCol;// = -1;

	int scrollOffsetX;// = 0;
	int scrollOffsetY;// = 0;

	int offscreenWidth;
	int offscreenHeight;

	long windowOffsetY;

	bool overflowX;// = 0;
	bool overflowY;// = 0;

	int totalWidth;// = 0;
	int totalHeight;// = 0;

	bool draggingHeader;// = false;
	bool editingInitialized;// = false;
	int draggedXPos;// = 0;

	int editingHeader;

	int activelyDraggedColumn;// = -1;

	HPEN headerPen;
	HPEN gridlinesPen;

	HBRUSH solidWhiteBrush;
	HBRUSH activeRowBrush;

	HDC offscreenDC;
	HBITMAP offscreenBitmap;
	RECT totalArea;

	HWND firstFocusedEditor;
	HWND lastFocusedEditor;

	/*When the tab hits this control, move to the next row */
	HWND tabCapture;
	/* Move to the previous row when the tab hits this control */
	HWND reverseTabCapture;

	void clearColumns();
	void addColumn(wchar_t * header, int width);

	void initialize();

	/*Editing */
	void startEditing(int previous, int row, int col);
	void stopEditing();
	void stopHeaderTitleEditing();
	void startHeaderTitleEditing(int col);
	void scrollEditors(int offsetX, int offsetY);
	void adjustEditors();
	void SetScroll(HWND hWnd);

	void PageDown();
	void PageUp();

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
	void DrawCellText(HDC hdc, RECT client);
	void DrawActiveRow(HDC hdc, RECT client);
	void ClearActiveRow(int row, HDC hdc, RECT client);
	
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnVScroll(WPARAM wParam, LPARAM lParam);
	LRESULT OnHScroll(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);

public:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK DetailWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	void setDelegate(GridDelegate* d);
	GridDelegate* getDelegate();
	void reloadData();
	int GetActiveRow();
	void SetActiveRow(int row);
	void ScrollRowIntoView(int row);

	CinchGrid(HWND hWnd, HINSTANCE hInst, GridDelegate * delegate);

	static HWND CreateCinchGrid(HWND parent, GridDelegate * delegate);
};


