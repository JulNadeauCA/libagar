/*	$Csoft: region.h,v 1.4 2002/05/24 09:15:31 vedge Exp $	*/

TAILQ_HEAD(widgetsq, widget);

/* Widget container, attach to windows. */
struct region {
	struct	object obj;

	/* Set on init */
	int	flags;
#define REGION_HALIGN	0x01	/* Align widgets horizontally */
#define REGION_VALIGN	0x02	/* Align widgets vertically */
#define REGION_LEFT	0x04	/* Left/top justify widgets */
#define REGION_CENTER	0x08	/* Center widgets */
#define REGION_RIGHT	0x10	/* Right/down justify widgets */
#define REGION_BORDER	0x20	/* Draw region decorations */

	int	rx, ry;			/* Requested coordinates (%) */
	int	rw, rh;			/* Requested geometry (%) */
	int	x, y;			/* Allocated coordinates (pixels) */
	int	w, h;			/* Allocated geometry (pixels) */
	int	spacing;		/* Spacing factor between widgets */

	/* Set on attach */
	struct	window *win;		/* Back pointer to window */
	struct	widgetsq widgetsh;	/* Child widgets */
	TAILQ_ENTRY(region) regions;	/* Regions in window */
};

#ifdef DEBUG
# define REGION_PUT_PIXEL(reg, rrx, rry, c) do {			\
	if ((rrx) > (reg)->w || (rry) > (reg)->h) {			\
		fatal("%s: %d,%d > %dx%d\n", OBJECT(reg)->name,		\
		    (rrx), (rry), (reg)->w, (reg)->h);			\
	}								\
	WINDOW_PUT_PIXEL((reg)->win, (reg)->x+(rrx), (reg)->y+(rry),	\
	    (c));							\
} while (/*CONSTCOND*/0)
# define REGION_PUT_ALPHAPIXEL(reg, rrx, rry, c, wa) do {		\
	if ((rrx) > (reg)->w || (rry) > (reg)->h) {			\
		fatal("%s: %d,%d > %dx%d\n", OBJECT(reg)->name,		\
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

void	 region_init(struct region *, int, int, int, int, int);
void	 region_destroy(void *);
void	 region_attach(void *, void *);
void	 region_detach(void *, void *);

