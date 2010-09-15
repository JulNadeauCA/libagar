/*	Public domain	*/

#ifndef _AGAR_WIDGET_DIR_DLG_H_
#define _AGAR_WIDGET_DIR_DLG_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/window.h>
#include <agar/gui/tlist.h>
#include <agar/gui/combo.h>
#include <agar/gui/pane.h>
#include <agar/gui/label.h>

#include <agar/gui/begin.h>

struct ag_dir_dlg;

typedef struct ag_dir_dlg {
	AG_Widget wid;
	Uint flags;
#define AG_DIRDLG_MULTI		0x001	/* Return a set of directories */
#define AG_DIRDLG_CLOSEWIN	0x002	/* Close parent window on success or
					   if "Cancel" is pressed */
#define AG_DIRDLG_LOAD		0x004	/* Directory must exist and be readable */
#define AG_DIRDLG_SAVE		0x008	/* Directory must be writeable */
#define AG_DIRDLG_ASYNC		0x010	/* Separate thread for load/save fn */
#define AG_DIRDLG_RESET_ONSHOW	0x020	/* Reset listing on show */
#define AG_DIRDLG_HFILL		0x100
#define AG_DIRDLG_VFILL		0x200
#define AG_DIRDLG_EXPAND	(AG_DIRDLG_HFILL|AG_DIRDLG_VFILL)

	char cwd[AG_PATHNAME_MAX];		/* Current working directory */
	AG_Tlist *tlDirs;			/* List of directories */
	AG_Textbox *tbInput;			/* Filename input */
	AG_Button *btnOk;			/* OK button */
	AG_Button *btnCancel;			/* Cancel button */
	AG_Event *okAction;			/* OK action */
	AG_Event *cancelAction;			/* Cancel action */
	char *dirMRU;				/* MRU Directory */
	AG_Combo *comLoc;			/* Locations list */
} AG_DirDlg;

__BEGIN_DECLS
extern AG_WidgetClass agDirDlgClass;

AG_DirDlg *AG_DirDlgNew(void *, Uint);
AG_DirDlg *AG_DirDlgNewMRU(void *, const char *, Uint);

void AG_DirDlgSetDirectoryMRU(AG_DirDlg *, const char *, const char *);
int  AG_DirDlgSetDirectoryS(AG_DirDlg *, const char *);
int  AG_DirDlgSetDirectory(AG_DirDlg *, const char *, ...)
                            FORMAT_ATTRIBUTE(printf,2,3)
			    NONNULL_ATTRIBUTE(2);
void AG_DirDlgOkAction(AG_DirDlg *, AG_EventFn, const char *, ...);
void AG_DirDlgCancelAction(AG_DirDlg *, AG_EventFn, const char *, ...);
int  AG_DirDlgCheckReadAccess(AG_DirDlg *);
int  AG_DirDlgCheckWriteAccess(AG_DirDlg *);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_DIR_DLG_H_ */
