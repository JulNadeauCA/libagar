/*	$Csoft$	*/
/*	Public domain	*/

struct tilestack {
	struct	 widget wid;

	int	 flags;
#define TILESTACK_HORIZ	0x01
#define TILESTACK_VERT	0x02
	
	int	 offs;
};

struct tilestack	*tilestack_new(struct region *, int, int, int);
void			 tilestack_init(struct tilestack *, int, int, int);
void		 	 tilestack_destroy(void *);
void			 tilestack_draw(void *);

