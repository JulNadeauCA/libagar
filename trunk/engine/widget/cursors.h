/*	$Csoft: cursors.h,v 1.4 2005/09/27 00:25:22 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CURSORS_H_
#define _AGAR_WIDGET_CURSORS_H_
#include "begin_code.h"

enum {
	AG_FILL_CURSOR,
	AG_ERASE_CURSOR,
	AG_PICK_CURSOR,
	AG_HRESIZE_CURSOR,
	AG_VRESIZE_CURSOR,
	AG_LRDIAG_CURSOR,
	AG_LLDIAG_CURSOR,
	AG_LAST_CURSOR
};

extern SDL_Cursor *agCursors[];

__BEGIN_DECLS
void AG_CursorsInit(void);
void AG_CursorsDestroy(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_CURSORS_H_ */
