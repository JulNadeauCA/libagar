/*	Public domain	*/

#ifndef _AGAR_GUI_EDITABLE_H_
#define _AGAR_GUI_EDITABLE_H_

#include <agar/gui/widget.h>
#include <agar/gui/text.h>

#include <agar/gui/begin.h>

struct ag_popup_menu;

/* Working UCS-4 text buffer for internal use */
typedef struct ag_editable_buffer {
	AG_Variable *var;		/* Variable binding (if any) */
	Uint32 *s;			/* String buffer (UCS-4 encoding) */
	size_t len;			/* String length (chars) */
	size_t maxLen;			/* Available buffer size (bytes) */
	int reallocable;		/* Buffer can be realloc'd */
} AG_EditableBuffer;

/* Internal clipboard for copy/paste and kill/yank */
typedef struct ag_editable_clipboard {
	AG_Mutex lock;
	char encoding[32];		/* Character set encoding */
	Uint32 *s;			/* UCS-4 buffer */
	size_t len;			/* Length in characters */
} AG_EditableClipboard;

typedef struct ag_editable {
	struct ag_widget wid;
	
	Uint flags;
#define AG_EDITABLE_HFILL         0x000001
#define AG_EDITABLE_VFILL         0x000002
#define AG_EDITABLE_EXPAND        (AG_EDITABLE_HFILL|AG_EDITABLE_VFILL)
#define AG_EDITABLE_MULTILINE     0x000004 /* Multiline edition */
#define AG_EDITABLE_BLINK_ON      0x000008 /* Cursor blink state (internal) */
#define AG_EDITABLE_PASSWORD      0x000010 /* Password (hidden) input */
#define AG_EDITABLE_ABANDON_FOCUS 0x000020 /* Abandon focus on return */
#define AG_EDITABLE_INT_ONLY      0x000040 /* Accepts only int input */
#define AG_EDITABLE_FLT_ONLY      0x000080 /* Accepts only float input */
#define AG_EDITABLE_CATCH_TAB     0x000100 /* Process tab key input */
#define AG_EDITABLE_CURSOR_MOVING 0x000200 /* Cursor is being moved */
#define AG_EDITABLE_KEEPVISCURSOR 0x000800 /* Try to keep cursor visible */
#define AG_EDITABLE_MARKPREF      0x002000 /* Mark current cursor position */
#define AG_EDITABLE_EXCL          0x004000 /* Exclusive access to buffer */
#define AG_EDITABLE_NOEMACS       0x008000 /* Disable emacs-style fn keys */
#define AG_EDITABLE_NOLATIN1      0x020000 /* Disable LATIN-1 combinations */
#define AG_EDITABLE_WORDWRAP      0x040000 /* Word wrapping */
#define AG_EDITABLE_NOPOPUP	  0x080000 /* Disable popup menu */
#define AG_EDITABLE_WORDSELECT	  0x100000 /* Select whole words */
#define AG_EDITABLE_READONLY	  0x200000 /* Disable user input */
#define AG_EDITABLE_MULTILINGUAL  0x400000 /* Multilingual edition */

	const char *encoding;		/* Character set (default "UTF-8") */
	AG_Text *text;			/* Default binding */
	int wPre, hPre;			/* Size hint */
	int pos;			/* Cursor position */
	int sel;			/* Selection offset / range */
	int selDblClick;		/* Double click position */
	Uint32 compose;			/* For input composition */
	int xCurs, yCurs;		/* Last cursor position */
	int xSelStart, ySelStart;	/* Last selection start position */
	int xSelEnd, ySelEnd;		/* Last selection end position */
	int xCursPref;			/* Requested cursor position */
	int x;				/* Horizontal offset (px) */
	int xMax;			/* Rightmost x of largest line (px) */
	int y;				/* Vertical offset (lines) */
	int yMax;			/* Lowest y (lines) */
	int yVis;			/* Maximum visible area (lines) */
	Uint32 wheelTicks;		/* For wheel acceleration */
	AG_EditableBuffer sBuf;		/* Working buffer (for STATIC) */
	AG_Rect r;			/* View area */
	AG_CursorArea *ca;		/* Text cursor-change area */
	int fontMaxHeight;		/* Maximum character height */
	int lineSkip;			/* Y-increment in multiline mode */
	struct ag_popup_menu *pm;	/* Right-click popup menu */
	enum ag_language lang;		/* Selected language (for AG_Text) */
	int xScrollPx;			/* Explicit scroll request in pixels */
	int *xScrollTo, *yScrollTo;	/* Scroll to specified position */
	AG_Timer toRepeat;		/* Key repeat timer */
	AG_Timer toCursorBlink;		/* Cursor blink timer */
	AG_Timer toDblClick;		/* Double click timer */
} AG_Editable;

#define AGEDITABLE(p) ((AG_Editable *)(p))
#ifdef _AGAR_INTERNAL
#define EDITABLE(p) AGEDITABLE(p)
#endif

