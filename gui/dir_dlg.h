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
#define AG_DIRDLG_RESET_ONSHOW	0x020	/* Reset listing on show */
#define AG_DIRDLG_HFILL		0x100
#define AG_DIRDLG_VFILL		0x200
#define AG_DIRDLG_EXPAND	(AG_DIRDLG_HFILL|AG_DIRDLG_VFILL)
#define AG_DIRDLG_NOBUTTONS	0x400	/* No OK/Cancel buttons */

	char cwd[AG_PATHNAME_MAX-4];		/* Current working directory */

	AG_Tlist   *_Nonnull tlDirs;		/* List of directories */
	AG_Textbox *_Nonnull tbInput;		/* Filename input */
	AG_Button  *_Nonnull btnOk;		/* "OK" button */
	AG_Button  *_Nonnull btnCancel;		/* "Cancel" button */

	AG_Event *_Nullable okAction;		/* "OK" callback */
	AG_Event *_Nullable cancelAction;	/* "Cancel" callback */

	char *_Nullable dirMRU;			/* MRU Directory */
	AG_Combo *_Nonnull comLoc;		/* Locations list */
} AG_DirDlg;

#define AGDIRDLG(obj)            ((AG_DirDlg *)(obj))
#define AGCDIRDLG(obj)           ((const AG_DirDlg *)(obj))
#define AG_DIRDLG_SELF()          AGDIRDLG( AG_OBJECT(0,"AG_Widget:AG_DirDlg:*") )
#define AG_DIRDLG_PTR(n)          AGDIRDLG( AG_OBJECT((n),"AG_Widget:AG_DirDlg:*") )
#define AG_DIRDLG_NAMED(n)        AGDIRDLG( AG_OBJECT_NAMED((n),"AG_Widget:AG_DirDlg:*") )
#define AG_CONST_DIRDLG_SELF()   AGCDIRDLG( AG_CONST_OBJECT(0,"AG_Widget:AG_DirDlg:*") )
#define AG_CONST_DIRDLG_PTR(n)   AGCDIRDLG( AG_CONST_OBJECT((n),"AG_Widget:AG_DirDlg:*") )
#define AG_CONST_DIRDLG_NAMED(n) AGCDIRDLG( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_DirDlg:*") )

__BEGIN_DECLS
extern AG_WidgetClass agDirDlgClass;

AG_DirDlg *_Nonnull AG_DirDlgNew(void *_Nullable, Uint);
AG_DirDlg *_Nonnull AG_DirDlgNewMRU(void *_Nullable, const char *_Nonnull, Uint);

void AG_DirDlgSetDirectoryMRU(AG_DirDlg *_Nonnull, const char *_Nonnull,
                              const char *_Nonnull);
int  AG_DirDlgSetDirectoryS(AG_DirDlg *_Nonnull, const char *_Nullable);
int  AG_DirDlgSetDirectory(AG_DirDlg *_Nonnull, const char *_Nonnull, ...)
                          FORMAT_ATTRIBUTE(printf,2,3);
void AG_DirDlgOkAction(AG_DirDlg *_Nonnull, _Nonnull AG_EventFn,
                       const char *_Nullable, ...);
void AG_DirDlgCancelAction(AG_DirDlg *_Nonnull, _Nonnull AG_EventFn,
                           const char *_Nullable, ...);
int  AG_DirDlgCheckReadAccess(AG_DirDlg *_Nonnull);
int  AG_DirDlgCheckWriteAccess(AG_DirDlg *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_DIR_DLG_H_ */
