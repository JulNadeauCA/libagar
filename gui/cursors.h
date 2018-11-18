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

__BEGIN_DECLS
int  AG_InitStockCursors(struct ag_driver *_Nonnull);
void AG_FreeCursors(struct ag_driver *_Nonnull);

AG_Cursor *_Nullable AG_CursorNew(void *_Nonnull, Uint,Uint,
                                  const Uint8 *_Nonnull,
				  const Uint8 *_Nonnull, int,int);

AG_Cursor *_Nullable AG_CursorFromXPM(void *_Nonnull, char *_Nonnull [_Nonnull],
                                      int,int);

void                 AG_CursorFree(void *_Nonnull, AG_Cursor *_Nonnull);

/* Initialize an AG_Cursor structure. */
static __inline__ void
AG_CursorInit(AG_Cursor *_Nonnull ac)
{
	ac->data = NULL;
	ac->mask = NULL;
	ac->w = 0;
	ac->h = 0;
	ac->xHot = 0;
	ac->yHot = 0;
	ac->p = NULL;
}

/* Return a pointer to a built-in cursor. */
static __inline__ AG_Cursor *_Nonnull
AG_GetStockCursor(void *_Nonnull obj, int name)
{
	AG_Driver *drv = AGDRIVER(obj);
	AG_Cursor *ac;
	int i = 0;

	AG_TAILQ_FOREACH(ac, &drv->cursors, cursors) {
		if (i++ == name)
			break;
	}
	if (ac == NULL) {
		AG_FatalError("AG_GetStockCursor");
	}
	return (ac);
}

/* Return a pointer to the active cursor. */
static __inline__ AG_Cursor *_Nullable
AG_GetActiveCursor(void *_Nonnull drv)
{
	return (AGDRIVER(drv)->activeCursor);
}

/* Show/hide the active cursor. */
static __inline__ int
AG_CursorIsVisible(void *_Nonnull drv)
{
	return AGDRIVER_CLASS(drv)->getCursorVisibility(drv);
}
static __inline__ void
AG_ShowCursor(void *_Nonnull drv)
{
	AGDRIVER_CLASS(drv)->setCursorVisibility(drv, 1);
}
static __inline__ void
AG_HideCursor(void *_Nonnull drv)
{
	AGDRIVER_CLASS(drv)->setCursorVisibility(drv, 0);
}
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_GUI_CURSORS_H_ */
