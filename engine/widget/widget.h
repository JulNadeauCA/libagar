/*	$Csoft: widget.h,v 1.76 2004/03/31 02:30:04 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_H_
#define _AGAR_WIDGET_H_

#include "begin_code.h"

#define WIDGET_COLORS_MAX	16
#define WIDGET_COLOR_NAME_MAX	16
#define WIDGET_TYPE_MAX		32
#define WIDGET_BINDING_NAME_MAX	16

struct widget_ops {
	const struct object_ops	obops;
	void			(*draw)(void *);
	void			(*scale)(void *, int, int);
};

enum widget_binding_type {
	WIDGET_NONE,
	WIDGET_BOOL,
	WIDGET_UINT,
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
	char	name[WIDGET_BINDING_NAME_MAX];
	int	type;
	int	vtype;

	pthread_mutex_t	*mutex;	
	void		*p1, *p2;
	size_t		 size;

	SLIST_ENTRY(widget_binding) bindings;
};

struct widget {
	struct object obj;

	char	 type[WIDGET_TYPE_MAX];
	int	 flags;
#define WIDGET_FOCUSABLE	  0x01	/* Can grab focus */
#define WIDGET_FOCUSED		  0x02	/* Holds focus (optimization) */
#define WIDGET_UNFOCUSED_MOTION	  0x04	/* Unfocused mousemotion events */
#define WIDGET_UNFOCUSED_BUTTONUP 0x08	/* Unfocused mousebuttonup events */
#define WIDGET_CLIPPING		  0x10	/* Enable clipping */
#define WIDGET_WFILL		  0x20	/* Expand to fill available width */
#define WIDGET_HFILL		  0x40	/* Expand to fill available height */
#define WIDGET_EXCEDENT		  0x80	/* Used internally for scale */

	int	 cx, cy;		/* Coordinates in view */
	int	 x, y;			/* Coordinates in parent */
	int	 w, h;			/* Allocated geometry */

	char	 color_names[WIDGET_COLORS_MAX][WIDGET_COLOR_NAME_MAX];
	Uint32	 colors[WIDGET_COLORS_MAX];
	int	ncolors;

	pthread_mutex_t			 bindings_lock;
	SLIST_HEAD(, widget_binding)	 bindings;	    /* Bound values */
};

#define WIDGET(wi)		((struct widget *)(wi))
#define WIDGET_COLOR(wi, ind)	WIDGET(wi)->colors[ind]
#define WIDGET_OPS(ob)		((struct widget_ops *)OBJECT(ob)->ops)
#define WIDGET_SCALE(wi, w, h)	WIDGET_OPS(wi)->scale((wi), (w), (h))

__BEGIN_DECLS
void	 widget_init(void *, const char *, const void *, int);
void	 widget_destroy(void *);
void	 widget_draw(void *);
void	 widget_scale(void *, int, int);

void		 widget_set_type(void *, const char *);
void		 widget_focus(void *);
struct widget	*widget_find_focus(void *);
__inline__ int	 widget_holds_focus(void *);
__inline__ int	 widget_relative_area(void *, int, int);
__inline__ int	 widget_area(void *, int, int);
void		 widget_update_coords(void *, int, int);

void		 widget_map_color(void *, int, const char *, Uint8, Uint8,
		                  Uint8, Uint8);
__inline__ int   widget_push_color(struct widget *, Uint32);
__inline__ void  widget_pop_color(struct widget *);

__inline__ void	 widget_blit(void *, SDL_Surface *, int, int);
__inline__ void	 widget_put_pixel(void *, int, int, Uint32);

void  widget_mousemotion(struct window *, struct widget *, int, int, int, int,
	                 int);
void  widget_mousebuttonup(struct window *, struct widget *, int, int, int);
int   widget_mousebuttondown(struct window *, struct widget *, int, int, int);

struct widget_binding	*widget_bind(void *, const char *,
			             enum widget_binding_type, ...);
struct widget_binding	*widget_bind_protected(void *, const char *,
			                       pthread_mutex_t *,
					       enum widget_binding_type, ...);
struct widget_binding	*widget_get_binding(void *, const char *, ...);
__inline__ void		 widget_binding_lock(struct widget_binding *);
__inline__ void		 widget_binding_unlock(struct widget_binding *);
__inline__ void		 widget_binding_modified(struct widget_binding *);

__inline__ unsigned int	 widget_get_uint(void *, const char *);
__inline__ int		 widget_get_int(void *, const char *);
#define			 widget_get_bool widget_get_int
__inline__ Uint8	 widget_get_uint8(void *, const char *);
__inline__ Sint8	 widget_get_sint8(void *, const char *);
__inline__ Uint16	 widget_get_uint16(void *, const char *);
__inline__ Sint16	 widget_get_sint16(void *, const char *);
__inline__ Uint32	 widget_get_uint32(void *, const char *);
__inline__ Sint32	 widget_get_sint32(void *, const char *);
__inline__ float	 widget_get_float(void *, const char *);
__inline__ double	 widget_get_double(void *, const char *);

__inline__ void	  *widget_get_pointer(void *, const char *);
__inline__ char	  *widget_get_string(void *, const char *);
__inline__ size_t  widget_copy_string(void *, const char *, char *, size_t)
		       BOUNDED_ATTRIBUTE(__string__, 3, 4);

__inline__ void	 widget_set_uint(void *, const char *, unsigned int);
__inline__ void	 widget_set_int(void *, const char *, int);
#define		 widget_set_bool widget_set_int
__inline__ void	 widget_set_uint8(void *, const char *, Uint8);
__inline__ void	 widget_set_sint8(void *, const char *, Sint8);
__inline__ void	 widget_set_uint16(void *, const char *, Uint16);
__inline__ void	 widget_set_sint16(void *, const char *, Sint16);
__inline__ void	 widget_set_uint32(void *, const char *, Uint32);
__inline__ void	 widget_set_sint32(void *, const char *, Sint32);
__inline__ void	 widget_set_float(void *, const char *, float);
__inline__ void	 widget_set_double(void *, const char *, double);
__inline__ void	 widget_set_string(void *, const char *, const char *);
__inline__ void	 widget_set_pointer(void *, const char *, void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_H_ */
