/*	Public domain	*/

#ifndef _AGAR_GUI_EDITABLE_H_
#define _AGAR_GUI_EDITABLE_H_

#include <agar/gui/widget.h>
#include <agar/gui/text.h>

#include <agar/gui/begin.h>

#define AG_EDITABLE_STRING_MAX 1024	/* For "default" string binding */

struct ag_cursor_area;

/* Working UCS-4 text buffer for internal use */
typedef struct ag_editable_buffer {
	AG_Variable *var;		/* Variable binding (if any) */
	Uint32 *s;			/* String buffer */
	size_t len;			/* String length (chars) */
	size_t maxLen;			/* Available buffer size (bytes) */
	int reallocable;		/* Buffer can be realloc'd */
} AG_EditableBuffer;

typedef struct ag_editable {
	struct ag_widget wid;
	
	Uint flags;
#define AG_EDITABLE_HFILL         0x00001
#define AG_EDITABLE_VFILL         0x00002
#define AG_EDITABLE_EXPAND        (AG_EDITABLE_HFILL|AG_EDITABLE_VFILL)
#define AG_EDITABLE_MULTILINE     0x00004 /* Multiline edition */
#define AG_EDITABLE_BLINK_ON      0x00008 /* Cursor blink state (internal) */
#define AG_EDITABLE_PASSWORD      0x00010 /* Password (hidden) input */
#define AG_EDITABLE_ABANDON_FOCUS 0x00020 /* Abandon focus on return */
#define AG_EDITABLE_INT_ONLY      0x00040 /* Accepts only int input */
#define AG_EDITABLE_FLT_ONLY      0x00080 /* Accepts only float input */
#define AG_EDITABLE_CATCH_TAB     0x00100 /* Process tab key input */
#define AG_EDITABLE_CURSOR_MOVING 0x00200 /* Cursor is being moved */
#define AG_EDITABLE_NOSCROLL      0x00800 /* Inhibit automatic scrolling */
#define AG_EDITABLE_NOSCROLL_ONCE 0x01000 /* Inhibit scrolling at next draw */
#define AG_EDITABLE_MARKPREF      0x02000 /* Mark current cursor position */
#define AG_EDITABLE_EXCL          0x04000 /* Exclusive access to buffer */
#define AG_EDITABLE_NOEMACS       0x08000 /* Disable emacs-style fn keys */
#define AG_EDITABLE_NOWORDSEEK    0x10000 /* Disable ALT+b/ALT+f emacs keys */
#define AG_EDITABLE_NOLATIN1      0x20000 /* Disable LATIN-1 combinations */
#define AG_EDITABLE_WORDWRAP      0x40000 /* Word wrapping */
#define AG_EDITABLE_GROW          0x80000 /* Grow string buffer as needed */

	const char *encoding;		/* Character set (default "UTF-8") */

	char string[AG_EDITABLE_STRING_MAX]; /* Default "string" binding */
	int wPre, hPre;			/* Size hint */
	int pos;			/* Cursor position */
	int sel;			/* Selection offset / range */
	int selDblClick;		/* Double click position */
	Uint32 compose;			/* For input composition */
	int xCurs, yCurs;		/* Last cursor position */
	int xCursPref;			/* "Preferred" cursor position */
	AG_Timeout toDelay;		/* Pre-repeat delay timer */
	AG_Timeout toRepeat;		/* Repeat timer */
	AG_Timeout toCursorBlink;	/* Cursor blink timer */
	int x;				/* Horizontal offset (px) */
	int xMax;			/* Rightmost x of largest line (px) */
	int y;				/* Vertical offset (lines) */
	int yMax;			/* Lowest y (lines) */
	int yVis;			/* Maximum visible area (lines) */
	Uint32 wheelTicks;		/* For wheel acceleration */
	AG_KeySym repeatKey;		/* Last keysym */
	AG_KeyMod repeatMod;		/* Last keymod */
	Uint32 repeatUnicode;		/* Last unicode translated key */
	AG_EditableBuffer sBuf;		/* Working buffer (for STATIC) */
	AG_Rect r;			/* View area */
	struct ag_cursor_area *ca;	/* For "text" cursor change */
	AG_Font *font;			/* Font for text rendering */
} AG_Editable;

#define AGEDITABLE(p) ((AG_Editable *)(p))
#ifdef _AGAR_INTERNAL
#define EDITABLE(p) AGEDITABLE(p)
#endif

__BEGIN_DECLS
extern AG_WidgetClass agEditableClass;

AG_Editable *AG_EditableNew(void *, Uint);
void         AG_EditableBindUTF8(AG_Editable *, char *, size_t);
void         AG_EditableBindASCII(AG_Editable *, char *, size_t);
void         AG_EditableBindEncoded(AG_Editable *, const char *, char *, size_t);
void         AG_EditableBindText(AG_Editable *, AG_Text *);

void         AG_EditableSizeHint(AG_Editable *, const char *);
void         AG_EditableSizeHintPixels(AG_Editable *, Uint, Uint);
void         AG_EditableSizeHintLines(AG_Editable *, Uint);
#define      AG_EditablePrescale AG_EditableSizeHint
void         AG_EditableSetPassword(AG_Editable *, int);
void         AG_EditableSetWordWrap(AG_Editable *, int);
void         AG_EditableSetExcl(AG_Editable *, int);
void         AG_EditableSetFltOnly(AG_Editable *, int);
void         AG_EditableSetIntOnly(AG_Editable *, int);
void         AG_EditableSetFont(AG_Editable *, AG_Font *);

int  AG_EditableMapPosition(AG_Editable *, int, int, int *, int);
void AG_EditableMoveCursor(AG_Editable *, int, int, int);
int  AG_EditableGetCursorPos(AG_Editable *);
int  AG_EditableSetCursorPos(AG_Editable *, int);

void     AG_EditableSetString(AG_Editable *, const char *);
#define  AG_EditableClearString(tb) AG_EditableSetString((tb),NULL)
void     AG_EditablePrintf(void *, const char *, ...);
char    *AG_EditableDupString(AG_Editable *);
size_t   AG_EditableCopyString(AG_Editable *, char *, size_t)
                               BOUNDED_ATTRIBUTE(__string__, 2, 3);
int      AG_EditableInt(AG_Editable *);
float    AG_EditableFlt(AG_Editable *);
double   AG_EditableDbl(AG_Editable *);

#ifdef AG_LEGACY
# define AG_EditableSetStatic AG_EditableSetExcl
# define AG_EDITABLE_STATIC AG_EDITABLE_EXCL
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
