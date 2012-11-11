
#include "stdafx.h"
#include <string.h>


int ReferenceDelegate::totalColumns(){
	return 30;
}

int ReferenceDelegate::totalRows(){
	return 15;
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

bool ReferenceDelegate::drawHorizontalGridlines(){
	return true;
}

bool ReferenceDelegate::drawVerticalGridlines(){
	return true;
}

int ReferenceDelegate::rowHeight(){
	return 32;
}

