/*	$Csoft$	*/
/*	Public domain	*/

struct tileq {
	struct	 widget wid;

	int	 flags;
#define TILEQ_HORIZ	0x01
#define TILEQ_VERT	0x02

	struct {
		int	x;
		int	y;
	} mouse;

	struct	 mapedit *med;
	int	 offs;
};

struct tileq	*tileq_new(struct region *, struct mapedit *, int, int, int);
void		 tileq_init(struct tileq *, struct mapedit *, int, int, int);
void		 tileq_destroy(void *);
void		 tileq_draw(void *);

