
#include "stdafx.h"
#include <string.h>


int HardCodedDelegate::totalColumns(){
	return 3;
}

int HardCodedDelegate::totalRows(){
	return 100;
}

int HardCodedDelegate::columnWidth(int column){
	if ( column == 0 ){
		return 25;
	}
	return 200;
}

wchar_t* HardCodedDelegate::headerContent(int col) {
	if ( col == 0 ){
		return TEXT("");
	} 
	if ( col == 1 ){
		return TEXT("Date");
	}
	return TEXT("Text");
}

bool HardCodedDelegate::stickyHeaders(){
	return true;
}

wchar_t* HardCodedDelegate::cellContent(int row, int col) {
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

HFONT HardCodedDelegate::getFont(){
	HFONT hFont=CreateFont(32,0,0,0,0,0,0,0,0,0,0,0,0,TEXT("MS Shell Dlg"));
	return hFont;
}

bool HardCodedDelegate::drawHorizontalGridlines(){
	return true;
}

bool HardCodedDelegate::drawVerticalGridlines(){
	return true;
}

int HardCodedDelegate::rowHeight(){
	return 60;
}

