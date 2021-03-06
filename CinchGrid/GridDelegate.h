
#include "stdafx.h"
#include <string>
using namespace std;


class CinchGrid;

#define CINCH_GRID_MAXIMIZE_WIDTH -1

class GridDelegate
{
public:
	virtual ~GridDelegate() {};
	virtual int totalRows()=0;
	virtual int totalColumns()=0;

	virtual int columnWidth(int column)=0;
	virtual int rowHeight() = 0;
	virtual void headerContent(int, wstring &)=0;

	virtual void LoadSegment(int start_row, int len, wchar_t*** data, int*, int*)=0;
	virtual void CleanupSegment(int rowCount, int colCount, wchar_t*** data)=0;

	virtual bool stickyHeaders() = 0;

	virtual bool drawHorizontalGridlines() = 0;
	virtual bool drawVerticalGridlines() = 0;
	
	virtual bool rowSelection() = 0;

	virtual void willReloadData() = 0;
	virtual void didReloadData() = 0;

	virtual void didChangeColumnWidth(int, int)=0;

	virtual void didSelectRow(int) = 0;

	virtual bool allowEditing(int col) = 0;
	virtual bool allowHeaderTitleEditing(int col)=0;
	virtual void editingFinished(int row, int col, wchar_t*** data) = 0;
	virtual void willLoseFocus()=0;

	virtual bool allowSorting(int col)=0;
	virtual void sortAscending(int col)=0;
	virtual void sortDescending(int col)=0;
	virtual void sortOff(int col)=0;

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
	int sorts[MAX_COLUMNS];
public:
	ReferenceDelegate();
	int totalRows();
	int totalColumns();

	int columnWidth(int column);
	int rowHeight();

	void headerContent(int, wstring &);

	void LoadSegment(int start_row, int len, wchar_t*** data, int*, int*);
	void CleanupSegment(int rowCount, int colCount, wchar_t*** data);

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
	void editingFinished(int row, int col, wchar_t*** data);
	void willLoseFocus();

	bool allowSorting(int col);
	void sortAscending(int col);
	void sortDescending(int col);
	void sortOff(int col);

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
public:
	ReferenceEditableDelegate();
	~ReferenceEditableDelegate();
	int totalRows();
	int totalColumns();

	int columnWidth(int column);
	int rowHeight();

	void headerContent(int, wstring &);

	void LoadSegment(int start_row, int len, wchar_t*** data, int*, int*);
	void CleanupSegment(int rowCount, int colCount, wchar_t*** data);

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
	void editingFinished(int row, int col, wchar_t*** data);
	void willLoseFocus();

	bool allowNewRows();
	bool allowNewColumns();
	void prepareNewRow(int row);

	void headerContextClick(HWND grid, int x, int y);

	HFONT getFont();
	HFONT getEditFont();

	bool allowSorting(int col);
	void sortAscending(int col);
	void sortDescending(int col);
	void sortOff(int col);

	void setGrid(CinchGrid* grid);
};
