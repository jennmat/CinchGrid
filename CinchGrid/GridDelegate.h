
#include "stdafx.h"

class CinchGrid;

class GridDelegate
{
public:
	virtual int totalRows()=0;
	virtual int totalColumns()=0;

	virtual int columnWidth(int column)=0;
	virtual int rowHeight() = 0;
	virtual int headerContentLength(int)=0;
	virtual void headerContent(int, wchar_t*)=0;
	virtual const wchar_t* cellContent(int, int)=0;

	virtual bool stickyHeaders() = 0;

	virtual bool drawHorizontalGridlines() = 0;
	virtual bool drawVerticalGridlines() = 0;
	
	virtual bool rowSelection() = 0;

	virtual void willReloadData() = 0;
	virtual void didReloadData() = 0;

	virtual void didChangeColumnWidth(int, int)=0;

	virtual void didSelectRow(int) = 0;

	virtual void setupEditorForCell(HWND editor, int row, int col) = 0;
	virtual bool allowEditing(int col) = 0;
	virtual bool allowHeaderTitleEditing(int col)=0;
	virtual HWND editorForColumn(int, HWND parent, HINSTANCE hInst) = 0;
	virtual void editingFinished(HWND editor, int row, int col) = 0;
	virtual void willLoseFocus()=0;

	virtual bool allowNewRows() = 0;
	virtual bool allowNewColumns() = 0;
	virtual void prepareNewRow(int row) = 0;

	virtual void headerContextClick(HWND hwnd, int x, int y) = 0;

	virtual HFONT getFont()=0;
	virtual HFONT getEditFont() = 0;

	virtual void setGrid(CinchGrid* grid) = 0;
};

#define MAX_ROWS 4000
#define MAX_COLUMNS 300

class ReferenceDelegate : public GridDelegate {
private:
	int rowCount;
	int columnCount;
	wchar_t* data[MAX_ROWS][MAX_COLUMNS];
public:
	ReferenceDelegate();
	int totalRows();
	int totalColumns();

	int columnWidth(int column);
	int rowHeight();

	wchar_t* headerContent(int);
	const wchar_t* cellContent(int, int);

	bool stickyHeaders();
	
	bool rowSelection();

	bool drawHorizontalGridlines();
	bool drawVerticalGridlines();

	void willReloadData();
	void didReloadData();

	void didChangeColumnWidth(int, int);

	void didSelectRow(int);

	bool allowEditing(int);
	bool allowHeaderTitleEditing(int);
	void setupEditorForCell(HWND editor, int row, int col);
	HWND editorForColumn(int, HWND parent, HINSTANCE hInst) ;
	void editingFinished(HWND editor, int row, int col);
	void willLoseFocus();

	bool allowNewRows();
	bool allowNewColumns();
	void prepareNewRow(int row);

	void headerContextClick(HWND grid, int x, int y);

	HFONT getFont();
	HFONT getEditFont();

	void setGrid(CinchGrid* grid);
};


class ReferenceEditableDelegate : public GridDelegate {
private:
	int rowCount;
	int columnCount;
	wchar_t* data[MAX_ROWS][MAX_COLUMNS];
public:
	ReferenceEditableDelegate();
	int totalRows();
	int totalColumns();

	int columnWidth(int column);
	int rowHeight();

	wchar_t* headerContent(int);
	const wchar_t* cellContent(int, int);

	bool stickyHeaders();
	
	bool rowSelection();

	void willReloadData();
	void didReloadData();

	void didChangeColumnWidth(int, int);

	void didSelectRow(int row);

	bool drawHorizontalGridlines();
	bool drawVerticalGridlines();

	bool allowEditing(int);
	bool allowHeaderTitleEditing(int);
	void setupEditorForCell(HWND editor, int row, int col);
	HWND editorForColumn(int, HWND parent, HINSTANCE hInst) ;
	void editingFinished(HWND editor, int row, int col);
	void willLoseFocus();

	bool allowNewRows();
	bool allowNewColumns();
	void prepareNewRow(int row);

	void headerContextClick(HWND grid, int x, int y);

	HFONT getFont();
	HFONT getEditFont();

	void setGrid(CinchGrid* grid);
};
