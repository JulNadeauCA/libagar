/*	Public domain	*/

#ifndef _AGAR_GUI_TEXTBOX_H_
#define _AGAR_GUI_TEXTBOX_H_

#include <agar/gui/widget.h>
#include <agar/gui/editable.h>
#include <agar/gui/label.h>
#include <agar/gui/scrollbar.h>

#include <agar/gui/begin.h>

typedef struct ag_textbox {
	struct ag_widget _inherit;
	struct ag_editable *ed;

	AG_Label *lbl;			/* Textbox label */
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
	AG_Scrollbar *hBar, *vBar;	/* Scrollbars for MULTILINE */
	AG_Rect r;			/* View area */
	AG_Text *text;			/* Pointer to default binding */
} AG_Textbox;

#define AGTEXTBOX(p) ((AG_Textbox *)(p))
#ifdef _AGAR_INTERNAL
#define TEXTBOX(p) AGTEXTBOX(p)
#endif

__BEGIN_DECLS
extern AG_WidgetClass agTextboxClass;

AG_Textbox *AG_TextboxNew(void *, Uint, const char *, ...)
                          FORMAT_ATTRIBUTE(printf,3,4);
AG_Textbox *AG_TextboxNewS(void *, Uint, const char *);
#define     AG_TextboxSizeHint(tb,text) AG_EditableSizeHint((tb)->ed,(text))
#define     AG_TextboxSizeHintPixels(tb,w,h) \
            AG_EditableSizeHintPixels((tb)->ed,(w),(h))
#define     AG_TextboxSizeHintLines(tb,l) \
            AG_EditableSizeHintLines((tb)->ed,(l))
void        AG_TextboxSetLabel(AG_Textbox *, const char *, ...)
                               FORMAT_ATTRIBUTE(printf,2,3)
			       NONNULL_ATTRIBUTE(2);
void        AG_TextboxSetLabelS(AG_Textbox *, const char *);
void        AG_TextboxSetWordWrap(AG_Textbox *, int);
void        AG_TextboxPrintf(AG_Textbox *, const char *, ...);

#define     AG_TextboxSetPassword(tb,flag) AG_EditableSetPassword((tb)->ed,(flag))
#define     AG_TextboxSetExcl(tb,flag) AG_EditableSetExcl((tb)->ed,(flag))
#define     AG_TextboxSetFltOnly(tb,flag) AG_EditableSetFltOnly((tb)->ed,(flag))
#define     AG_TextboxSetIntOnly(tb,flag) AG_EditableSetIntOnly((tb)->ed,(flag))
#define     AG_TextboxSetLang(tb,lang) AG_EditableSetLang((tb)->ed,(lang))

int         AG_TextboxMapPosition(AG_Textbox *, int, int, int *);
void        AG_TextboxMoveCursor(AG_Textbox *, int, int);
void        AG_TextboxSetCursorPos(AG_Textbox *, int);
#define     AG_TextboxGetCursorPos(tb) AG_EditableGetCursorPos((tb)->ed)

#define AG_TextboxBindUTF8(tb,p,sz)        AG_EditableBindUTF8((tb)->ed,(p),(sz))
#define AG_TextboxBindASCII(tb,p,sz)       AG_EditableBindASCII((tb)->ed,(p),(sz))
#define AG_TextboxBindEncoded(tb,enc,p,sz) AG_EditableBindEncoded((tb)->ed,(enc),(p),(sz))
#define AG_TextboxBindText(tb,txt)         AG_EditableBindText((tb)->ed,(txt))

#define AG_TextboxSetString(tb,s) AG_EditableSetString((tb)->ed,(s))
#define	AG_TextboxClearString(tb) AG_EditableSetString((tb)->ed,NULL)
#define AG_TextboxDupString(tb) AG_EditableDupString((tb)->ed)
#define AG_TextboxCopyString(tb,p,len) AG_EditableCopyString((tb)->ed,(p),(len))

#define AG_TextboxInt(tb) AG_EditableInt((tb)->ed)
#define AG_TextboxFlt(tb) AG_EditableFlt((tb)->ed)
#define AG_TextboxDbl(tb) AG_EditableDbl((tb)->ed)

#ifdef AG_LEGACY
# define AG_TEXTBOX_NOWORDSEEK 0
# define AG_TEXTBOX_STATIC AG_TEXTBOX_EXCL
# define AG_TextboxSetStatic AG_TextboxSetExcl
# define AG_TextboxPrescale AG_TextboxSizeHint
# define AG_TextboxSetWriteable(tb,flag) do {	\
	if (flag) {				\
	 	AG_WidgetEnable(tb);		\
	} else {				\
		AG_WidgetDisable(tb);		\
	}					\
} while (0)
#endif /* AG_LEGACY */

__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_TEXTBOX_H_ */
