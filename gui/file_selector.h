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
	AG_Textbox *_Nonnull  tbox;	/* Textbox */
	AG_Button  *_Nonnull  button;	/* "Browse" button */
	AG_FileDlg *_Nonnull  fileDlg;	/* File selection widget */
	AG_Window  *_Nullable panel;	/* Expanded panel */
	int wSaved, hSaved;		/* Saved popup geometry */
	int wPreList, hPreList;		/* Size hints */
} AG_FileSelector;

__BEGIN_DECLS
extern AG_WidgetClass agFileSelectorClass;

AG_FileSelector *_Nonnull AG_FileSelectorNew(void *_Nullable, Uint,
                                             const char *_Nullable);
void AG_FileSelectorSetFile(AG_FileSelector *_Nonnull, const char *_Nonnull);
void AG_FileSelectorSetDirectory(AG_FileSelector *_Nonnull, const char *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_FILE_SELECTOR_H_ */
