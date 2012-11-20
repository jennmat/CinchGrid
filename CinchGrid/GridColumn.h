

class GridColumn {
private:
	wchar_t * header;
	int width;
	HWND editor;
public:
	GridColumn(wchar_t*, int);
	int getWidth();
	void setWidth(int);
	wchar_t* getHeader();
	HWND getEditor();
	void setEditor(HWND);
};