/*	$Csoft: graph.h,v 1.10 2003/04/25 09:47:10 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_GRAPH_H_
#define _AGAR_WIDGET_GRAPH_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define GRAPH_CAPTION_MAX	32
#define GRAPH_ITEM_NAME_MAX	32

struct graph;

struct graph_item {
	char	 name[GRAPH_ITEM_NAME_MAX];	/* Description */
	Uint32	 color;				/* Line color */
	Sint32	*vals;				/* Value array */
	Uint32	 nvals;
	Uint32	 maxvals;

	struct graph		*graph;		/* Back pointer */
	TAILQ_ENTRY(graph_item)	 items;
};

TAILQ_HEAD(itemq, graph_item);

enum graph_type {
	GRAPH_POINTS,
	GRAPH_LINES
};

struct graph {
	struct widget	 wid;
	enum graph_type	 type;
	char		 caption[GRAPH_CAPTION_MAX];
	int		 flags;
#define GRAPH_SCROLL	0x01	/* Scroll if the end is not visible */
#define GRAPH_ORIGIN	0x02	/* Visible origin */

	Sint32		 yrange;	/* Max. value */
	Sint32		 xoffs;		/* Display offset */
	int		 xinc;		/* X increment */
	int		 origin_y;	/* Origin position (%) */
	struct itemq	 items;		/* Lists of values */
};

__BEGIN_DECLS
extern DECLSPEC struct graph	  *graph_new(void *, const char *,
				             enum graph_type, int, Sint32);
extern DECLSPEC struct graph_item *graph_add_item(struct graph *, const char *,
				                  Uint8, Uint8, Uint8);

extern DECLSPEC void	 graph_init(struct graph *, const char *,
			            enum graph_type, int, Sint32);
extern DECLSPEC void	 graph_destroy(void *);
extern DECLSPEC int	 graph_load(void *, struct netbuf *);
extern DECLSPEC int	 graph_save(void *, struct netbuf *);
extern DECLSPEC void	 graph_draw(void *);
extern DECLSPEC void	 graph_scale(void *, int, int);

extern DECLSPEC void	 graph_plot(struct graph_item *, Sint32);
extern __inline__ void	 graph_scroll(struct graph *, int);
extern DECLSPEC void	 graph_free_items(struct graph *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_GRAPH_H_ */
