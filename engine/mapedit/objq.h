/*	$Csoft: objq.h,v 1.1 2002/06/24 18:41:11 vedge Exp $	*/
/*	Public domain	*/

struct objq {
	struct	 widget wid;

	int	 flags;
#define OBJQ_HORIZ	0x01
#define OBJQ_VERT	0x02

	struct {
		int	x;
		int	y;
	} mouse;

	struct	 mapedit *med;
	int	 offs;
};

struct objq	*objq_new(struct region *, struct mapedit *, int, int, int);
void		 objq_init(struct objq *, struct mapedit *, int, int, int);
void		 objq_destroy(void *);
void		 objq_draw(void *);

