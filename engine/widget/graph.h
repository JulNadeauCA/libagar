/*	$Csoft: graph.h,v 1.2 2002/07/21 10:57:12 vedge Exp $	*/
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
#define GRAPH_ORIGIN	0x02	/* Visible zero line */

	int	 origin_y;		/* Current origin position */
	Uint32	 origin_color[2];	/* Color for origin line */

	int	 xinc;		/* X increment */
	Sint32	 yrange;	/* Max. value */

	int	 xoffs;		/* Display offset */
	char	 *caption;	/* Graph description */
	struct	 itemq items;	/* Lists of values */
};

struct graph	*graph_new(struct region *, const char *, enum graph_type,
		     Sint32, int, int, int);
void		 graph_init(struct graph *, const char *, enum graph_type,
		     Sint32, int, int, int);
struct graph_item *graph_add_item(struct graph *, char *, Uint32);
void	 	 graph_destroy(void *);
void		 graph_draw(void *);
void		 graph_plot(struct graph_item *, Sint32);

