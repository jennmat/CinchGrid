
#include "stdafx.h"
#include <string>
using namespace std;

class GridColumn {
private:
	wstring header;
	int width;
	HWND editor;
public:
	GridColumn(wstring, int);
	~GridColumn();
	int getWidth();
	void setWidth(int);
	wstring getHeader();
	void setHeader(wchar_t*);
	HWND getEditor();
	void setEditor(HWND);
};