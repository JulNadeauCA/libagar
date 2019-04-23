/*	Public domain	*/

#ifndef _AGAR_GUI_TEXTBOX_H_
#define _AGAR_GUI_TEXTBOX_H_

#include <agar/gui/widget.h>
#include <agar/gui/editable.h>
#include <agar/gui/label.h>
#include <agar/gui/scrollbar.h>
#include <agar/gui/begin.h>

typedef struct ag_textbox {
	struct ag_widget _inherit;        /* AG_Widget -> AG_Textbox */

	struct ag_editable *_Nonnull ed;  /* Input field */
	AG_Label *_Nullable lbl;          /* Optional text label */

	Uint flags;
#define AG_TEXTBOX_MULTILINE     0x000001 /* Enable multiline edition */
#define AG_TEXTBOX_PASSWORD      0x000004 /* Hide buffer contents */
#define AG_TEXTBOX_ABANDON_FOCUS 0x000008 /* Lose focus on return */
#define AG_TEXTBOX_COMBO         0x000010 /* Used by AG_Combo */
#define AG_TEXTBOX_HFILL         0x000020
#define AG_TEXTBOX_VFILL         0x000040
#define AG_TEXTBOX_EXPAND        (AG_TEXTBOX_HFILL|AG_TEXTBOX_VFILL)
#define AG_TEXTBOX_READONLY      0x000100 /* Disable user input */
#define AG_TEXTBOX_INT_ONLY      0x000200 /* Accepts only valid strtol() input */
#define AG_TEXTBOX_FLT_ONLY      0x000400 /* Accepts only valid strtof() input */
#define AG_TEXTBOX_CATCH_TAB     0x000800 /* Enter literal tabs into text
					    instead of cycling focus */
#define AG_TEXTBOX_CURSOR_MOVING 0x001000 /* Cursor is being moved */
#define AG_TEXTBOX_EXCL          0x004000 /* Exclusive access to buffer */
#define AG_TEXTBOX_NOEMACS       0x008000 /* Disable emacs-style fn keys */
#define AG_TEXTBOX_NOLATIN1      0x020000 /* Disable LATIN-1 combinations */
#define AG_TEXTBOX_WORDWRAP	 0x040000 /* Enable word wrapping */
#define AG_TEXTBOX_NOPOPUP	 0x080000 /* Disable popup menu */
#define AG_TEXTBOX_MULTILINGUAL	 0x100000 /* Enable multilingual edition */

	int boxPadX, boxPadY;		/* Padding around textbox */
	AG_Scrollbar *_Nullable hBar;	/* Horizontal bar (for MULTILINE) */
	AG_Scrollbar *_Nullable vBar;	/* Vertical bar (for MULTILINE) */
	AG_Rect r;			/* View area */
	AG_Text *_Nonnull text;		/* Pointer to default binding */
} AG_Textbox;

#define AGTEXTBOX(p) ((AG_Textbox *)(p))

__BEGIN_DECLS
extern AG_WidgetClass agTextboxClass;

AG_Textbox *_Nonnull AG_TextboxNewS(void *_Nullable, Uint, const char *_Nullable);
AG_Textbox *_Nonnull AG_TextboxNew(void *_Nullable, Uint, const char *_Nullable, ...)
                                  FORMAT_ATTRIBUTE(printf,3,4);

void AG_TextboxSizeHint(AG_Textbox *_Nonnull, const char *_Nonnull);
void AG_TextboxSizeHintPixels(AG_Textbox *_Nonnull, Uint,Uint);
void AG_TextboxSizeHintLines(AG_Textbox *_Nonnull, Uint);

void AG_TextboxSetLabelS(AG_Textbox *_Nonnull, const char *_Nonnull);
void AG_TextboxSetLabel(AG_Textbox *_Nonnull, const char *_Nonnull, ...)
                       FORMAT_ATTRIBUTE(printf,2,3);

void AG_TextboxSetWordWrap(AG_Textbox *_Nonnull, int);
void AG_TextboxPrintf(AG_Textbox *_Nonnull, const char *_Nonnull, ...);

void AG_TextboxSetPassword(AG_Textbox *, int);
void AG_TextboxSetExcl(AG_Textbox *, int);
void AG_TextboxSetFltOnly(AG_Textbox *, int);
void AG_TextboxSetIntOnly(AG_Textbox *, int);
void AG_TextboxSetLang(AG_Textbox *, enum ag_language);

int  AG_TextboxMapPosition(AG_Textbox *_Nonnull, int,int, int *_Nonnull);
void AG_TextboxMoveCursor(AG_Textbox *_Nonnull, int,int);
void AG_TextboxSetCursorPos(AG_Textbox *_Nonnull, int);
int  AG_TextboxGetCursorPos(AG_Textbox *_Nonnull);

void AG_TextboxBindUTF8(AG_Textbox *_Nonnull, char *_Nonnull, AG_Size);
void AG_TextboxBindASCII(AG_Textbox *_Nonnull, char *_Nonnull, AG_Size);
void AG_TextboxBindEncoded(AG_Textbox *_Nonnull, const char *_Nonnull,
                           char *_Nonnull, AG_Size);
void AG_TextboxBindText(AG_Textbox *_Nonnull, AG_Text *_Nonnull);

void           AG_TextboxSetString(AG_Textbox *_Nonnull, const char *_Nonnull);
void           AG_TextboxClearString(AG_Textbox *_Nonnull);
char *_Nonnull AG_TextboxDupString(AG_Textbox *_Nonnull);
AG_Size        AG_TextboxCopyString(AG_Textbox *_Nonnull, char *_Nonnull,
                                    AG_Size);
int            AG_TextboxInt(AG_Textbox *_Nonnull);
float          AG_TextboxFloat(AG_Textbox *_Nonnull);
double         AG_TextboxDouble(AG_Textbox *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_TEXTBOX_H_ */
