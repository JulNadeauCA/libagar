/*	$Csoft	    */

struct direction {
	int	tick;		/* Move tick */
	int	hiwat;		/* Move tick high watermark */

	int	current;	/* Current direction mask (move) */
	int	clear;		/* Clear direction mask (stop move) */
	int	moved;		/* Moved direction mask */
#define DIR_UP		0x01
#define DIR_DOWN	0x02
#define DIR_LEFT	0x04
#define DIR_RIGHT	0x08
#define DIR_ALL		0xff

	int	flags;
#define DIR_SCROLL	0x01
#define DIR_ONMAP	0x02
};

int	direction_init(struct direction *, int, int);
void	direction_set(struct direction *, int, int);
int	direction_update(struct direction *, struct map *, int *, int *);

