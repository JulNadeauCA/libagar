/*	$Csoft: window.h,v 1.63 2003/04/25 09:47:10 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_WINDOW_H_
#define _AGAR_WIDGET_WINDOW_H_

#include <engine/widget/region.h>

#include "begin_code.h"

#define WINDOW_CAPTION_MAX	96

enum window_event {
	WINDOW_MOUSEBUTTONUP,
	WINDOW_MOUSEBUTTONDOWN,
	WINDOW_KEYUP,
	WINDOW_KEYDOWN,
	WINDOW_MOUSEMOTION,
	WINDOW_MOUSEOUT
};
	
TAILQ_HEAD(regionsq, region);

struct widget;

struct window {
	struct widget	wid;		/* For primitives and color schemes */

	int		flags;
#define WINDOW_TITLEBAR		0x01	/* Draw a title bar */
#define WINDOW_SCALE		0x02	/* Scale the initial geometry (%) */
#define WINDOW_CENTER		0x04	/* Center the initial position */
#define WINDOW_SHOWN		0x10	/* Window is visible */
#define WINDOW_SAVE_POSITION	0x20	/* Save position/geometry on close */
#define WINDOW_HIDDEN_BODY	0x40	/* Draw the titlebar only */
#define WINDOW_PERSISTENT	WINDOW_HIDDEN_BODY

	enum {
		WINDOW_NO_BUTTON,
		WINDOW_CLOSE_BUTTON,
		WINDOW_HIDE_BUTTON
	} clicked_button;

	Uint32	*border;		/* Border colors */
	int	 borderw;		/* Border width */
	int	 titleh;		/* Titlebar height */
	int	 minw, minh;		/* Minimum window geometry */
	int	 xspacing, yspacing;	/* Spacing between regions */
	SDL_Rect body;			/* Area reserved for regions */

	pthread_mutex_t	 lock;
	SDL_Rect	 rd;				/* Current geometry */
	SDL_Rect	 saved_rd;			/* Original geometry */
	char		 caption[WINDOW_CAPTION_MAX];	/* Titlebar text */

	struct widget		*focus;		/* Focused widget */
	struct regionsq		 regionsh;	/* Regions */
	TAILQ_ENTRY(window)	 windows;	/* Windows in view */
	TAILQ_ENTRY(window)	 detach;	/* Windows to free */
};

#define WINDOW(w)	((struct window *)(w))

#define WINDOW_PUT_PIXEL(win, wrx, wry, c)			\
	VIEW_PUT_PIXEL(view->v, (win)->rd.x+(wrx),		\
	    (win)->rd.y+(wry), (c))

#define WINDOW_PUT_ALPHAPIXEL(win, wrx, wry, c, wa)		\
	VIEW_PUT_ALPHAPIXEL(view->v, (win)->rd.x+(wrx),		\
	    (win)->rd.y+(wry), (c), (wa))

#define WINDOW_INSIDE(win, xa, ya)				\
	((xa) > (win)->rd.x && (ya) > (win)->rd.y &&		\
	 (xa) < ((win)->rd.x+(win)->rd.w) && 			\
	 (ya) < ((win)->rd.y+(win)->rd.h))

#define WINDOW_FOCUSED(w)	(TAILQ_LAST(&view->windows, windowq) == (w))

__BEGIN_DECLS
extern DECLSPEC struct window	*window_new(char *, int, int, int, int, int,
				            int, int);
extern DECLSPEC struct window	*window_generic_new(int, int, const char *,
				                    ...);
extern DECLSPEC void	 	 window_init(struct window *, char *, int, int,
				             int, int, int, int, int);

extern DECLSPEC int	 window_load(void *, struct netbuf *);
extern DECLSPEC int	 window_save(void *, struct netbuf *);
extern DECLSPEC void	 window_destroy(void *);
extern DECLSPEC void	 window_attach(void *, void *);
extern DECLSPEC void	 window_detach(void *, void *);

extern DECLSPEC int	 window_show(struct window *);
extern DECLSPEC int	 window_hide(struct window *);
extern DECLSPEC void	 window_draw(struct window *);
extern DECLSPEC int	 window_event(SDL_Event *);
extern DECLSPEC void	 window_resize(struct window *);
extern DECLSPEC void	 window_set_caption(struct window *, const char *, ...);
extern DECLSPEC void	 window_set_spacing(struct window *, Uint8, Uint8);
extern DECLSPEC void	 window_set_geo(struct window *, Uint16, Uint16);
extern DECLSPEC void	 window_set_position(struct window *, Sint16, Sint16);
extern DECLSPEC void	 window_set_min_geo(struct window *, Uint16, Uint16);
extern DECLSPEC void	 window_set_titleh(struct window *, Uint8);

extern DECLSPEC void	 window_generic_detach(int, union evarg *);
extern DECLSPEC void	 window_generic_hide(int, union evarg *);
extern DECLSPEC void	 window_generic_show(int, union evarg *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_WINDOW_H_ */
