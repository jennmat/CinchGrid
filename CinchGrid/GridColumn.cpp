

#include "stdafx.h"



GridColumn::GridColumn(wchar_t * header, int width){
	this->header = header;
	this->width = width;
	this->editor = NULL;
}

GridColumn::~GridColumn(){
	delete header;
}

wchar_t * GridColumn::getHeader(){
	return this->header;
}

void GridColumn::setHeader(wchar_t* text){
	this->header = text;
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