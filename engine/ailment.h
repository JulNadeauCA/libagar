/*	$Csoft: ailment.h,v 1.2 2002/06/09 09:06:09 vedge Exp $	*/
/*	Public domain	*/

TAILQ_HEAD(ailmentq, ailment);

enum ailment_type {
	AILMENT_ALIVE,		/* Alive */
	AILMENT_DEAD,		/* Defeated */
	AILMENT_REGEN,		/* HP regen */
	AILMENT_POISON,		/* HP decay */
	AILMENT_OSMOSE,		/* MP drain */
	AILMENT_DRAIN,		/* HP drain */
	AILMENT_DOOM,		/* Doom effect */
};

struct ailment {
	enum	 ailment_type type;
	struct	 object *origin;	/* Originator */
	Uint32	 flags;
#define AILMENT_SAVE	0x01

	TAILQ_ENTRY(ailment) ailments;
};

/* Alive */
struct ailment_alive {
	struct	 ailment ail;

	Uint32	 ticks;			/* Ticks since alive */
};

/* Defeated */
struct ailment_dead {
	struct	 ailment ail;

	Uint32	 ticks;			/* Ticks since defeated */
	Uint32	 flags;
#define AIL_DEAD_CANREVIVE	0x01
};

/* Regen or poison */
struct ailment_hp {
	struct	 ailment ail;

	Sint32	 rate;		/* HP regained/lost at every tick */
};

/* Change in MP */
struct ailment_mp {
	struct	 ailment ail;

	Sint32	 rate;		/* MP regained/lost at every tick */
};

/* Doom */
struct ailment_doom {
	struct	 ailment ail;

	Uint32	 cur, max;	/* Increment */
};

#define AILMENT(ob)	((struct ailment *)(ob))

void	ailment_init(struct ailment *, enum ailment_type, struct object *,
	    Uint32);

