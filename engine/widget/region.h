/*	$Csoft: region.h,v 1.1 2002/05/19 14:30:24 vedge Exp $	*/

enum widget_align {
	WIDGET_HALIGN,		/* Align widgets horizontally */
	WIDGET_VALIGN		/* Align widgets vertically */
};

/* Widget container, attach to windows. */
struct region {
	struct	object obj;

	/* Set on init */
	enum	widget_align walign;	/* Widget alignment policy */
	int	rx, ry;			/* Requested coordinates (%) */
	int	rw, rh;			/* Requested geometry (%) */
	int	x, y;			/* Allocated coordinates (pixels) */
	int	w, h;			/* Allocated geometry (pixels) */
	int	spacing;		/* Spacing between widgets */

	/* Set on attach */
	struct	window *win;		/* Back pointer to window */
	TAILQ_HEAD(, widget) widgetsh;	/* Child widgets */
	SLIST_ENTRY(region) regions;	/* Regions in window */
};

struct region	*region_new(void *, enum widget_align, int, int, int, int, int);
void		 region_init(struct region *, enum widget_align, int, int, int,
		     int, int);
void		 region_destroy(void *);
void		 region_attach(void *, void *);
void		 region_detach(void *, void *);

