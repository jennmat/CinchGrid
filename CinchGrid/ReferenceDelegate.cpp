
#include "stdafx.h"
#include <string.h>
#include <sstream>

#define NOSORT 0
#define ASCENDING 1
#define DESCENDING 2

ReferenceDelegate::ReferenceDelegate(){
	rowCount = 100;
	columnCount = 2;
	for(int i=0;i<columnCount;i++){
		sorts[i] = NOSORT;
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
	//return CINCH_GRID_MAXIMIZE_WIDTH;
	//return column == 0 ? 250 : CINCH_GRID_MAXIMIZE_WIDTH;
	return 250;
}


void ReferenceDelegate::headerContent(int col, wstring &content) {
	//if ( col == 0 ){
	//} 
	///if ( col == 1 ){
		//return TEXT("Date");
	//}
	//return TEXT("Text");
	if ( col == 0 )
		content = L"Numbers";
	else 
		content = L"Letters";
}

bool ReferenceDelegate::stickyHeaders(){
	return true;
}




int ReferenceDelegate::LoadSegment(int start_row, int len, wchar_t*** data){

	bool ascending = false;
	bool descending = false;
	if ( sorts[1] == ASCENDING || sorts[0] == ASCENDING) {
		ascending = true;
	} else if ( sorts[0] == DESCENDING || sorts[1] == DESCENDING) {
		descending = true;
	}

	int row = start_row;
	for(int i=0; i<len; i++){
		for(int col=0; col<columnCount; col++){
			wstringstream s;
			if ( col == 0 ){
				if ( descending == true ){
					s << rowCount - 1 - row;
				} else {
					s << row;
				}
			} else {
				if ( descending == true ){
					s << (char)((rowCount-1-row)%26+65);
				} else {
					s << (char)(row%26+65);
				}
			}
			
			wstring str = s.str();
			const wchar_t* t = str.c_str();
			data[i][col] = new wchar_t[20];
			wcscpy_s(data[i][col], 20, t);
		}
		row++;
	}
	return len;
}

void ReferenceDelegate::CleanupSegment(int len, wchar_t*** data){
	for(int i=0; i<len; i++){
		for(int col=0; col<columnCount; col++){
			delete data[i][col];
		}
	}
}

HFONT ReferenceDelegate::getFont(){
	HFONT hFont=CreateFont(-12,0,0,0,400,0,0,0,1,0,0,0,0,TEXT("Segoe UI"));
	return hFont;
}

HFONT ReferenceDelegate::getEditFont(){
	HFONT hFont=CreateFont(-12,0,0,0,400,0,0,0,1,0,0,0,0,TEXT("Segoe UI"));
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

void ReferenceDelegate::sortAscending(int col){
	sorts[col] = ASCENDING;
}

void ReferenceDelegate::sortDescending(int col){
	sorts[col] = DESCENDING;
}

void ReferenceDelegate::sortOff(int col){
	sorts[col] = NOSORT;
}

void ReferenceDelegate::willLoseFocus(){
}

bool ReferenceDelegate::allowHeaderTitleEditing(int col){
	return false;
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

void ReferenceDelegate::editingFinished(HWND editor, int row, int col, wchar_t*** data)
{
	data[row][col] = (wchar_t *)malloc(100*sizeof(TCHAR));
	GetWindowText(editor, data[row][col], 100);

}

bool ReferenceDelegate::allowSorting(int col){
	return true;
}

bool ReferenceDelegate::allowNewRows() {
	return false;
}

bool ReferenceDelegate::allowNewColumns() {
	return false;
}

void ReferenceDelegate::prepareNewRow(int row){
	rowCount++;
}

void ReferenceDelegate::setupEditorForCell(HWND editor, int row, int col, wchar_t***){
}

void ReferenceDelegate::headerContextClick(HWND grid, int x, int y){
	MessageBox(NULL, L"Header right click", L"Cinch Grid", MB_OK);
}

void ReferenceDelegate::willReloadData(){
}

void ReferenceDelegate::didReloadData(){
}

void ReferenceDelegate::didSelectRow(int row){
}

void ReferenceDelegate::setGrid(CinchGrid* grid){
}

void ReferenceDelegate::didChangeColumnWidth(int col, int newWidth){
}