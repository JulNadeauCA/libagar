/*	$Csoft: graph.h,v 1.17 2003/09/07 07:58:37 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_GRAPH_H_
#define _AGAR_WIDGET_GRAPH_H_

#include <engine/widget/widget.h>
#include <engine/widget/label.h>

#include "begin_code.h"

typedef Sint16 graph_val_t;

struct graph;

struct graph_item {
	char	 name[LABEL_MAX];	/* Description */
	Uint32	 color;			/* Line color */
	graph_val_t *vals;		/* Value array */
	Uint32	 nvals;
	Uint32	 maxvals;
	Uint32	 limit;

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
	char		 caption[LABEL_MAX];
	int		 flags;
#define GRAPH_SCROLL	0x01	/* Scroll if the end is not visible */
#define GRAPH_ORIGIN	0x02	/* Visible origin */

	graph_val_t	 yrange;	/* Max. value */
	graph_val_t	 xoffs;		/* Display offset */
	int		 origin_y;	/* Origin position (%) */
	struct itemq	 items;		/* Lists of values */
};

__BEGIN_DECLS
struct graph *graph_new(void *, const char *, enum graph_type, int,
	                graph_val_t);

struct graph_item	*graph_add_item(struct graph *, const char *, Uint8,
			                Uint8, Uint8, Uint32);
void			 graph_free_items(struct graph *);

void	 graph_init(struct graph *, const char *, enum graph_type, int,
	            graph_val_t);
void	 graph_destroy(void *);
void	 graph_draw(void *);
void	 graph_scale(void *, int, int);

void		 graph_plot(struct graph_item *, graph_val_t);
__inline__ void	 graph_scroll(struct graph *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_GRAPH_H_ */
