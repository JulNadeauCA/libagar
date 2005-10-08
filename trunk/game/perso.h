/*	$Csoft: perso.h,v 1.22 2005/09/20 13:46:29 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PERSO_H_
#define _AGAR_PERSO_H_

#include <engine/actor.h>

#include "begin_code.h"

#define AG_PERSO_NAME_MAX	256

typedef struct ag_perso {
	struct ag_actor obj;

	pthread_mutex_t	lock;
	void *tileset;			/* Graphics source */
	char  name[AG_PERSO_NAME_MAX];	/* Name set by user */
	Uint32 flags;
	Uint32 seed;			/* Random seed */
	Sint32 level;			/* Current level */
	Uint32 exp;			/* Experience */
	int age;			/* Age */
	int maxhp, maxmp;		/* Maximum HP/MP */
	int hp, mp;			/* Effective HP/MP */
	Uint32 nzuars;			/* Money */
	AG_Timeout move_to;		/* Movement timer */
} AG_Perso;

#define AGPERSO(ob)	((AG_Perso *)(ob))

__BEGIN_DECLS
AG_Perso *AG_PersoNew(void *, const char *);
void	  AG_PersoInit(void *, const char *);
void	  AG_PersoReinit(void *);
void	  AG_PersoDestroy(void *);
void	 *AG_PersoEdit(void *);
void	  AG_PersoArgs(void *);
int	  AG_PersoLoad(void *, AG_Netbuf *);
int	  AG_PersoSave(void *, AG_Netbuf *);
void	  AG_PersoMap(void *, void *);
void	  AG_PersoUpdate(void *, void *);
int	  AG_PersoKeydown(void *, int, int);
int	  AG_PersoKeyup(void *, int, int);
int       AG_PersoCanWalkTo(void *, int, int);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_PERSO_H_ */
