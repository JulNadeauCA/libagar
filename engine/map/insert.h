/*	$Csoft: insert.h,v 1.1 2005/08/10 07:03:11 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct mapview;
struct tool;

struct insert_tool {
	struct tool tool;
	enum gfx_snap_mode snap_mode;
	int replace_mode;
	int angle;
	struct map mTmp;
	struct mapview *mvTmp;
};

#include "close_code.h"
