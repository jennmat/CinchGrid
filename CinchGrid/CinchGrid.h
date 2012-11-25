#pragma once

#include "resource.h"
#include "stdafx.h"

#define MAX_COLUMNS 1000

#define MAX_SCROLL_RANGE 10000

#define LEFT_MARGIN 3

#define COL_SPACING 100

#define TAB_CAPTURE_CLASS -100
#define REVERSE_TAB_CAPTURE_CLASS -200


class CinchGrid {
private:
	
	int numColumns;
	GridColumn* columns[MAX_COLUMNS];
	GridDelegate* delegate;

	HWND hWnd;
	HINSTANCE hInst;

	int activeRow;// = -1;
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

	void addColumn(wchar_t * header, int width);

	/*Editing */
	void startEditing(int previous, int row);
	void scrollEditors(int offsetX, int offsetY);

	void SetScroll(HWND hWnd);

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
	


public:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK DetailWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);


	CinchGrid(HWND hWnd, HINSTANCE hInst);

	static HWND CreateCinchGrid(HWND parent);
};

