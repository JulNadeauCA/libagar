/*	$Csoft: window.h,v 1.74 2003/11/15 03:53:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_WINDOW_H_
#define _AGAR_WIDGET_WINDOW_H_

#include <engine/widget/widget.h>
#include <engine/widget/titlebar.h>

#include "begin_code.h"

enum window_close_mode {
	WINDOW_HIDE,		/* Hide window on close */
	WINDOW_DETACH,		/* Detach window on close */
	WINDOW_IGNORE		/* Ignore the titlebar close button */
};

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

	int	 flags;
#define WINDOW_PERSISTENT	0x01	/* Persistent position/geometry */
#define WINDOW_CASCADE		0x02	/* Increment position slightly */

#ifdef DEBUG
	char	 caption[128];
#endif
	int	 visible;		/* Window is visible */
	Uint32	*border;		/* Border colors */
	int	 borderw;

	pthread_mutex_t		 lock;
	struct titlebar		*tbar;		/* Titlebar (or NULL) */
	enum window_alignment	 alignment;	/* Initial position */
	int			 spacing;	/* Widget spacing */
	int			 padding;	/* Widget padding */
	int	 		 minw, minh;	/* Minimum geometry */
	
	TAILQ_HEAD(,window)	 subwins;	/* Sub-windows */
	TAILQ_ENTRY(window)	 windows;	/* Active window list */
	TAILQ_ENTRY(window)	 swins;		/* Sub-window list */
	TAILQ_ENTRY(window)	 detach;	/* Zombie window list */
};

#define WINDOW_FOCUSED(w)	(TAILQ_LAST(&view->windows, windowq) == (w))

__BEGIN_DECLS
struct window	*window_new(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2);

void	 window_init(void *, const char *);
int	 window_load(void *, struct netbuf *);
int	 window_save(void *, struct netbuf *);
void	 window_destroy(void *);
void	 window_draw(void *);
void	 window_scale(void *, int, int);

void	 window_set_caption(struct window *, const char *, ...)
	     FORMAT_ATTRIBUTE(printf, 2, 3)
	     NONNULL_ATTRIBUTE(2);
void	 window_set_spacing(struct window *, int);
void	 window_set_padding(struct window *, int);
void	 window_set_position(struct window *, enum window_alignment, int);
void	 window_set_closure(struct window *, enum window_close_mode);

void	 window_attach(struct window *, struct window *);
void	 window_detach(struct window *, struct window *);
void	 window_show(struct window *);
void	 window_hide(struct window *);
int	 window_toggle_visibility(struct window *);
int	 window_event(SDL_Event *);
void	 window_resize(struct window *);

void	 window_generic_detach(int, union evarg *);
void	 window_generic_hide(int, union evarg *);
void	 window_generic_show(int, union evarg *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_WINDOW_H_ */
