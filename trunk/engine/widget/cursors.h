/*	$Csoft: cursors.h,v 1.3 2005/03/03 10:59:26 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CURSORS_H_
#define _AGAR_WIDGET_CURSORS_H_
#include "begin_code.h"

enum {
	AG_FILL_CURSOR,
	AG_ERASE_CURSOR,
	AG_PICK_CURSOR,
	AG_LAST_CURSOR
};

extern SDL_Cursor *agCursors[];

__BEGIN_DECLS
void AG_CursorsInit(void);
void AG_CursorsDestroy(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_CURSORS_H_ */
