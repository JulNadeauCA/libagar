/*	$Csoft: perso.h,v 1.12 2003/06/10 07:53:49 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PERSO_H_
#define _AGAR_PERSO_H_
#include "begin_code.h"

#define PERSO_NAME_MAX	64

struct perso {
	struct object obj;

	pthread_mutex_t	 lock;
	Uint16		 name[PERSO_NAME_MAX];	/* Name set by user */
	Uint32		 flags;
	Uint32		 seed;			/* Random seed */
	Sint32		 level;			/* Current level */
	Uint32		 exp;			/* Experience */
	int		 age;			/* Age */
	int		 maxhp, maxmp;		/* Maximum HP/MP */
	int		 hp, mp;		/* Effective HP/MP */
	Uint32		 nzuars;		/* Money */
};

#define PERSO(ob)	((struct perso *)(ob))

__BEGIN_DECLS
struct perso	*perso_new(void *, const char *);
void		 perso_init(void *, const char *);
void		 perso_destroy(void *);
void		 perso_edit(void *);
int		 perso_load(void *, struct netbuf *);
int		 perso_save(void *, struct netbuf *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_PERSO_H_ */