__BEGIN_DECLS
extern AG_WidgetClass agEditableClass;
extern AG_EditableClipboard agEditableClipbrd;
extern AG_EditableClipboard agEditableKillring;

AG_Editable *AG_EditableNew(void *, Uint);
void         AG_EditableBindUTF8(AG_Editable *, char *, size_t);
void         AG_EditableBindASCII(AG_Editable *, char *, size_t);
void         AG_EditableBindEncoded(AG_Editable *, const char *, char *, size_t);
void         AG_EditableBindText(AG_Editable *, AG_Text *);
void         AG_EditableSetLang(AG_Editable *, enum ag_language);

AG_EditableBuffer *AG_EditableGetBuffer(AG_Editable *);
void               AG_EditableReleaseBuffer(AG_Editable *, AG_EditableBuffer *);
void               AG_EditableClearBuffer(AG_Editable *, AG_EditableBuffer *);
int                AG_EditableGrowBuffer(AG_Editable *, AG_EditableBuffer *,
                                         Uint32 *, size_t);

int  AG_EditableCut(AG_Editable *, AG_EditableBuffer *, AG_EditableClipboard *);
int  AG_EditableCopy(AG_Editable *, AG_EditableBuffer *, AG_EditableClipboard *);
void AG_EditableCopyChunk(AG_Editable *, AG_EditableClipboard *, Uint32 *,
                          size_t);
int  AG_EditablePaste(AG_Editable *, AG_EditableBuffer *, AG_EditableClipboard *);
int  AG_EditableDelete(AG_Editable *, AG_EditableBuffer *);
void AG_EditableSelectAll(AG_Editable *, AG_EditableBuffer *);

void         AG_EditableSizeHint(AG_Editable *, const char *);
void         AG_EditableSizeHintPixels(AG_Editable *, Uint, Uint);
void         AG_EditableSizeHintLines(AG_Editable *, Uint);
#define      AG_EditablePrescale AG_EditableSizeHint
void         AG_EditableSetPassword(AG_Editable *, int);
void         AG_EditableSetWordWrap(AG_Editable *, int);
void         AG_EditableSetExcl(AG_Editable *, int);
void         AG_EditableSetFltOnly(AG_Editable *, int);
void         AG_EditableSetIntOnly(AG_Editable *, int);

void     AG_EditableSetString(AG_Editable *, const char *);
#define  AG_EditableClearString(tb) AG_EditableSetString((tb),NULL)
void     AG_EditablePrintf(void *, const char *, ...);
char    *AG_EditableDupString(AG_Editable *);
size_t   AG_EditableCopyString(AG_Editable *, char *, size_t)
                               BOUNDED_ATTRIBUTE(__string__, 2, 3);
int      AG_EditableInt(AG_Editable *);
float    AG_EditableFlt(AG_Editable *);
double   AG_EditableDbl(AG_Editable *);

void     AG_EditableInitClipboards(void);
void     AG_EditableDestroyClipboards(void);

int  AG_EditableMapPosition(AG_Editable *, AG_EditableBuffer *, int, int, int *);
void AG_EditableMoveCursor(AG_Editable *, AG_EditableBuffer *, int, int);
int  AG_EditableSetCursorPos(AG_Editable *, AG_EditableBuffer *, int);

/* Return current cursor position in text. */
static __inline__ int
AG_EditableGetCursorPos(AG_Editable *ed)
{
	int rv;
	AG_ObjectLock(ed);
	rv = ed->pos;
	AG_ObjectUnlock(ed);
	return (rv);
}

/* Return 1 if the Editable is effectively read-only. */
static __inline__ int
AG_EditableReadOnly(AG_Editable *ed)
{
	return (ed->flags & AG_EDITABLE_READONLY) || AG_WidgetDisabled(ed);
}

/*
 * Ensure that the selection range is valid. The Editable and buffer
 * must both be locked.
 */
static __inline__ void
AG_EditableValidateSelection(AG_Editable *ed, AG_EditableBuffer *buf)
{
	if ((Uint)ed->pos > buf->len) {
		ed->pos = (int)buf->len;
		ed->sel = 0;
	}
	if (ed->sel != 0) {
		int ep = ed->pos + ed->sel;
		if (ep < 0) {
			ed->pos = 0;
			ed->sel = 0;
		} else if ((Uint)ep > buf->len) {
			ed->pos = (int)buf->len;
			ed->sel = 0;
		}
	}
}	

#ifdef AG_LEGACY
# define AG_EditableSetFont AG_SetFont
# define AG_EditableSetStatic AG_EditableSetExcl
# define AG_EDITABLE_STATIC AG_EDITABLE_EXCL
# define AG_EDITABLE_NOWORDSEEK 0
# define AG_EditableSetWriteable(tb,flag) do {	\
	if (flag) {				\
	 	AG_WidgetEnable(tb);		\
	} else {				\
		AG_WidgetDisable(tb);		\
	}					\
} while (0)
#endif /* AG_LEGACY */

__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_EDITABLE_H_ */
