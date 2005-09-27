/*	$Csoft: file_dlg.h,v 1.4 2005/09/19 07:26:29 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_FILE_DLG_H_
#define _AGAR_WIDGET_FILE_DLG_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>
#include <engine/widget/combo.h>
#include <engine/widget/hpane.h>

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
			       const char *, void (*)(int, union evarg *),
			       const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_FILE_DLG_H_ */
