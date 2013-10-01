
#include "stdafx.h"
#include <string.h>
#include <stdio.h>

ReferenceEditableDelegate::ReferenceEditableDelegate(){
	rowCount = 25;
}

ReferenceEditableDelegate::~ReferenceEditableDelegate(){
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

void ReferenceEditableDelegate::headerContent(int col, wstring& content) {
	content = L"";
}

bool ReferenceEditableDelegate::stickyHeaders(){
	return false;
}


void ReferenceEditableDelegate::LoadSegment(int start_row, int len, wchar_t*** data, int* rows_loaded, int* cols_loaded){
	int row = start_row;
	for(int i=0; i<len; i++){
		for(int col=0; col<totalColumns(); col++){	
			if ( data[i][col] == nullptr ){
				data[i][col] = new wchar_t[2];
				wcscpy_s(data[i][col], 2, L"a");
			}
		}
		row++;
	}
	*rows_loaded = len;
	*cols_loaded = totalColumns();
}

void ReferenceEditableDelegate::CleanupSegment(int rowCount, int columnCount, wchar_t*** data){
	for(int i=0; i<rowCount; i++){
		for(int col=0; col<columnCount; col++){
			delete data[i][col];
		}
	}
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

void ReferenceEditableDelegate::editingFinished(HWND editor, int row, int col, wchar_t*** data)
{
	delete data[row][col];
	int len = GetWindowTextLength(editor) + sizeof(wchar_t);
	data[row][col] = new wchar_t[len];
	GetWindowText(editor, data[row][col], len);
	
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

void ReferenceEditableDelegate::setupEditorForCell(HWND editor, int row, int col, wchar_t*** data){
	if ( col == 0 ){
		const wchar_t* timeStr = data[row][col];
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
	SendMessage(editor, WM_SETTEXT, (WPARAM)0, (LPARAM)data[row][col]);
}

void ReferenceEditableDelegate::headerContextClick(HWND hwnd, int x, int y){

}

void ReferenceEditableDelegate::willReloadData(){
}

void ReferenceEditableDelegate::didReloadData(){
}

void ReferenceEditableDelegate::didSelectRow(int row){
}

void ReferenceEditableDelegate::setGrid(CinchGrid* grid){
}

void ReferenceEditableDelegate::didChangeColumnWidth(int col, int newWidth){
}

bool ReferenceEditableDelegate::allowSorting(int col){
	return false;
}

void ReferenceEditableDelegate::sortAscending(int col){
}

void ReferenceEditableDelegate::sortDescending(int col){
}

void ReferenceEditableDelegate::sortOff(int col){
}