/*	$Csoft: graph.h,v 1.12 2003/06/18 00:47:04 vedge Exp $	*/
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
	struct widget wid;

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
struct graph *graph_new(void *, const char *, enum graph_type, int, Sint32);

struct graph_item	*graph_add_item(struct graph *, const char *, Uint8,
			                Uint8, Uint8);
void			 graph_free_items(struct graph *);

void	 graph_init(struct graph *, const char *, enum graph_type, int, Sint32);
void	 graph_destroy(void *);
void	 graph_draw(void *);
void	 graph_scale(void *, int, int);

void		 graph_plot(struct graph_item *, Sint32);
__inline__ void	 graph_scroll(struct graph *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_GRAPH_H_ */
