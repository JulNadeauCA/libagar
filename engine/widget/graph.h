/*	$Csoft: graph.h,v 1.6 2002/12/17 01:05:00 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_GRAPH_H_
#define _AGAR_WIDGET_GRAPH_H_

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
	struct	 widget wid;

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

	struct	 itemq items;	/* Lists of values */
};

struct graph	*graph_new(struct region *, const char *, enum graph_type,
		     Uint32, Sint32, int, int);
void		 graph_init(struct graph *, const char *, enum graph_type,
		     Uint32, Sint32, int, int);
struct graph_item *graph_add_item(struct graph *, char *, Uint32);
void	 	 graph_destroy(void *);
void		 graph_draw(void *);
void		 graph_plot(struct graph_item *, Sint32);
int		 graph_load(void *, int);
int		 graph_save(void *, int);
void		 graph_scroll(struct graph *, int);

#endif /* _AGAR_WIDGET_GRAPH_H_ */
