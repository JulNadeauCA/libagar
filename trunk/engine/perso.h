/*	$Csoft: perso.h,v 1.21 2005/09/19 01:25:16 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PERSO_H_
#define _AGAR_PERSO_H_

#include <engine/actor.h>

#include "begin_code.h"

#define PERSO_NAME_MAX	256

struct perso {
	struct actor obj;

	pthread_mutex_t	 lock;
	void		*tileset;		/* Graphics source */
	char		 name[PERSO_NAME_MAX];	/* Name set by user */
	Uint32		 flags;
	Uint32		 seed;		/* Random seed */
	Sint32		 level;		/* Current level */
	Uint32		 exp;		/* Experience */
	int		 age;		/* Age */
	int		 maxhp, maxmp;	/* Maximum HP/MP */
	int		 hp, mp;	/* Effective HP/MP */
	Uint32		 nzuars;	/* Money */
	struct timeout	 move_to;	/* Movement timer */
};

#define PERSO(ob)	((struct perso *)(ob))

__BEGIN_DECLS
struct perso	*perso_new(void *, const char *);
void		 perso_init(void *, const char *);
void		 perso_reinit(void *);
void		 perso_destroy(void *);
void		*perso_edit(void *);
void		 perso_args(void *);
int		 perso_load(void *, struct netbuf *);
int		 perso_save(void *, struct netbuf *);
void		 perso_map(void *, void *);
void		 perso_update(void *, void *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_PERSO_H_ */
