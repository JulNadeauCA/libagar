/*	$Csoft	    */

struct direction {
	int	tick;		/* Move tick */
	int	hiwat;		/* Move tick high watermark */
	int	speed;		/* Soft-scroll increments */

	void	*ob;		/* Object back pointer */
	struct	map *map;	/* Map back pointer */

	int	set;		/* Set direction (move) */
	int	current;	/* Current direction mask */
	int	clear;		/* Clear direction mask (stop move) */
	int	moved;		/* Moved direction mask */
#define DIR_UP		0x01
#define DIR_DOWN	0x02
#define DIR_LEFT	0x04
#define DIR_RIGHT	0x08
#define DIR_ALL		0xff

	int	flags;
#define DIR_SCROLL	0x01
};

int	direction_init(struct direction *, void *, struct map *, int, int, int);
void	direction_set(struct direction *, int, int);
int	direction_update(struct direction *, int *, int *);
void	direction_moved(struct direction *, int *, int *, int);

