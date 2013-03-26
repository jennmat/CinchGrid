
#include "stdafx.h"
#include <string.h>

ReferenceDelegate::ReferenceDelegate(){
	int i,j;
	rowCount = 200;
	columnCount = 1;

	for(i=0;i<MAX_ROWS;i++){
		for(j=0; j<MAX_COLUMNS; j++){
			data[i][j]=NULL;
		}
	}
}

int ReferenceDelegate::totalColumns(){
	return columnCount;
}

int ReferenceDelegate::totalRows(){
	return rowCount;
}

int ReferenceDelegate::columnWidth(int column){
	//if ( column == 0 ){
	//	return 25;
	//
	return 125;
}

wchar_t* ReferenceDelegate::headerContent(int col) {
	//if ( col == 0 ){
		return TEXT("");
	//} 
	///if ( col == 1 ){
		//return TEXT("Date");
	//}
	//return TEXT("Text");
}

bool ReferenceDelegate::stickyHeaders(){
	return true;
}

const wchar_t* ReferenceDelegate::cellContent(int row, int col) {
	
	//if( data[row][col] != NULL ){
	//	return data[row][col];
	//}
	
	//if ( col == 0 ){
		wchar_t* buffer = (wchar_t*)malloc(20*sizeof(wchar_t));
		_itow_s(row*col, buffer, 20, 10);

		return buffer;
//	}

	//if( col == 1 ){
	//		return TEXT("");
	//	}
	//return TEXT("Random Text");
	//return TEXT("");
}

HFONT ReferenceDelegate::getFont(){
	HFONT hFont=CreateFont(17,0,0,0,0,0,0,0,0,0,0,0,0,TEXT("MS Shell Dlg"));
	return hFont;
}

HFONT ReferenceDelegate::getEditFont(){
	HFONT hFont=CreateFont(18,0,0,0,0,0,0,0,0,0,0,0,0,TEXT("MS Shell Dlg"));
	return hFont;
}

bool ReferenceDelegate::drawHorizontalGridlines(){
	return true;
}

bool ReferenceDelegate::drawVerticalGridlines(){
	return true;
}

int ReferenceDelegate::rowHeight(){
	return 25;
}

bool ReferenceDelegate::rowSelection() {
	return true;
}

bool ReferenceDelegate::allowEditing(int col){
	if( col == 0 ) return false;
	return false;
}

void ReferenceDelegate::willLoseFocus(){
}

bool ReferenceDelegate::allowHeaderTitleEditing(int col){
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
	//if ( col == 1 ){
		//return CreateWindowEx(0, DATETIMEPICK_CLASS, TEXT("DateTime"), WS_CHILD|WS_VISIBLE|WS_TABSTOP,
		//	0, 0, 0, 0, parent, NULL, hInst, NULL);

	//} else if ( col == 2 ) {

		//return CreateComboBox(parent,hInst);
	//}
	//if ( col == 1 ){
		//return CreateWindowEx(0, L"BUTTON", L"", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|WS_TABSTOP,
		//0, 0, 0, 0, parent, NULL, hInst, NULL);

	//}

	return CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		0, 0, 0, 0, parent, NULL, hInst, NULL);
	
}

void ReferenceDelegate::editingFinished(HWND editor, int row, int col)
{
	data[row][col] = (wchar_t *)malloc(100*sizeof(TCHAR));
	GetWindowText(editor, data[row][col], 100);

}

bool ReferenceDelegate::allowNewRows() {
	return false;
}

bool ReferenceDelegate::allowNewColumns() {
	return false;
}

void ReferenceDelegate::prepareNewRow(int row){
	rowCount++;
	data[row][0] = NULL;
	data[row][1] = TEXT("");
	data[row][2] = TEXT("");
}

void ReferenceDelegate::setupEditorForCell(HWND editor, int row, int col){
	SendMessage(editor, WM_SETTEXT, (WPARAM)0, (LPARAM)this->cellContent(row, col));
}

void ReferenceDelegate::headerContextClick(HWND grid, int x, int y){
	MessageBox(NULL, L"Header right click", L"Cinch Grid", MB_OK);
}