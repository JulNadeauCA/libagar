/*	$Csoft: cursors.h,v 1.2 2005/02/27 06:52:02 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CURSORS_H_
#define _AGAR_WIDGET_CURSORS_H_
#include "begin_code.h"

enum {
	FILL_CURSOR,
	ERASE_CURSOR,
	PICK_CURSOR,
	LAST_CURSOR
};

extern SDL_Cursor *cursors[];

__BEGIN_DECLS
void cursors_init(void);
void cursors_destroy(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_CURSORS_H_ */
