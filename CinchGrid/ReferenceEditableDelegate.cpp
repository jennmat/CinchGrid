
#include "stdafx.h"
#include <string.h>
#include <stdio.h>

ReferenceEditableDelegate::ReferenceEditableDelegate(){
	rowCount = 3;
}

ReferenceEditableDelegate::~ReferenceEditableDelegate(){
}

int ReferenceEditableDelegate::totalColumns(){
	return 4;
}

int ReferenceEditableDelegate::totalRows(){
	return rowCount;
}

int ReferenceEditableDelegate::columnWidth(int column){
	return 85;
}

int ReferenceEditableDelegate::rowHeight(){
	return 22;
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
				data[i][col] = new wchar_t[200];
				memset(data[i][col], 0, sizeof(wchar_t)*200);
				wcscpy_s(data[i][col], 2, L"");
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

void ReferenceEditableDelegate::editingFinished(int row, int col, wchar_t*** data)
{
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
	return false;
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