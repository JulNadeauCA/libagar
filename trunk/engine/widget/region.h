/*	$Csoft: region.h,v 1.8 2003/01/23 02:00:57 vedge Exp $	*/
/*	Public domain	*/

TAILQ_HEAD(widgetsq, widget);

struct region {
	struct object	obj;

	/* Set on init */
	int	flags;
#define REGION_HALIGN	0x01	/* Align widgets horizontally */
#define REGION_VALIGN	0x02	/* Align widgets vertically */
#define REGION_CLIPPING	0x04	/* Set the clipping rectangle to the region
			 	   area before drawing */
#define REGION_RESIZING	0x10	/* Region is being resized */

	int	rx, ry;			/* Requested coordinates (%) */
	int	rw, rh;			/* Requested geometry (%) */
	int	x, y;			/* Allocated coordinates (pixels) */
	int	w, h;			/* Allocated geometry (pixels) */
	Uint8	xspacing, yspacing;	/* Spacing between widgets */

	/* Set on attach */
	struct window	*win;		/* Back pointer to parent window */
	struct widgetsq	 widgets;	/* Widgets in region */
	int		nwidgets;

	TAILQ_ENTRY(region) regions;	/* Regions in window */
};

#ifdef DEBUG
# define REGION_PUT_PIXEL(reg, rrx, rry, c) do {			\
	if ((rrx) > (reg)->w || (rry) > (reg)->h) {			\
		fatal("%s: %d,%d > %dx%d", OBJECT(reg)->name,		\
		    (rrx), (rry), (reg)->w, (reg)->h);			\
	}								\
	WINDOW_PUT_PIXEL((reg)->win, (reg)->x+(rrx), (reg)->y+(rry),	\
	    (c));							\
} while (/*CONSTCOND*/0)
# define REGION_PUT_ALPHAPIXEL(reg, rrx, rry, c, wa) do {		\
	if ((rrx) > (reg)->w || (rry) > (reg)->h) {			\
		fatal("%s: %d,%d > %dx%d", OBJECT(reg)->name,		\
		    (rrx), (rry), (reg)->w, (reg)->h);			\
	}								\
	WINDOW_PUT_ALPHAPIXEL((reg)->win, (reg)->x+(rrx),		\
	    (reg)->y+(rry), (c), (wa));					\
} while (/*CONSTCOND*/0)
#else
# define REGION_PUT_PIXEL(reg, rx, ry, c) \
	WINDOW_PUT_PIXEL((reg)->win, (rx)+(reg)->x, (ry)+(reg)->y, (c))
# define REGION_PUT_ALPHAPIXEL(reg, rx, ry, c, wa) \
	WINDOW_PUT_ALPHAPIXEL((reg)->win, (rx)+(reg)->x, (ry)+(reg)->y, \
	    (c), (wa))
#endif

struct region	*region_new(void *, int, int, int, int, int);
void		 region_init(struct region *, int, int, int, int, int);
void		 region_destroy(void *);
void		 region_attach(void *, void *);
void		 region_detach(void *, void *);
void		 region_set_spacing(struct region *, Uint8, Uint8);

