

class GridDelegate
{
public:
	virtual int totalRows()=0;
	virtual int totalColumns()=0;

	virtual int columnWidth(int column)=0;
	virtual int rowHeight() = 0;
	virtual wchar_t* headerContent(int)=0;
	virtual wchar_t* cellContent(int, int)=0;

	virtual bool stickyHeaders() = 0;

	virtual bool drawHorizontalGridlines() = 0;
	virtual bool drawVerticalGridlines() = 0;
	
	virtual bool rowSelection() = 0;

	virtual void setupEditorForCell(HWND editor, int row, int col) = 0;
	virtual bool allowEditing(int col) = 0;
	virtual HWND editorForColumn(int, HWND parent, HINSTANCE hInst) = 0;
	virtual void editingFinished(HWND editor, int row, int col) = 0;

	virtual bool allowNewRows() = 0;
	virtual void prepareNewRow(int row) = 0;

	virtual HFONT getFont()=0;
	virtual HFONT getEditFont() = 0;
};

#define MAX_ROWS 4000
#define TOTAL_COLS 3

class ReferenceDelegate : public GridDelegate {
private:
	int rowCount;
	wchar_t* data[MAX_ROWS][TOTAL_COLS];
public:
	ReferenceDelegate();
	int totalRows();
	int totalColumns();

	int columnWidth(int column);
	int rowHeight();

	wchar_t* headerContent(int);
	wchar_t* cellContent(int, int);

	bool stickyHeaders();
	
	bool rowSelection();

	bool drawHorizontalGridlines();
	bool drawVerticalGridlines();

	bool allowEditing(int);
	void setupEditorForCell(HWND editor, int row, int col);
	HWND editorForColumn(int, HWND parent, HINSTANCE hInst) ;
	void editingFinished(HWND editor, int row, int col);

	bool allowNewRows();
	void prepareNewRow(int row);

	HFONT getFont();
	HFONT getEditFont();
};
