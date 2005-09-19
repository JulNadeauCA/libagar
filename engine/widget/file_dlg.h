/*	$Csoft: file_dlg.h,v 1.3 2005/09/19 05:25:23 vedge Exp $	*/
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

struct AGFileType {
	const char *descr;		/* Description */
	char **exts;			/* Filename extensions */
	u_int nexts;
	struct event *action;		/* Action (save/load) */
	TAILQ_ENTRY(AGFileType) types;
};

struct AGFileDlg {
	struct widget wid;
	int flags;
#define FILEDLG_MULTI	0x01		/* Return a set of files */
#define FILEDLG_NOCLOSE	0x02		/* Never close the parent window */

	char cwd[MAXPATHLEN];		/* Current working directory */
	struct hpane *hPane;
	struct hpane_div *hDiv;
	struct tlist *tlDirs;		/* List of directories */
	struct tlist *tlFiles;		/* List of files */
	struct textbox *tbFile;		/* Filename input */
	struct combo *comTypes;		/* File types combo */
	struct button *btnOk;		/* OK button */
	struct button *btnCancel;	/* Cancel button */
	TAILQ_HEAD(,AGFileType) types;	/* File type handlers */
};

__BEGIN_DECLS
struct AGFileDlg *file_dlg_new(void *, int, const char *, const char *);
void file_dlg_init(struct AGFileDlg *, int, const char *, const char *);
void file_dlg_scale(void *, int, int);
void file_dlg_destroy(void *);

struct AGFileType *file_dlg_type(struct AGFileDlg *, const char *,
				 const char *, void (*)(int, union evarg *),
				 const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_FILE_DLG_H_ */
