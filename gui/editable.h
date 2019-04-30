/*	Public domain	*/

#ifndef _AGAR_GUI_EDITABLE_H_
#define _AGAR_GUI_EDITABLE_H_

#include <agar/gui/widget.h>
#include <agar/gui/text.h>

#include <agar/gui/begin.h>

struct ag_popup_menu;

/* Working UCS-4 text buffer for internal use */
typedef struct ag_editable_buffer {
	AG_Variable *_Nullable var;	/* Variable binding (if any) */
	Uint32 *_Nullable s;		/* String buffer (UCS-4 encoding) */
	AG_Size len;			/* String length (chars) */
	AG_Size maxLen;			/* Available buffer size (bytes) */
	int reallocable;		/* Buffer can be realloc'd */
} AG_EditableBuffer;

/* Internal clipboard for copy/paste and kill/yank */
typedef struct ag_editable_clipboard {
	_Nonnull_Mutex AG_Mutex lock;
	char encoding[32];		/* Character set encoding */
	Uint32 *_Nullable s;		/* UCS-4 buffer */
	AG_Size len;			/* Length in characters */
} AG_EditableClipboard;

typedef struct ag_editable {
	struct ag_widget wid;		/* AG_Widget -> AG_Editable */
	
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

	const char *_Nonnull encoding;	/* Character set (default "UTF-8") */
	AG_Text *_Nonnull text;		/* Default binding */
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
	AG_CursorArea *_Nullable ca;	/* Text cursor-change area */
	int fontMaxHeight;		/* Maximum character height */
	int lineSkip;			/* Y-increment in multiline mode */
	struct ag_popup_menu *_Nullable pm; /* Right-click popup menu */
	enum ag_language lang;		/* Selected language (for AG_Text) */
	int xScrollPx;			/* Explicit scroll request in pixels */
	int *_Nullable xScrollTo;	/* Scroll to that X-position */
	int *_Nullable yScrollTo;	/* Scroll to that Y-position */
	AG_Timer toRepeat;		/* Key repeat timer */
	AG_Timer toCursorBlink;		/* Cursor blink timer */
	AG_Timer toDblClick;		/* Double click timer */
} AG_Editable;

#define AGEDITABLE(p) ((AG_Editable *)(p))

__BEGIN_DECLS
extern AG_WidgetClass agEditableClass;
extern AG_EditableClipboard agEditableClipbrd;
extern AG_EditableClipboard agEditableKillring;

AG_Editable *_Nonnull AG_EditableNew(void *_Nullable , Uint);
void AG_EditableBindUTF8(AG_Editable *_Nonnull, char *_Nonnull, AG_Size);
void AG_EditableBindASCII(AG_Editable *_Nonnull, char *_Nonnull, AG_Size);
void AG_EditableBindEncoded(AG_Editable *_Nonnull, const char *_Nonnull,
                            char *_Nonnull, AG_Size);
void AG_EditableBindText(AG_Editable *_Nonnull, AG_Text *_Nonnull);
void AG_EditableSetLang(AG_Editable *_Nonnull, enum ag_language);

AG_EditableBuffer *_Nullable AG_EditableGetBuffer(AG_Editable *_Nonnull);
void AG_EditableReleaseBuffer(AG_Editable *_Nonnull, AG_EditableBuffer *_Nonnull);
void AG_EditableClearBuffer(AG_Editable *_Nonnull, AG_EditableBuffer *_Nonnull);
int  AG_EditableGrowBuffer(AG_Editable *_Nonnull, AG_EditableBuffer *_Nonnull,
                           Uint32 *_Nonnull, AG_Size);

int  AG_EditableCut(AG_Editable *_Nonnull, AG_EditableBuffer *_Nonnull,
                    AG_EditableClipboard *_Nonnull);
int  AG_EditableCopy(AG_Editable *_Nonnull, AG_EditableBuffer *_Nonnull,
                     AG_EditableClipboard *_Nonnull);
void AG_EditableCopyChunk(AG_Editable *_Nonnull, AG_EditableClipboard *_Nonnull,
                          Uint32 *_Nonnull, AG_Size);
int  AG_EditablePaste(AG_Editable *_Nonnull, AG_EditableBuffer *_Nonnull,
                      AG_EditableClipboard *_Nonnull);
int  AG_EditableDelete(AG_Editable *_Nonnull, AG_EditableBuffer *_Nonnull);
void AG_EditableSelectAll(AG_Editable *_Nonnull, AG_EditableBuffer *_Nonnull);

void AG_EditableSizeHint(AG_Editable *_Nonnull, const char *_Nonnull);
void AG_EditableSizeHintPixels(AG_Editable *_Nonnull, Uint,Uint);
void AG_EditableSizeHintLines(AG_Editable *_Nonnull, Uint);
void AG_EditableSetPassword(AG_Editable *_Nonnull, int);
void AG_EditableSetWordWrap(AG_Editable *_Nonnull, int);
void AG_EditableSetExcl(AG_Editable *_Nonnull, int);
void AG_EditableSetFltOnly(AG_Editable *_Nonnull, int);
void AG_EditableSetIntOnly(AG_Editable *_Nonnull, int);

void AG_EditableSetString(AG_Editable *_Nonnull, const char *_Nullable);
void AG_EditableClearString(AG_Editable *_Nonnull);
void AG_EditablePrintf(void *_Nonnull, const char *_Nullable, ...);

char *_Nonnull AG_EditableDupString(AG_Editable *_Nonnull);
AG_Size        AG_EditableCopyString(AG_Editable *_Nonnull, char *_Nonnull,
                                     AG_Size);

int    AG_EditableInt(AG_Editable *_Nonnull);

#ifdef AG_HAVE_FLOAT
float  AG_EditableFlt(AG_Editable *_Nonnull);
double AG_EditableDbl(AG_Editable *_Nonnull);
#endif

void AG_EditableInitClipboards(void);
void AG_EditableDestroyClipboards(void);

int  AG_EditableMapPosition(AG_Editable *_Nonnull, AG_EditableBuffer *_Nonnull,
                            int,int, int *_Nonnull);
void AG_EditableMoveCursor(AG_Editable *_Nonnull, AG_EditableBuffer *_Nonnull,
                           int,int);
int  AG_EditableGetCursorPos(AG_Editable *_Nonnull)
                            _Pure_Attribute_If_Unthreaded;
int  AG_EditableSetCursorPos(AG_Editable *_Nonnull, AG_EditableBuffer *_Nonnull,
                             int);
int  AG_EditableReadOnly(AG_Editable *_Nonnull)
                        _Pure_Attribute_If_Unthreaded;

void AG_EditableValidateSelection(AG_Editable *_Nonnull, AG_EditableBuffer *_Nonnull);

#ifdef AG_LEGACY
#define AG_EditablePrescale(ed,s) AG_EditableSizeHint((ed),(s))
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_EDITABLE_H_ */
