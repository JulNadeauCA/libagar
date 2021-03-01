/*	Public domain	*/

#include <agar/map/begin.h>

struct map_view;
struct map_tool;

typedef struct map_insert_tool {
	struct map_tool tool;
	enum rg_snap_mode snap_mode;
	int replace_mode;
	int angle;
	Uint32 _pad;
	struct map mTmp;
	struct map_view *_Nullable mvTmp;
} MAP_InsertTool;

#include <agar/map/close.h>
