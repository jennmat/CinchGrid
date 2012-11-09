

class GridColumn {
private:
	wchar_t * header;
	int width;
public:
	GridColumn(wchar_t*, int);
	int getWidth();
	void setWidth(int);
	wchar_t* getHeader();
};