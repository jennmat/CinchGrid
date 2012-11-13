
#include "stdafx.h"
#include <string.h>


int ReferenceDelegate::totalColumns(){
	return 4;
}

int ReferenceDelegate::totalRows(){
	return 150;
}

int ReferenceDelegate::columnWidth(int column){
	if ( column == 0 ){
		return 25;
	}
	return 200;
}

wchar_t* ReferenceDelegate::headerContent(int col) {
	if ( col == 0 ){
		return TEXT("");
	} 
	if ( col == 1 ){
		return TEXT("Date");
	}
	return TEXT("Text");
}

bool ReferenceDelegate::stickyHeaders(){
	return true;
}

wchar_t* ReferenceDelegate::cellContent(int row, int col) {
	if ( col == 0 ){
		wchar_t* buffer = (wchar_t*)malloc(20*sizeof(wchar_t));
		_itow_s((row+1)*(col+1), buffer, 20, 10);

		return buffer;
	}
	
	if( col == 1 ){
		return TEXT("10/22/2012");
	}
	return TEXT("Random Text");
}

HFONT ReferenceDelegate::getFont(){
	HFONT hFont=CreateFont(18,0,0,0,0,0,0,0,0,0,0,0,0,TEXT("MS Shell Dlg"));
	return hFont;
}

HFONT ReferenceDelegate::getEditFont(){
	HFONT hFont=CreateFont(18,0,0,0,0,0,0,0,0,0,0,0,0,TEXT("MS Shell Dlg"));
	return hFont;
}

bool ReferenceDelegate::drawHorizontalGridlines(){
	return false;
}

bool ReferenceDelegate::drawVerticalGridlines(){
	return true;
}

int ReferenceDelegate::rowHeight(){
	return 32;
}

bool ReferenceDelegate::rowSelection() {
	return false;
}

bool ReferenceDelegate::allowEditing(int col){
	if( col == 0 ) return false;
	return true;
}

HWND CreateComboBox(HWND parent, HINSTANCE hInst){
	HWND cb = CreateWindowEx(WS_EX_CLIENTEDGE, L"COMBOBOX", L"", WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP,
		0, 0, 40, 32, parent, NULL, hInst, NULL);


	TCHAR Planets[9][10] =  
	{
		TEXT("Mercury"), TEXT("Venus"), TEXT("Terra"), TEXT("Mars"), 
		TEXT("Jupiter"), TEXT("Saturn"), TEXT("Uranus"), TEXT("Neptune"), 
		TEXT("Pluto??") 
	};
       
	TCHAR A[16]; 
	int  k = 0; 

	memset(&A,0,sizeof(A));       
	for (k = 0; k <= 8; k += 1)
	{
		wcscpy_s(A, sizeof(A)/sizeof(TCHAR),  (TCHAR*)Planets[k]);

		// Add string to combobox.
		SendMessage(cb,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) A); 
	}
  
	// Send the CB_SETCURSEL message to display an initial item 
	//  in the selection field  
	SendMessage(cb, CB_SETCURSEL, (WPARAM)2, (LPARAM)0);
	return cb;
}

HWND ReferenceDelegate::editorForColumn(int col, HWND parent, HINSTANCE hInst){
	if ( col == 1 ){
		return CreateWindowEx(0, DATETIMEPICK_CLASS, TEXT("DateTime"), WS_CHILD|WS_VISIBLE|WS_TABSTOP,
			0, 0, 0, 0, parent, NULL, hInst, NULL);

	} else if ( col == 2 ) {

		return CreateComboBox(parent,hInst);
	}

	return CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		0, 0, 0, 0, parent, NULL, hInst, NULL);

}

void ReferenceDelegate::setupEditorForCell(HWND editor, int row, int col){
	SendMessage(editor, WM_SETTEXT, (WPARAM)0, (LPARAM)this->cellContent(row, col));
}

