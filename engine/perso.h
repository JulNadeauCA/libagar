/*	$Csoft: perso.h,v 1.6 2003/04/25 09:47:05 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PERSO_H_
#define _AGAR_PERSO_H_
#include "begin_code.h"

struct perso {
	struct object	obj;

	pthread_mutex_t	 lock;
	char		*name;		/* Character name set by user */
	Uint32		 flags;
#define PERSO_FOCUSED	0x01
#define PERSO_EPHEMERAL	0x00
	int		 level;			/* Current level */
	int		 exp;			/* Experience */
	int		 age;			/* Age */
	Uint32		 seed;			/* Random seed */
	int		 maxhp, maxmp;		/* Maximum HP/MP */
	int		 hp, mp;		/* Effective HP/MP */
	Uint32		 nzuars;		/* Money */
	SDL_TimerID	 timer;
};

#define PERSO(ob)	((struct perso *)(ob))

__BEGIN_DECLS
extern DECLSPEC struct perso	*perso_new(struct map *, char *, int, int);
extern DECLSPEC void		 perso_init(struct perso *, char *, int, int);
extern DECLSPEC void		 perso_destroy(void *);
extern DECLSPEC void		 perso_destroy(void *);
extern DECLSPEC int		 perso_load(void *, struct netbuf *);
extern DECLSPEC int		 perso_save(void *, struct netbuf *);
extern DECLSPEC void		 perso_attached(int, union evarg *);
extern DECLSPEC void		 perso_detached(int, union evarg *);
extern DECLSPEC void		 perso_say(struct perso *, const char *, ...);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_PERSO_H_ */
