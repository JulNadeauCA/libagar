/*	$Csoft: tilestack.h,v 1.2 2002/07/07 06:28:41 vedge Exp $	*/
/*	Public domain	*/

struct tilestack {
	struct	 widget wid;

	int	 flags;
#define TILESTACK_HORIZ		0x01
#define TILESTACK_VERT		0x02

	struct	 mapview *mv;

	int	 offs;
};

struct tilestack	*tilestack_new(struct region *, int, int, int,
			     struct mapview *mv);
void			 tilestack_init(struct tilestack *, int, int, int,
			     struct mapview *mv);
void		 	 tilestack_destroy(void *);
void			 tilestack_draw(void *);

