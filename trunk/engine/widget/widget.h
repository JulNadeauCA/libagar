/*	$Csoft: widget.h,v 1.53 2003/03/02 00:35:37 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_H_
#define _AGAR_WIDGET_H_

#include <config/floating_point.h>

#define WIDGET_MAX_COLORS	16

struct widget_ops {
	const struct object_ops	obops;

	/* Draw directly to video memory. */
	void	 (*widget_draw)(void *);

	/* Update cached surface. */
	void	 (*widget_update)(void *, SDL_Surface *);
};

struct widget_color {
	char	*name;		/* Identifier */
	int	ind;		/* Index into color array */
	SLIST_ENTRY(widget_color) colors;
};

enum widget_binding_type {
	WIDGET_NONE,
	WIDGET_BOOL,
	WIDGET_INT,
	WIDGET_UINT8,
	WIDGET_SINT8,
	WIDGET_UINT16,
	WIDGET_SINT16,
	WIDGET_UINT32,
	WIDGET_SINT32,
	WIDGET_FLOAT,
	WIDGET_DOUBLE,
	WIDGET_STRING,
	WIDGET_POINTER,
	WIDGET_PROP
};

struct widget_binding {
	enum widget_binding_type	 type;		/* Type of value */
	char				*name;		/* Identifier */
	pthread_mutex_t			*mutex;		/* Optional lock */
	void				*p1, *p2;
	SLIST_ENTRY(widget_binding)	 bindings;
};

struct window;
struct region;

/* Structure shares the parent window's lock. */
struct widget {
	struct object	obj;

	int		flags;
#define WIDGET_NO_FOCUS		  0x01	/* Cannot gain focus */
#define WIDGET_UNFOCUSED_MOTION	  0x02	/* Receive window-mousemotion events
					   even when the widget isn't focused */
#define WIDGET_UNFOCUSED_BUTTONUP 0x04	/* Receive window-mousebuttonup events
					   even when the widget isn't focused */
#define WIDGET_CLIPPING		  0x08	/* Set the clipping rectangle to the
					   widget area before drawing. */

	char		*type;		/* Widget type identifier */
	struct window	*win;		/* Parent window */
	struct region	*reg;		/* Parent region */

	int	 rw, rh;		/* Requested geometry (%) */
	int	 x, y;			/* Allocated coordinates in window */
	int	 w, h;			/* Allocated geometry */

	SLIST_HEAD(, widget_color) colors;		     /* Color scheme */
	Uint32			   color[WIDGET_MAX_COLORS]; /* Color array */
	int			  ncolors;		     /* Color count */

	SLIST_HEAD(, widget_binding)	 bindings;	/* Variable bindings */
	pthread_mutex_t			 bindings_lock;
	
	TAILQ_ENTRY(widget)		widgets;	/* Region */
};

#define WIDGET(wi)	((struct widget *)(wi))
#define WIDGET_OPS(ob)	((struct widget_ops *)OBJECT((ob))->ops)

#define WIDGET_COLOR(wi,ind)	WIDGET(wi)->color[ind]

/* Expand to absolute widget coordinates. */
#define WIDGET_ABSX(wi)	((WIDGET((wi))->win->rd.x) + WIDGET((wi))->x)
#define WIDGET_ABSY(wi)	((WIDGET((wi))->win->rd.y) + WIDGET((wi))->y)

#define WIDGET_PUT_PIXEL(wid, wdrx, wdry, c)				\
	 WINDOW_PUT_PIXEL(WIDGET((wid))->win,				\
	     WIDGET((wid))->x+(wdrx), WIDGET((wid))->y+(wdry), (c))

#define WIDGET_PUT_ALPHAPIXEL(wid, wdrx, wdry, c, wa)			\
	 WINDOW_PUT_ALPHAPIXEL(WIDGET((wid))->win,			\
	     WIDGET((wid))->x+(wdrx), WIDGET((wid))->y+(wdry), (c), (wa))

#define WIDGET_FOCUSED(wid)	(WIDGET((wid))->win->focus == WIDGET((wid)))
#define WIDGET_FOCUS(wid)					\
	if ((WIDGET((wid))->flags & WIDGET_NO_FOCUS) == 0) {	\
		WIDGET((wid))->win->focus = WIDGET((wid));	\
		event_post((wid), "widget-gainfocus", NULL);	\
	}

#define WIDGET_INSIDE(wida, xa, ya)					\
    ((xa) > WIDGET_ABSX((wida))	&& (ya) > WIDGET_ABSY((wida)) &&	\
     (xa) < (WIDGET_ABSX((wida)) + WIDGET((wida))->w) &&		\
     (ya) < (WIDGET_ABSY((wida)) + WIDGET((wida))->h))

#define WIDGET_INSIDE_RELATIVE(wida, xa, ya)			\
    ((xa) >= 0 && (ya) >= 0 &&					\
     (xa) <= WIDGET((wida))->w && (ya) <= WIDGET((wida))->h)

void	widget_init(struct widget *, char *, const void *, int, int);
void	widget_destroy(void *);
void	widget_set_parent(void *, void *);
void	widget_set_position(void *, Sint16, Sint16);
void	widget_set_geometry(void *, Uint16, Uint16);
void	widget_get_position(void *, Sint16 *, Sint16 *);
void	widget_get_geometry(void *, Uint16 *, Uint16 *);
void	widget_map_color(void *, int, char *, Uint8, Uint8, Uint8);

struct widget_binding	*widget_bind(void *, const char *,
			     enum widget_binding_type, ...);
struct widget_binding	*_widget_binding_get(void *, const char *, void *, int);
#define			  widget_binding_get(widp, name, res) \
			 _widget_binding_get((widp), (name), (res), 0)
#define			  widget_binding_get_locked(widp, name, res) \
			 _widget_binding_get((widp), (name), (res), 1)

void	widget_binding_lock(struct widget_binding *);
void	widget_binding_modified(struct widget_binding *);
void	widget_binding_unlock(struct widget_binding *);

int	 widget_get_int(void *, const char *);
#define	 widget_get_bool widget_get_int
Uint8	 widget_get_uint8(void *, const char *);
Sint8	 widget_get_sint8(void *, const char *);
Uint16	 widget_get_uint16(void *, const char *);
Sint16	 widget_get_sint16(void *, const char *);
Uint32	 widget_get_uint32(void *, const char *);
Sint32	 widget_get_sint32(void *, const char *);
#ifdef FLOATING_POINT
float	 widget_get_float(void *, const char *);
double	 widget_get_double(void *, const char *);
#endif
char	*widget_get_string(void *, const char *);
void	*widget_get_pointer(void *, const char *);

void	 widget_set_int(void *, const char *, int);
#define	 widget_set_bool widget_set_int
void	 widget_set_uint8(void *, const char *, Uint8);
void	 widget_set_sint8(void *, const char *, Sint8);
void	 widget_set_uint16(void *, const char *, Uint16);
void	 widget_set_sint16(void *, const char *, Sint16);
void	 widget_set_uint32(void *, const char *, Uint32);
void	 widget_set_sint32(void *, const char *, Sint32);
#ifdef FLOATING_POINT
void	 widget_set_float(void *, const char *, float);
void	 widget_set_double(void *, const char *, double);
#endif
void	 widget_set_string(void *, const char *, char *);
void	 widget_set_pointer(void *, const char *, void *);

void	 widget_blit(void *, SDL_Surface *, int, int);

#endif /* _AGAR_WIDGET_H_ */
