/*	$Csoft: perso.h,v 1.3 2002/12/13 12:33:16 vedge Exp $	*/
/*	Public domain	*/

struct perso {
	struct object	obj;

	pthread_mutex_t	 lock;
	char		*name;		/* Character name set by user */
	Uint32		 flags;
#define PERSO_FOCUSED	0x01
#define PERSO_EPHEMERAL	0x00
	int	 level;			/* Current level */
	int	 exp;			/* Experience */
	int	 age;			/* Age */
	Uint32	 seed;			/* Random seed */
	int	 maxhp, maxmp;		/* Maximum HP/MP */
	int	 hp, mp;		/* Effective HP/MP */
	Uint32	 nzuars;		/* Money */
	SDL_TimerID	timer;
};

#define PERSO(ob)	((struct perso *)(ob))

struct perso	*perso_new(char *, char *, int, int);
void		 perso_init(struct perso *, char *, char *, int, int);
void		 perso_destroy(void *);
void		 perso_destroy(void *);
int		 perso_load(void *, struct netbuf *);
int		 perso_save(void *, struct netbuf *);
void		 perso_attached(int, union evarg *);
void		 perso_detached(int, union evarg *);
void		 perso_say(struct perso *, const char *, ...);

