/*	$Csoft: region.h,v 1.2 2002/05/19 15:27:56 vedge Exp $	*/

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
	int	spacing;		/* Spacing between widgets */

	/* Set on attach */
	struct	window *win;		/* Back pointer to window */
	TAILQ_HEAD(, widget) widgetsh;	/* Child widgets */
	SLIST_ENTRY(region) regions;	/* Regions in window */
};

#ifdef DEBUG
# define REGION_PUT_PIXEL(reg, rrx, rry, c) do {			\
	if ((rrx) > (reg)->w || (rry) > (reg)->h) {			\
		fatal("%s: %d,%d > %dx%d\n", OBJECT(reg)->name,		\
		    (rrx), (rry), (reg)->w, (reg)->h);			\
	}								\
	WINDOW_PUT_PIXEL((reg)->win, (reg)->x+(rrx), (reg)->y+(rry), (c)); \
} while (/*CONSTCOND*/0)
#else
# define REGION_PUT_PIXEL(reg, rx, ry, c) \
	WINDOW_PUT_PIXEL((reg)->win, (rx)+(reg)->x, (ry)+(reg)->y, (c))
#endif

struct region	*region_new(void *, int, int, int, int, int);

void	 region_init(struct region *, int, int, int, int, int);
void	 region_destroy(void *);
void	 region_attach(void *, void *);
void	 region_detach(void *, void *);

