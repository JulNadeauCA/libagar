/*	$Csoft: window.h,v 1.89 2005/04/02 04:30:26 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_WINDOW_H_
#define _AGAR_WIDGET_WINDOW_H_

#include <engine/widget/widget.h>
#include <engine/widget/titlebar.h>

#include "begin_code.h"

enum window_alignment {
	WINDOW_UPPER_LEFT,
	WINDOW_MIDDLE_LEFT,
	WINDOW_LOWER_LEFT,
	WINDOW_UPPER_RIGHT,
	WINDOW_MIDDLE_RIGHT,
	WINDOW_LOWER_RIGHT,
	WINDOW_CENTER,
	WINDOW_LOWER_CENTER,
	WINDOW_UPPER_CENTER
};

struct widget;

struct window {
	struct widget wid;

	int flags;
#define WINDOW_PERSISTENT	0x0001	/* Persistent position/geometry
				 	   (for named windows only) */
#define WINDOW_CASCADE		0x0002	/* Increment position slightly */
#define WINDOW_NO_TITLEBAR	0x0004	/* Disable the titlebar */
#define WINDOW_NO_DECORATIONS	0x0008	/* Disable the window borders */
#define WINDOW_HIDE		0x0010	/* Hide on window-close */
#define WINDOW_DETACH		0x0020	/* Detach on window-close */
#define WINDOW_MAXIMIZED	0x0040	/* Window is maximized */
#define WINDOW_ICONIFIED	0x0080	/* Window is minimized */
#define WINDOW_NO_HRESIZE	0x0100	/* Disable horizontal resize */
#define WINDOW_NO_VRESIZE	0x0200	/* Disable vertical resize */
#define WINDOW_NO_RESIZE	(WINDOW_NO_HRESIZE|WINDOW_NO_VRESIZE)
#define WINDOW_NO_CLOSE		0x0400	/* Disable close button */
#define WINDOW_NO_MINIMIZE	0x0800	/* Disable minimize button */
#define WINDOW_NO_MAXIMIZE	0x1000	/* Disable maximize button */
#define WINDOW_MODAL		0x2000	/* Modal window behavior */
#define WINDOW_INHIBIT_FOCUS	0x4000	/* Prevent widgets from gaining focus */

	char caption[128];
	int visible;				/* Window is visible */

	pthread_mutex_t lock;
	struct titlebar	*tbar;			/* Titlebar (if any) */
	enum window_alignment alignment;	/* Initial position */
	int spacing;				/* Default spacing (px) */
	int xpadding;				/* Window padding (px) */
	int ypadding_top, ypadding_bot;
	int minw, minh;				/* Minimum geometry (px) */
	int savx, savy;				/* Saved coordinates (px) */
	int savw, savh;				/* Saved geometry (px) */
	
	TAILQ_HEAD(,window) subwins;		/* Sub-windows */
	TAILQ_ENTRY(window) windows;		/* Active window list */
	TAILQ_ENTRY(window) swins;		/* Sub-window list */
	TAILQ_ENTRY(window) detach;		/* Zombie window list */
};

#define WINDOW_FOCUSED(w)	(TAILQ_LAST(&view->windows, windowq) == (w))
#define WINDOW_UPDATE(win) \
	do { \
		WIDGET_OPS(win)->scale((win), WIDGET(win)->w, WIDGET(win)->h); \
		widget_update_coords((win), WIDGET(win)->x, WIDGET(win)->y); \
	} while (/*CONSTCOND*/0)

__BEGIN_DECLS
struct window	*window_new(int, const char *, ...)
		    FORMAT_ATTRIBUTE(printf, 2, 3);

void	 window_init(void *, const char *, int);
int	 window_load(void *, struct netbuf *);
int	 window_save(void *, struct netbuf *);
void	 window_destroy(void *);
void	 window_draw(void *);
void	 window_scale(void *, int, int);

void	 window_set_caption(struct window *, const char *, ...)
	     FORMAT_ATTRIBUTE(printf, 2, 3)
	     NONNULL_ATTRIBUTE(2);
void	 window_set_spacing(struct window *, int);
void	 window_set_padding(struct window *, int, int, int);
void	 window_set_position(struct window *, enum window_alignment, int);
void	 window_set_closure(struct window *, int);
void	 window_set_style(struct window *, const struct style *);
void	 window_set_geometry(struct window *, int, int, int, int);

void	 window_attach(struct window *, struct window *);
void	 window_detach(struct window *, struct window *);
void	 window_show(struct window *);
void	 window_hide(struct window *);
int	 window_toggle_visibility(struct window *);
int	 window_event(SDL_Event *);
void	 window_resize(struct window *);
void	 window_focus(struct window *);
void	 window_remove_titlebar(struct window *);
void	 window_clamp(struct window *);

void	 window_generic_detach(int, union evarg *);
void	 window_generic_hide(int, union evarg *);
void	 window_generic_show(int, union evarg *);

int	 window_surrounded(struct window *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_WINDOW_H_ */
