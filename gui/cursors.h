/*	Public domain	*/

#ifndef _AGAR_GUI_CURSORS_H_
#define _AGAR_GUI_CURSORS_H_

#include <agar/gui/drv.h>

#include <agar/gui/begin.h>

#define AG_CURSOR_MAX_W 32
#define AG_CURSOR_MAX_H 32

/* Built-in cursors */
enum {
	AG_DEFAULT_CURSOR,
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

struct ag_driver;

typedef struct ag_cursor {
	Uint w, h;			/* Cursor dimensions */
	Uint8 *_Nonnull data;		/* Raw bitmap data */
	Uint8 *_Nonnull mask;		/* Transparency mask */
	int  xHot, yHot;		/* Hotspot */
	void *_Nullable p;		/* Driver data */
	AG_TAILQ_ENTRY(ag_cursor) cursors;
} AG_Cursor;

#define AGCURSOR(p) ((AG_Cursor *)(p))

__BEGIN_DECLS
void AG_InitStockCursors(struct ag_driver *_Nonnull);
void AG_FreeCursors(struct ag_driver *_Nonnull);

AG_Cursor *_Nullable AG_CursorNew(void *_Nonnull, Uint,Uint,
                                  const Uint8 *_Nonnull,
				  const Uint8 *_Nonnull, int,int);

AG_Cursor *_Nullable AG_CursorFromXPM(void *_Nonnull, char *_Nonnull [_Nonnull],
                                      int,int);

void AG_CursorInit(AG_Cursor *_Nonnull);
void AG_CursorFree(void *_Nonnull, AG_Cursor *_Nonnull);

AG_Cursor *_Nonnull  AG_GetStockCursor(void *_Nonnull, int);
AG_Cursor *_Nullable AG_GetActiveCursor(void *_Nonnull);

int  AG_CursorIsVisible(void *_Nonnull);
void AG_ShowCursor(void *_Nonnull);
void AG_HideCursor(void *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_GUI_CURSORS_H_ */
