

#include "stdafx.h"



GridColumn::GridColumn(wchar_t * header, int width){
	this->header = header;
	this->width = width;
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

