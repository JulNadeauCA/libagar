/*	Public domain	*/

#ifndef _AGAR_WIDGET_TEXTBOX_H_
#define _AGAR_WIDGET_TEXTBOX_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/editable.h>
#include <gui/scrollbar.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/editable.h>
#include <agar/gui/scrollbar.h>
#endif

#include "begin_code.h"

#define AG_TEXTBOX_STRING_MAX AG_EDITABLE_STRING_MAX

typedef struct ag_textbox {
	struct ag_widget _inherit;
	struct ag_editable *ed;

	char *labelText;		/* Label text */
	int   label;			/* Label surface mapping */

	Uint flags;
#define AG_TEXTBOX_MULTILINE     0x00001 /* Enable multiline edition */
#define AG_TEXTBOX_PASSWORD      0x00004 /* Hide buffer contents */
#define AG_TEXTBOX_ABANDON_FOCUS 0x00008 /* Lose focus on return */
#define AG_TEXTBOX_COMBO         0x00010 /* Used by AG_Combo */
#define AG_TEXTBOX_HFILL         0x00020
#define AG_TEXTBOX_VFILL         0x00040
#define AG_TEXTBOX_EXPAND        (AG_TEXTBOX_HFILL|AG_TEXTBOX_VFILL)
#define AG_TEXTBOX_READONLY      0x00100 /* Equivalent to WidgetDisable() */
#define AG_TEXTBOX_INT_ONLY      0x00200 /* Accepts only valid strtol() input */
#define AG_TEXTBOX_FLT_ONLY      0x00400 /* Accepts only valid strtof() input */
#define AG_TEXTBOX_CATCH_TAB     0x00800 /* Enter literal tabs into text
					    instead of cycling focus */
#define AG_TEXTBOX_CURSOR_MOVING 0x01000 /* Cursor is being moved */
#define AG_TEXTBOX_NO_HFILL      0x02000
#define AG_TEXTBOX_STATIC        0x04000 /* String binding will not change */
#define AG_TEXTBOX_NOEMACS       0x08000 /* Disable emacs-style fn keys */
#define AG_TEXTBOX_NOWORDSEEK    0x10000 /* Disable ALT+b/ALT+f emacs keys */
#define AG_TEXTBOX_NOLATIN1      0x20000 /* Disable LATIN-1 combinations */

	int boxPadX, boxPadY;		/* Padding around textbox */
	int lblPadL, lblPadR;		/* Padding around label */
	int wLbl, hLbl;			/* Label dimensions */
	AG_Scrollbar *hBar, *vBar;	/* Scrollbars for MULTILINE */
} AG_Textbox;

#define AGTEXTBOX(p) ((AG_Textbox *)(p))
#ifdef _AGAR_INTERNAL
#define TEXTBOX(p) AGTEXTBOX(p)
#endif

__BEGIN_DECLS
extern AG_WidgetClass agTextboxClass;

AG_Textbox *AG_TextboxNew(void *, Uint, const char *);
#define     AG_TextboxSizeHint(tb,text) AG_EditableSizeHint((tb)->ed,(text))
#define     AG_TextboxSizeHintPixels(tb,w,h) \
            AG_EditableSizeHint((tb)->ed,(w),(h))
void        AG_TextboxSetLabel(AG_Textbox *, const char *, ...);
#define     AG_TextboxSetPassword(tb,flag) \
            AG_EditableSetPassword((tb)->ed,(flag))
#define     AG_TextboxSetStatic(tb,flag) AG_EditableSetStatic((tb)->ed,(flag))

#define AG_TextboxMapPosition(tb,x,y,pos,abs) \
	AG_EditableMapPosition((tb)->ed,(x),(y),(pos),(abs))
#define AG_TextboxMoveCursor(tb,x,y,abs) \
	AG_EditableMoveCursor((tb)->ed,(x),(y),(abs))
#define AG_TextboxGetCursorPos(tb) AG_EditableGetCursorPos((tb)->ed)
#define AG_TextboxSetCursorPos(tb,pos) AG_EditableSetCursorPos((tb)->ed,(pos))

#define AG_TextboxSetString(tb,s) AG_EditableSetString((tb)->ed,(s))
#define AG_TextboxSetStringUCS4(tb,s) AG_EditableSetStringUCS4((tb)->ed,(s))
#define	AG_TextboxClearString(tb) AG_EditableSetString((tb)->ed,NULL)
void    AG_TextboxPrintf(AG_Textbox *, const char *, ...);
#define AG_TextboxDupString(tb) AG_EditableDupString((tb)->ed)
#define AG_TextboxDupStringUCS4(tb) AG_EditableDupStringUCS4((tb)->ed)
#define AG_TextboxCopyString(tb,p,len) AG_EditableCopyString((tb)->ed,(p),(len))
#define AG_TextboxCopyStringUCS4(tb,p,len) \
	AG_EditableCopyStringUCS4((tb)->ed,(p),(len))

#define AG_TextboxBufferChanged(tb) AG_EditableBufferChanged((tb)->ed)
#define AG_TextboxInt(tb) AG_EditableInt((tb)->ed)
#define AG_TextboxFlt(tb) AG_EditableFlt((tb)->ed)
#define AG_TextboxDbl(tb) AG_EditableDbl((tb)->ed)

/* Legacy interfaces */
#define AG_TextboxPrescale AG_TextboxSizeHint
#define AG_TextboxSetWriteable(tb,flag)	do {	\
	if (flag) {				\
	 	AG_WidgetEnable(tb);		\
	} else {				\
		AG_WidgetDisable(tb);		\
	}					\
} while (0)
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TEXTBOX_H_ */
