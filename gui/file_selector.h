/*	Public domain	*/

#ifndef _AGAR_WIDGET_FILE_SELECTOR_H_
#define _AGAR_WIDGET_FILE_SELECTOR_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/window.h>
#include <agar/gui/file_dlg.h>

#include <agar/gui/begin.h>

typedef struct ag_file_selector {
	struct ag_widget wid;
	Uint flags;
#define AG_FILE_SELECTOR_HFILL	  0x01
#define AG_FILE_SELECTOR_VFILL	  0x02
#define AG_FILE_SELECTOR_EXPAND	  (AG_FILE_SELECTOR_HFILL|\
				   AG_FILE_SELECTOR_VFILL)
#define AG_FILE_SELECTOR_ANY_FILE 0x04	/* Don't check file access */

	char inTxt[AG_PATHNAME_MAX];	/* Input text buffer */
	AG_Textbox *tbox;		/* Textbox */
	AG_Button *button;		/* "Browse" button */
	AG_FileDlg *filedlg;		/* File selection widget */
	AG_Window *panel;
	int wSaved, hSaved;		/* Saved popup geometry */
	int wPreList, hPreList;		/* Size hints */
} AG_FileSelector;

__BEGIN_DECLS
extern AG_WidgetClass agFileSelectorClass;

AG_FileSelector *AG_FileSelectorNew(void *, Uint, const char *);
void AG_FileSelectorSetFile(AG_FileSelector *, const char *);
void AG_FileSelectorSetDirectory(AG_FileSelector *, const char *);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_FILE_SELECTOR_H_ */
