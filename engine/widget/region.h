/*	$Csoft: region.h,v 1.10 2003/03/23 04:54:32 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_REGION_H_
#define _AGAR_WIDGET_REGION_H_

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

#define REGION_PUT_PIXEL(reg, rx, ry, c) \
	WINDOW_PUT_PIXEL((reg)->win, (rx)+(reg)->x, (ry)+(reg)->y, (c))
#define REGION_PUT_ALPHAPIXEL(reg, rx, ry, c, wa) \
	WINDOW_PUT_ALPHAPIXEL((reg)->win, (rx)+(reg)->x, (ry)+(reg)->y, \
	    (c), (wa))

struct region	*region_new(void *, int, int, int, int, int);
void		 region_init(struct region *, int, int, int, int, int);
void		 region_destroy(void *);
void		 region_attach(void *, void *);
void		 region_detach(void *, void *);
void		 region_set_spacing(struct region *, Uint8, Uint8);

#endif /* _AGAR_WIDGET_REGION_H_ */
