/*	$Csoft: statusbar.h,v 1.2 2004/03/28 05:55:38 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_STATUSBAR_H_
#define _AGAR_WIDGET_STATUSBAR_H_

#include <engine/widget/widget.h>
#include <engine/widget/box.h>
#include <engine/widget/label.h>

#include "begin_code.h"

#define STATUSBAR_MAX_LABELS 8

struct statusbar {
	struct box box;
	struct label *labels[STATUSBAR_MAX_LABELS];
	int nlabels;
};

__BEGIN_DECLS
struct statusbar *statusbar_new(void *);
void		  statusbar_init(struct statusbar *);
void		  statusbar_scale(void *, int, int);
void	 	  statusbar_destroy(void *);
struct label	 *statusbar_add_label(struct statusbar *, enum label_type,
		                      const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_STATUSBAR_H_ */
