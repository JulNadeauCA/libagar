/*	$Csoft: file_dlg.h,v 1.5 2005/09/27 00:25:22 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_FILE_DLG_H_
#define _AGAR_WIDGET_FILE_DLG_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/window.h>
#include <agar/gui/tlist.h>
#include <agar/gui/combo.h>
#include <agar/gui/hpane.h>
#include <agar/gui/label.h>

#include "begin_code.h"

typedef struct ag_file_type {
	const char *descr;			/* Description */
	char **exts;				/* Filename extensions */
	Uint nexts;
	AG_Event *action;			/* Action (save/load) */
	TAILQ_ENTRY(ag_file_type) types;
} AG_FileType;

typedef struct ag_file_dlg {
	AG_Widget wid;
	Uint flags;
#define AG_FILEDLG_MULTI	0x01	/* Return a set of files */
#define AG_FILEDLG_CLOSEWIN	0x02	/* Close the parent window when done */
#define AG_FILEDLG_LOAD		0x04	/* File must exist and be readable */
#define AG_FILEDLG_SAVE		0x08	/* File must be writeable */

	char cwd[MAXPATHLEN];			/* Current working directory */
	char cfile[MAXPATHLEN];			/* Current file path */
	AG_HPane *hPane;
	AG_HPaneDiv *hDiv;
	AG_Tlist *tlDirs;			/* List of directories */
	AG_Tlist *tlFiles;			/* List of files */
	AG_Label *lbCwd;			/* CWD label */
	AG_Textbox *tbFile;			/* Filename input */
	AG_Combo *comTypes;			/* File types combo */
	AG_Button *btnOk;			/* OK button */
	AG_Button *btnCancel;			/* Cancel button */
	TAILQ_HEAD(,ag_file_type) types;	/* File type handlers */
} AG_FileDlg;

__BEGIN_DECLS
AG_FileDlg *AG_FileDlgNew(void *, Uint);
void AG_FileDlgInit(AG_FileDlg *, Uint);
void AG_FileDlgScale(void *, int, int);
void AG_FileDlgDestroy(void *);
int AG_FileDlgSetDirectory(AG_FileDlg *, const char *);
void AG_FileDlgSetFilename(AG_FileDlg *, const char *, ...);

AG_FileType *AG_FileDlgAddType(AG_FileDlg *, const char *,
			       const char *, void (*)(AG_Event *),
			       const char *, ...);
__inline__ int AG_FileDlgAtRoot(AG_FileDlg *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_FILE_DLG_H_ */
