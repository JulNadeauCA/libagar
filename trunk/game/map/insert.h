/*	$Csoft: insert.h,v 1.2 2005/08/27 04:34:05 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct ag_mapview;
struct ag_maptool;

struct ag_map_insert_tool {
	struct ag_maptool tool;
	enum ag_gfx_snap_mode snap_mode;
	int replace_mode;
	int angle;
	struct ag_map mTmp;
	struct ag_mapview *mvTmp;
};

#include "close_code.h"
