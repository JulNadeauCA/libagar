/*	$Csoft$	*/

TAILQ_HEAD(ailmentq, ailment);

struct ailment {
	struct	 object *origin;	/* Originator */

	Uint32	 flags;
#define AILMENT_SAVE	0x01

	enum {
		AIL_REGEN,		/* HP regen */
		AIL_POISON,		/* HP decay */
		AIL_OSMOSE,		/* MP drain */
		AIL_DRAIN,		/* HP drain */
		AIL_DOOM,		/* Doom effect */
	} type;

	TAILQ_ENTRY(ailment) ailments;
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

