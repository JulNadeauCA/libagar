/*	$Csoft: perso.h,v 1.7 2003/05/08 12:15:53 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PERSO_H_
#define _AGAR_PERSO_H_
#include "begin_code.h"

struct perso {
	struct object	 obj;

	pthread_mutex_t	 lock;
	char		*name;			/* Character name set by user */
	Uint32		 flags;
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
extern DECLSPEC struct perso	*perso_new(void *, char *);
extern DECLSPEC void		 perso_init(void *, char *);
extern DECLSPEC void		 perso_destroy(void *);
extern DECLSPEC void		 perso_destroy(void *);
extern DECLSPEC int		 perso_load(void *, struct netbuf *);
extern DECLSPEC int		 perso_save(void *, struct netbuf *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_PERSO_H_ */
