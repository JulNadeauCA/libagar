/*	$Csoft: graph.h,v 1.3 2002/07/22 05:22:08 vedge Exp $	*/
/*	Public domain	*/

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

	Uint32	 origin_y;		/* Current origin position */
	Uint32	 origin_color[2];	/* Color for origin line */

	Uint32	 xinc;		/* X increment */
	Sint32	 yrange;	/* Max. value */

	Uint32	 xoffs;		/* Display offset */
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

