/*	$Csoft: char.h,v 1.25 2002/06/09 15:00:52 vedge Exp $	*/
/*	Public domain	*/

struct perso {
	struct object	obj;

	Uint32	 level;		/* Current level */
	Uint32	 exp;		/* Experience */
	Uint32	 age;		/* Age */
	Uint32	 seed;		/* Random seed */
	
	Uint32	 flags;
#define PERSO_FOCUSED	0x0001		/* Focused */
#define PERSO_DASH	0x0010		/* Boost timer temporarily */
#define PERSO_DONTSAVE	(PERSO_DASH)

	Uint32	 maxhp, maxmp;	/* Maximum HP/MP */
	Uint32	 hp, mp;	/* Effective HP/MP */
	Uint32	 maxspeed;	/* Max. speed in ms (saved) */
	Uint32	 nzuars;	/* Money */

	SDL_TimerID	timer;

	pthread_mutex_t	lock;	/* Lock on whole structure */
};

#define PERSO(ob)	((struct perso *)(ob))

struct perso	*perso_new(char *, char *);
void		 perso_init(struct perso *, char *, char *);
void		 perso_destroy(void *);
int		 perso_load(void *, int);
int		 perso_save(void *, int);
void		 perso_attached(int, union evarg *);
void		 perso_detached(int, union evarg *);

