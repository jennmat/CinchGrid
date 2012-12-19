

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
	void setHeader(wchar_t*);
	HWND getEditor();
	void setEditor(HWND);
};