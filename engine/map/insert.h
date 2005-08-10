/*	$Csoft$	*/
/*	Public domain	*/

#include "begin_code.h"

struct tool;

struct insert_tool {
	struct tool tool;
	enum {
		INSERT_SRC_ARTWORK,		/* From artwork list */
		INSERT_SRC_COPYBUF		/* From copy/paste buffer */
	} source;
	enum gfx_snap_mode snap_mode;
	int replace_mode;
	int angle;
};

#include "close_code.h"
