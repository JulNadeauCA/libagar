/*	$Csoft: graph.h,v 1.9 2003/04/12 01:45:49 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_GRAPH_H_
#define _AGAR_WIDGET_GRAPH_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

struct graph;

struct graph_item {
	char	*name;		/* Identifier */
	Uint32	color;		/* Foreground color */

	Sint32	*vals;		/* Relative values */
	Uint32	nvals, maxvals;

	struct	graph *graph;	/* Back pointer */
	TAILQ_ENTRY(graph_item) items;
};

TAILQ_HEAD(itemq, graph_item);

struct graph {
	struct widget	wid;

	enum graph_type {
		GRAPH_POINTS,
		GRAPH_LINES
	} type;

	Uint32	 flags;
#define GRAPH_SCROLL	0x01	/* Scroll when a point is past visible area */
#define GRAPH_ORIGIN	0x02	/* Visible zero line */

	Sint32	 yrange;	/* Max. value */

	Sint32	 xoffs;		/* Display offset */
	Uint8	 xinc;		/* X increment */
	Uint8	 origin_y;	/* Current origin position (%) */

	char	 *caption;	/* Graph description */

	struct itemq	items;	/* Lists of values */
};

__BEGIN_DECLS
extern DECLSPEC struct graph	  *graph_new(struct region *, const char *,
				             enum graph_type, Uint32, Sint32,
					     int, int);
extern DECLSPEC void		   graph_init(struct graph *, const char *,
				              enum graph_type, Uint32, Sint32,
					      int, int);
extern DECLSPEC struct graph_item *graph_add_item(struct graph *, char *,
				                  Uint32);
extern DECLSPEC void	 graph_destroy(void *);
extern DECLSPEC void	 graph_draw(void *);
extern DECLSPEC void	 graph_plot(struct graph_item *, Sint32);
extern DECLSPEC int	 graph_load(void *, struct netbuf *);
extern DECLSPEC int	 graph_save(void *, struct netbuf *);
extern DECLSPEC void	 graph_scroll(struct graph *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_GRAPH_H_ */
