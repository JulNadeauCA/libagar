/*	$Csoft: perso.h,v 1.8 2003/05/18 00:16:57 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PERSO_H_
#define _AGAR_PERSO_H_
#include "begin_code.h"

#define PERSO_NAME_MAX	64

struct perso {
	struct object	 obj;

	pthread_mutex_t	 lock;
	char		 name[PERSO_NAME_MAX];	/* Name set by user */
	Uint32		 flags;
	int		 level;			/* Current level */
	int		 exp;			/* Experience */
	int		 age;			/* Age */
	Uint32		 seed;			/* Random seed */
	int		 maxhp, maxmp;		/* Maximum HP/MP */
	int		 hp, mp;		/* Effective HP/MP */
	unsigned int	 nzuars;		/* Money */
	SDL_TimerID	 timer;
};

#define PERSO(ob)	((struct perso *)(ob))

__BEGIN_DECLS
extern DECLSPEC struct perso	*perso_new(void *, char *);
extern DECLSPEC void		 perso_init(void *, char *);
extern DECLSPEC void		 perso_destroy(void *);
extern DECLSPEC void		 perso_edit(void *);
extern DECLSPEC int		 perso_load(void *, struct netbuf *);
extern DECLSPEC int		 perso_save(void *, struct netbuf *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_PERSO_H_ */
