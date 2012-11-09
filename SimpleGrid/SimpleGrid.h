#pragma once

#include "resource.h"
#include "stdafx.h"

#define MAX_COLUMNS 1000

#define MAX_SCROLL_RANGE 10000

int numColumns;
GridColumn* columns[MAX_COLUMNS];
GridDelegate* delegate;
