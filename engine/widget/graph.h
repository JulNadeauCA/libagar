/*	$Csoft: graph.h,v 1.1 2002/07/20 18:55:59 vedge Exp $	*/
/*	Public domain	*/

struct graph_item {
	char	*name;		/* Identifier */
	Uint32	color;		/* Foreground color */

	Sint32	*vals;		/* Relative values */
	int	nvals, maxvals;

	TAILQ_ENTRY(graph_item) items;
};

TAILQ_HEAD(itemq, graph_item);

struct graph {
	struct	 widget wid;

	enum graph_type {
		GRAPH_POINTS,
		GRAPH_LINES
	} type;

	int	 flags;
#define GRAPH_SCROLL	0x01	/* Scroll when a point is past visible area */
	
	int	 xinc;		/* X increment */
	int	 xoffs;		/* Display offset */
	char	 *caption;	/* Graph description */
	struct	 itemq items;	/* Lists of values */
};

struct graph	*graph_new(struct region *, const char *, enum graph_type,
		     int, int, int);
void		 graph_init(struct graph *, const char *, enum graph_type,
		     int, int, int);
struct graph_item *graph_add_item(struct graph *, char *, Uint32);
void	 	 graph_destroy(void *);
void		 graph_draw(void *);
void		 graph_plot(struct graph_item *, Sint32);

