/*	$Csoft: file_dlg.h,v 1.9 2005/03/17 03:10:26 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_FILE_DLG_H_
#define _AGAR_WIDGET_FILE_DLG_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>
#include <engine/widget/combo.h>

#include "begin_code.h"

struct AGFileDlg {
	struct widget wid;
	
	int flags;
#define FILEDLG_MULTI	0x01		/* Return a set of files */

	char cwd[MAXPATHLEN];		/* Current working directory */

	struct tlist *tl_dirs;		/* List of directories */
	struct tlist *tl_files;		/* List of files */
	struct textbox *tb_file;	/* Filename input */
	struct button *btn_ok;		/* OK button */
	struct button *btn_cancel;	/* Cancel button */
};

__BEGIN_DECLS
struct AGFileDlg *file_dlg_new(void *, int, const char *);
void file_dlg_init(struct AGFileDlg *, int, const char *);
void file_dlg_scale(void *, int, int);
void file_dlg_destroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_FILE_DLG_H_ */
