/*	Public domain	*/

#include "begin_code.h"

struct map_view;
struct map_tool;

struct map_insert_tool {
	struct map_tool tool;
	enum rg_snap_mode snap_mode;
	int replace_mode;
	int angle;
	struct map mTmp;
	struct map_view *mvTmp;
};

#include "close_code.h"
