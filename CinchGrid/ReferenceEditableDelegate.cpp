
#include "stdafx.h"
#include <string.h>
#include <stdio.h>

ReferenceEditableDelegate::ReferenceEditableDelegate(){
	int i,j;
	rowCount = 0;

	for(i=0;i<MAX_ROWS;i++){
		for(j=0; j<2; j++){
			data[i][j]=NULL;
		}
	}
}

int ReferenceEditableDelegate::totalColumns(){
	return 2;
}

int ReferenceEditableDelegate::totalRows(){
	return rowCount;
}

int ReferenceEditableDelegate::columnWidth(int column){
	return 150;
}

int ReferenceEditableDelegate::rowHeight(){
	return 25;
}

wchar_t* ReferenceEditableDelegate::headerContent(int col) {
	return TEXT("");
}

bool ReferenceEditableDelegate::stickyHeaders(){
	return false;
}

const wchar_t* ReferenceEditableDelegate::cellContent(int row, int col) {
	if( data[row][col] != NULL ){
		return data[row][col];
	}
	
	return TEXT("");

}

HFONT ReferenceEditableDelegate::getFont(){
	HFONT hFont=CreateFont(17,0,0,0,0,0,0,0,0,0,0,0,0,TEXT("MS Shell Dlg"));
	return hFont;
}

HFONT ReferenceEditableDelegate::getEditFont(){
	HFONT hFont=CreateFont(18,0,0,0,0,0,0,0,0,0,0,0,0,TEXT("MS Shell Dlg"));
	return hFont;
}

bool ReferenceEditableDelegate::drawHorizontalGridlines(){
	return true;
}

bool ReferenceEditableDelegate::drawVerticalGridlines(){
	return true;
}


bool ReferenceEditableDelegate::rowSelection() {
	return false;
}

bool ReferenceEditableDelegate::allowEditing(int col){
	return true;
}

HWND ReferenceEditableDelegate::editorForColumn(int col, HWND parent, HINSTANCE hInst){
	if( col == 0 ){
		return CreateWindowEx(0, DATETIMEPICK_CLASS, TEXT("DateTime"), WS_CHILD|WS_VISIBLE|WS_TABSTOP,
			0, 0, 0, 0, parent, NULL, hInst, NULL);
	}

	return CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		0, 0, 0, 0, parent, NULL, hInst, NULL);
	
}

void ReferenceEditableDelegate::editingFinished(HWND editor, int row, int col)
{
	data[row][col] = (wchar_t *)malloc(100*sizeof(TCHAR));
	GetWindowText(editor, data[row][col], 100);
	
}

void ReferenceEditableDelegate::willLoseFocus(){
}

bool ReferenceEditableDelegate::allowNewRows() {
	return true;
}

void ReferenceEditableDelegate::prepareNewRow(int row){
	rowCount++;
}

bool ReferenceEditableDelegate::allowHeaderTitleEditing(int col) {
	return true;
}

bool ReferenceEditableDelegate::allowNewColumns() {
	return true;
}

void ReferenceEditableDelegate::setupEditorForCell(HWND editor, int row, int col){
	if ( col == 0 ){
		const wchar_t* timeStr = this->cellContent(row, col);
		if (timeStr == NULL ) return;

		SYSTEMTIME time;
		int month, day, year;
		GetLocalTime(&time);

		if ( wcslen(timeStr) > 0 ){
			swscanf_s(timeStr, TEXT("%d/%d/%d"), &month, &day, &year);
			time.wMonth = month;
			time.wDay = day;
			time.wYear = year;
		}

		DateTime_SetSystemtime(editor, GDT_VALID, &time);
		return;
	}
	SendMessage(editor, WM_SETTEXT, (WPARAM)0, (LPARAM)this->cellContent(row, col));
}

void ReferenceEditableDelegate::headerContextClick(HWND hwnd, int x, int y){

}

void ReferenceEditableDelegate::willReloadData(){
}

void ReferenceEditableDelegate::didReloadData(){
}