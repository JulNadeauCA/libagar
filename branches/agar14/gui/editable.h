/*	Public domain	*/

#ifndef _AGAR_WIDGET_EDITABLE_H_
#define _AGAR_WIDGET_EDITABLE_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

#define AG_EDITABLE_STRING_MAX 1024

enum ag_editable_encoding {
	AG_ENCODING_UTF8,
	AG_ENCODING_ASCII
};

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
#define AG_EDITABLE_NO_HFILL      0x00400
#define AG_EDITABLE_NOSCROLL      0x00800 /* Inhibit automatic scrolling */
#define AG_EDITABLE_NOSCROLL_ONCE 0x01000 /* Inhibit scrolling at next draw */
#define AG_EDITABLE_MARKPREF      0x02000 /* Mark current cursor position */
#define AG_EDITABLE_STATIC        0x04000 /* String binding will not change */
#define AG_EDITABLE_NOEMACS       0x08000 /* Disable emacs-style fn keys */
#define AG_EDITABLE_NOWORDSEEK    0x10000 /* Disable ALT+b/ALT+f emacs keys */
#define AG_EDITABLE_NOLATIN1      0x20000 /* Disable LATIN-1 combinations */

	enum ag_editable_encoding encoding;  /* Character set of buffer */
	char string[AG_EDITABLE_STRING_MAX]; /* Default string binding */
	int wPre, hPre;			/* Size hint */
	int pos;			/* Cursor position */
	Uint32 compose;			/* For input composition */
	int xCurs, yCurs;		/* Last cursor position */
	int xCursPref;			/* "Preferred" cursor position */

	int sel_x1, sel_x2;		/* Selection points */
	int sel_edit;			/* Point being edited */

	AG_Timeout toDelay;		/* Pre-repeat delay timer */
	AG_Timeout toRepeat;		/* Repeat timer */
	AG_Timeout toCursorBlink;	/* Cursor blink timer */

	int x;				/* Horizontal offset (px) */
	int xMax;			/* Rightmost x of largest line (px) */
	int y;				/* Vertical offset (lines) */
	int yMax;			/* Lowest y (lines) */
	int yVis;			/* Maximum visible area (lines) */
	Uint32 wheelTicks;		/* For wheel acceleration */
	SDLKey repeatKey;		/* Last keysym */
	SDLMod repeatMod;		/* Last keymod */
	Uint32 repeatUnicode;		/* Last unicode translated key */
	Uint32 *ucsBuf;			/* UCS4 buffer (for STATIC) */
	Uint    ucsLen;			/* Buffer length (for STATIC) */
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

void         AG_EditableSizeHint(AG_Editable *, const char *);
void         AG_EditableSizeHintPixels(AG_Editable *, Uint, Uint);
#define      AG_EditablePrescale AG_EditableSizeHint
void         AG_EditableSetPassword(AG_Editable *, int);
void         AG_EditableSetStatic(AG_Editable *, int);
void         AG_EditableSetFltOnly(AG_Editable *, int);
void         AG_EditableSetIntOnly(AG_Editable *, int);

int  AG_EditableMapPosition(AG_Editable *, int, int, int *, int);
void AG_EditableMoveCursor(AG_Editable *, int, int, int);
int  AG_EditableGetCursorPos(AG_Editable *);
int  AG_EditableSetCursorPos(AG_Editable *, int);

void     AG_EditableSetString(AG_Editable *, const char *);
void     AG_EditableSetStringUCS4(AG_Editable *, const Uint32 *);
#define  AG_EditableClearString(tb) AG_EditableSetString((tb),NULL)
void     AG_EditablePrintf(AG_Editable *, const char *, ...);
char    *AG_EditableDupString(AG_Editable *);
Uint32  *AG_EditableDupStringUCS4(AG_Editable *);
size_t   AG_EditableCopyString(AG_Editable *, char *, size_t)
                               BOUNDED_ATTRIBUTE(__string__, 2, 3);
size_t   AG_EditableCopyStringUCS4(AG_Editable *, Uint32 *, size_t);
int      AG_EditableInt(AG_Editable *);
float    AG_EditableFlt(AG_Editable *);
double   AG_EditableDbl(AG_Editable *);

/* Legacy */
#define AG_EditableSetWriteable(tb,flag)	do {	\
	if (flag) {				\
	 	AG_WidgetEnable(tb);		\
	} else {				\
		AG_WidgetDisable(tb);		\
	}					\
} while (0)

static __inline__ void
AG_EditableBufferChanged(AG_Editable *ed)
{
	AG_ObjectLock(ed);
	if (ed->flags & AG_EDITABLE_STATIC) {
		free(ed->ucsBuf);
		ed->ucsBuf = NULL;
		ed->ucsLen = 0;
	}
	AG_ObjectUnlock(ed);
}
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_EDITABLE_H_ */
