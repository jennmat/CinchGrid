

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

	virtual HFONT getFont()=0;
};


class ReferenceDelegate : public GridDelegate {
public:
	int totalRows();
	int totalColumns();

	int columnWidth(int column);
	int rowHeight();

	wchar_t* headerContent(int);
	wchar_t* cellContent(int, int);

	bool stickyHeaders();

	bool drawHorizontalGridlines();
	bool drawVerticalGridlines();

	HFONT getFont();
};
