/*	$Csoft: widget.h,v 1.61 2003/06/06 03:18:15 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_H_
#define _AGAR_WIDGET_H_

#include <config/floating_point.h>

#include "begin_code.h"

#define WIDGET_COLORS_MAX	16
#define WIDGET_TYPE_MAX		32

struct widget_ops {
	const struct object_ops	obops;
	void			(*draw)(void *);
	void			(*scale)(void *, int, int);
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
	size_t				 size;
	SLIST_ENTRY(widget_binding)	 bindings;
};

struct widget {
	struct object obj;

	char	 type[WIDGET_TYPE_MAX];
	int	 flags;
#define WIDGET_NO_FOCUS		  0x01	/* Cannot gain focus */
#define WIDGET_FOCUSED		  0x02	/* Holds focus (optimization) */
#define WIDGET_UNFOCUSED_MOTION	  0x04	/* Unfocused mousemotion events */
#define WIDGET_UNFOCUSED_BUTTONUP 0x08	/* Unfocused mousebuttonup events */
#define WIDGET_CLIPPING		  0x10	/* Enable clipping */
#define WIDGET_WFILL		  0x20	/* Expand to fill available width */
#define WIDGET_HFILL		  0x40	/* Expand to fill available height */

	int	 cx, cy;		/* Coordinates in view */
	int	 x, y;			/* Coordinates in parent */
	int	 w, h;			/* Allocated geometry */

	char				*color_names[WIDGET_COLORS_MAX];
	Uint32				 colors[WIDGET_COLORS_MAX];
	int				ncolors;
	SLIST_HEAD(, widget_binding)	 bindings;
	pthread_mutex_t			 bindings_lock;
};

#define WIDGET(wi)		((struct widget *)(wi))
#define WIDGET_OPS(ob)		((struct widget_ops *)OBJECT((ob))->ops)
#define WIDGET_COLOR(wi,ind)	WIDGET(wi)->colors[ind]

__BEGIN_DECLS
extern DECLSPEC void	 widget_init(void *, const char *, const void *, int);
extern DECLSPEC void	 widget_destroy(void *);
extern DECLSPEC void	 widget_draw(void *);
extern DECLSPEC int	 widget_load(void *, struct netbuf *);
extern DECLSPEC int	 widget_save(void *, struct netbuf *);

extern DECLSPEC void	 widget_set_type(void *, const char *);
extern DECLSPEC void	 widget_set_geometry(void *, int, int);
extern DECLSPEC void	 widget_focus(void *);
extern __inline__ int	 widget_holds_focus(void *);
extern struct widget	*widget_find_focus(void *);
extern __inline__ int	 widget_relative_area(void *, int, int);
extern __inline__ int	 widget_area(void *, int, int);
extern DECLSPEC void	 widget_map_color(void *, int, const char *, Uint8,
			                  Uint8, Uint8, Uint8);

extern __inline__ void	 widget_blit(void *, SDL_Surface *, int, int);
extern __inline__ void	 widget_put_pixel(void *, int, int, Uint32);

extern DECLSPEC void	widget_mousemotion(struct window *, struct widget *,
			                   int, int, int, int);
extern DECLSPEC void	widget_mousebuttonup(struct window *, struct widget *,
			                     int, int, int);
extern DECLSPEC int	widget_mousebuttondown(struct window *, struct widget *,
			                       int, int, int);

#define widget_binding_get(widp, name, res) \
	_widget_binding_get((widp), (name), (res), 0)
#define	widget_binding_get_locked(widp, name, res) \
	_widget_binding_get((widp), (name), (res), 1)

#define	widget_get_bool widget_get_int
#define	widget_set_bool widget_set_int

extern DECLSPEC struct widget_binding	*widget_bind(void *, const char *,
					             enum widget_binding_type,
						     ...);
extern DECLSPEC struct widget_binding	*_widget_binding_get(void *,
					                     const char *,
							     void *, int);

extern __inline__ void	 widget_binding_lock(struct widget_binding *);
extern __inline__ void	 widget_binding_unlock(struct widget_binding *);
extern __inline__ void	 widget_binding_modified(struct widget_binding *);

extern __inline__ int	 widget_get_int(void *, const char *);
extern __inline__ Uint8	 widget_get_uint8(void *, const char *);
extern __inline__ Sint8	 widget_get_sint8(void *, const char *);
extern __inline__ Uint16 widget_get_uint16(void *, const char *);
extern __inline__ Sint16 widget_get_sint16(void *, const char *);
extern __inline__ Uint32 widget_get_uint32(void *, const char *);
extern __inline__ Sint32 widget_get_sint32(void *, const char *);
#ifdef FLOATING_POINT
extern __inline__ float	 widget_get_float(void *, const char *);
extern __inline__ double widget_get_double(void *, const char *);
#endif
extern __inline__ char	*widget_get_string(void *, const char *);
extern __inline__ size_t widget_copy_string(void *, const char *, char *,
			                    size_t);
extern __inline__ void	*widget_get_pointer(void *, const char *);
extern __inline__ void	 widget_set_int(void *, const char *, int);
extern __inline__ void	 widget_set_uint8(void *, const char *, Uint8);
extern __inline__ void	 widget_set_sint8(void *, const char *, Sint8);
extern __inline__ void	 widget_set_uint16(void *, const char *, Uint16);
extern __inline__ void	 widget_set_sint16(void *, const char *, Sint16);
extern __inline__ void	 widget_set_uint32(void *, const char *, Uint32);
extern __inline__ void	 widget_set_sint32(void *, const char *, Sint32);
#ifdef FLOATING_POINT
extern __inline__ void	 widget_set_float(void *, const char *, float);
extern __inline__ void	 widget_set_double(void *, const char *, double);
#endif
extern __inline__ void	 widget_set_string(void *, const char *, char *);
extern __inline__ void	 widget_set_pointer(void *, const char *, void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_H_ */
