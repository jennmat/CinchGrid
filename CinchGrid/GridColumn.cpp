

#include "stdafx.h"



GridColumn::GridColumn(wchar_t * header, int width){
	this->header = header;
	this->width = width;
	this->editor = NULL;
}

wchar_t * GridColumn::getHeader(){
	return this->header;
}

int GridColumn::getWidth(){
	return width;
}

void GridColumn::setWidth(int width){
	this->width = width;
}

HWND GridColumn::getEditor(){
	return this->editor;
}

void GridColumn::setEditor(HWND hwnd){
	this->editor = hwnd;
}