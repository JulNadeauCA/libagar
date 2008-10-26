/*	Public domain	*/

#ifndef _AGAR_WIDGET_CURSORS_H_
#define _AGAR_WIDGET_CURSORS_H_
#include <agar/gui/begin.h>

enum {
	AG_FILL_CURSOR,
	AG_ERASE_CURSOR,
	AG_PICK_CURSOR,
	AG_HRESIZE_CURSOR,
	AG_VRESIZE_CURSOR,
	AG_LRDIAG_CURSOR,
	AG_LLDIAG_CURSOR,
	AG_TEXT_CURSOR,
	AG_LAST_CURSOR
};

__BEGIN_DECLS
extern SDL_Cursor *agCursors[];
extern SDL_Cursor *agDefaultCursor;

void AG_CursorsInit(void);
void AG_CursorsDestroy(void);
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_WIDGET_CURSORS_H_ */
