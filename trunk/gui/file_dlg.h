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

#include "begin_code.h"

typedef struct ag_file_type {
	const char *descr;			/* Description */
	char **exts;				/* Filename extensions */
	u_int nexts;
	AG_Event *action;			/* Action (save/load) */
	TAILQ_ENTRY(ag_file_type) types;
} AG_FileType;

typedef struct ag_file_dlg {
	AG_Widget wid;
	int flags;
#define AG_FILEDLG_MULTI	0x01	/* Return a set of files */
#define AG_FILEDLG_NOCLOSE	0x02	/* Never close the parent window */

	char cwd[MAXPATHLEN];			/* Current working directory */
	AG_HPane *hPane;
	AG_HPaneDiv *hDiv;
	AG_Tlist *tlDirs;			/* List of directories */
	AG_Tlist *tlFiles;			/* List of files */
	AG_Textbox *tbFile;			/* Filename input */
	AG_Combo *comTypes;			/* File types combo */
	AG_Button *btnOk;			/* OK button */
	AG_Button *btnCancel;			/* Cancel button */
	TAILQ_HEAD(,ag_file_type) types;	/* File type handlers */
} AG_FileDlg;

__BEGIN_DECLS
AG_FileDlg *AG_FileDlgNew(void *, int, const char *, const char *);
void AG_FileDlgInit(AG_FileDlg *, int, const char *, const char *);
void AG_FileDlgScale(void *, int, int);
void AG_FileDlgDestroy(void *);

AG_FileType *AG_FileDlgAddType(AG_FileDlg *, const char *,
			       const char *, void (*)(AG_Event *),
			       const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_FILE_DLG_H_ */
