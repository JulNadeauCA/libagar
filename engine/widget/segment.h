/*	$Csoft$	*/

enum region_walign {
	WIDGET_HALIGN,		/* From left to right */
	WIDGET_VALIGN		/* From top to bottom */
};

/* Widget container, attach to windows. */
struct region {
	struct	object obj;

	enum	region_walign walign;	/* Widget alignment policy */

	int	reqpct;			/* Requested % of parent */
	int	x, y;			/* Allocated coordinates */
	int	w, h;			/* Allocated geometry */
	int	lastx, lasty;

	/* Inconsistent until attached. */
	struct	window *win;			/* Back pointer to window */
	SLIST_HEAD(, segchild) childsh;		/* Child widgets/segments */
	SLIST_ENTRY(segment) window_segs;	/* Segments in window */
};

struct segment	*segment_new(void *, enum segment_salign, int,
		     enum segment_walign, int);
void		 segment_init(struct segment *, enum segment_salign, int,
		     enum segment_walign, int);
void		 segment_destroy(void *);
void		 segment_attach(void *, void *);
void		 segment_detach(void *, void *);

