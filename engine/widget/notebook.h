/*	$Csoft: notebook.h,v 1.4 2005/01/08 03:35:55 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_NOTEBOOK_H_
#define _AGAR_WIDGET_NOTEBOOK_H_

#include <engine/widget/widget.h>
#include <engine/widget/box.h>

#include "begin_code.h"

enum notebook_tab_alignment {
	NOTEBOOK_TABS_TOP,
	NOTEBOOK_TABS_BOTTOM,
	NOTEBOOK_TABS_LEFT,
	NOTEBOOK_TABS_RIGHT
};

struct notebook_tab {
	struct box box;
	int label;
	TAILQ_ENTRY(notebook_tab) tabs;
};

struct notebook {
	struct widget wid;
	enum notebook_tab_alignment tab_align;
	int flags;
#define NOTEBOOK_WFILL	0x01		/* Expand to fill available width */
#define NOTEBOOK_HFILL	0x02		/* Expand to fill available height */
	pthread_mutex_t	lock;
	int bar_w, bar_h;		/* Dimensions of tab button bar */
	int cont_w, cont_h;		/* Dimensions of largest container */
	int tab_rad;			/* Radius for chamfered tab edges */
	struct notebook_tab *sel_tab;
	TAILQ_HEAD(,notebook_tab) tabs;
};

__BEGIN_DECLS
struct notebook	*notebook_new(void *, int);
void notebook_init(struct notebook *, int);
void notebook_destroy(void *);
void notebook_draw(void *);
void notebook_scale(void *, int, int);
void notebook_set_color(struct notebook *, Uint8, Uint8, Uint8);
void notebook_set_tab_alignment(struct notebook *, enum notebook_tab_alignment);

struct notebook_tab *notebook_add_tab(struct notebook *, const char *,
		                      enum box_type);
void notebook_del_tab(struct notebook *, struct notebook_tab *);
void notebook_select_tab(struct notebook *, struct notebook_tab *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_NOTEBOOK_H_ */
