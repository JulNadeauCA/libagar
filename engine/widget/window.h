/*	$Csoft: window.h,v 1.67 2003/06/06 03:18:15 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_WINDOW_H_
#define _AGAR_WIDGET_WINDOW_H_

#include <engine/widget/widget.h>
#include <engine/widget/titlebar.h>

#include "begin_code.h"

enum window_close_mode {
	WINDOW_HIDE,		/* Hide window on close */
	WINDOW_DETACH		/* Detach window on close */
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
#define WINDOW_SAVE_POSITION	0x01	/* Save position/geometry on close */
#define WINDOW_CASCADE		0x02	/* Increment position slightly */

	int	 visible;		/* Window is visible */
	Uint32	*border;		/* Border colors */
	int	 borderw;

	pthread_mutex_t		 lock;
	struct titlebar		*tbar;		/* Titlebar (or NULL) */
	enum window_alignment	 alignment;	/* Initial position */
	int			 spacing;	/* Widget spacing */
	int			 padding;	/* Widget padding */

	int	 minw, minh;			/* Minimum geometry */
	int	 saved_w, saved_h;		/* Saved geometry */

	TAILQ_ENTRY(window)	 windows;	/* Active windows */
	TAILQ_ENTRY(window)	 detach;	/* Zombie windows */
};

#define WINDOW_FOCUSED(w)	(TAILQ_LAST(&view->windows, windowq) == (w))

__BEGIN_DECLS
extern DECLSPEC struct window	*window_new(const char *, ...);

extern DECLSPEC void	 window_init(void *, const char *);
extern DECLSPEC int	 window_load(void *, struct netbuf *);
extern DECLSPEC int	 window_save(void *, struct netbuf *);
extern DECLSPEC void	 window_destroy(void *);

extern DECLSPEC void	 window_draw(void *);
extern DECLSPEC void	 window_scale(void *, int, int);

extern DECLSPEC void	 window_attach(void *, void *);
extern DECLSPEC void	 window_detach(void *, void *);
extern DECLSPEC void	 window_show(struct window *);
extern DECLSPEC void	 window_hide(struct window *);
extern DECLSPEC int	 window_toggle_visibility(struct window *);
extern DECLSPEC int	 window_event(SDL_Event *);
extern DECLSPEC void	 window_resize(struct window *);
extern DECLSPEC void	 window_remap_widgets(void *, int, int);

extern DECLSPEC void	 window_set_caption(struct window *, const char *, ...);
extern DECLSPEC void	 window_set_spacing(struct window *, int);
extern DECLSPEC void	 window_set_padding(struct window *, int);
extern DECLSPEC void	 window_set_position(struct window *,
			                     enum window_alignment, int);
extern DECLSPEC void	 window_set_closure(struct window *,
			                    enum window_close_mode);

extern DECLSPEC void	 window_generic_detach(int, union evarg *);
extern DECLSPEC void	 window_generic_hide(int, union evarg *);
extern DECLSPEC void	 window_generic_show(int, union evarg *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_WINDOW_H_ */
