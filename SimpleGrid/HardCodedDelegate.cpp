
#include "stdafx.h"
#include <string.h>


int HardCodedDelegate::totalColumns(){
	return 2;
}

int HardCodedDelegate::totalRows(){
	return 100;
}

int HardCodedDelegate::columnWidth(int column){
	return 200;
}

wchar_t* HardCodedDelegate::headerContent(int col) {
	return TEXT("Header");
}

bool HardCodedDelegate::stickyHeaders(){
	return true;
}

wchar_t* HardCodedDelegate::cellContent(int row, int col) {
	wchar_t* buffer = (wchar_t*)malloc(20*sizeof(wchar_t));
	_itow_s((row+1)*(col+1), buffer, 20, 10);

	return buffer;
	//if( col == 0 ){
//		return TEXT("10/22/2012");
//	} 
	//if ( col == 1 ){
//		return TEXT("Random Text");
//	}
}

HFONT HardCodedDelegate::getFont(){
	HFONT hFont=CreateFont(18,0,0,0,0,0,0,0,0,0,0,0,0,TEXT("MS Shell Dlg"));
	return hFont;
}
