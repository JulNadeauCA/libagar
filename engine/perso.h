/*	$Csoft: perso.h,v 1.2 2002/11/22 04:43:45 vedge Exp $	*/
/*	Public domain	*/

struct perso {
	struct object	obj;

	char	*name;		/* Character name */
	Uint32	 level;		/* Current level */
	Uint32	 exp;		/* Experience */
	Uint32	 age;		/* Age */
	Uint32	 seed;		/* Random seed */
	
	Uint32	 flags;
#define PERSO_FOCUSED	0x01		/* Focused */
#define PERSO_EPHEMERAL	0x00

	Uint32	 maxhp, maxmp;	/* Maximum HP/MP */
	Uint32	 hp, mp;	/* Effective HP/MP */
	Uint32	 maxspeed;	/* Max. speed in ms (saved) */
	Uint32	 nzuars;	/* Money */

	SDL_TimerID	timer;

	pthread_mutex_t	lock;	/* Lock on whole structure */
};

#define PERSO(ob)	((struct perso *)(ob))

struct perso	*perso_new(char *, char *, Uint32, Uint32);
void		 perso_init(struct perso *, char *, char *, Uint32, Uint32);
void		 perso_destroy(void *);
void		 perso_destroy(void *);
int		 perso_load(void *, int);
int		 perso_save(void *, int);
void		 perso_attached(int, union evarg *);
void		 perso_detached(int, union evarg *);

void		 perso_say(struct perso *, const char *, ...);

