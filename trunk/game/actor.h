/*	$Csoft: actor.h,v 1.1 2005/09/20 13:46:29 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ACTOR_H_
#define _AGAR_ACTOR_H_

#include <engine/map/map.h>

#include "begin_code.h"

typedef struct ag_actor_ops {
	struct ag_object_ops ops;
	void (*map)(void *, void *);
	void (*unmap)(void *, void *);
	void (*update)(void *, void *);
	int (*keydown)(void *, int ks, int km);
	int (*keyup)(void *, int ks, int km);
	int (*mousemotion)(void *, int x, int y, int xrel, int yrel, int btn);
	int (*mousebuttondown)(void *, int x, int y, int btn);
	int (*mousebuttonup)(void *, int x, int y, int btn);
	void (*joyaxis)(void *, int dev, int axis, int value);
	void (*joyball)(void *, int dev, int ball, int xrel, int yrel);
	void (*joyhat)(void *, int dev, int hat, int value);
	void (*joybutton)(void *, int dev, int button, int state);
} AG_ActorOps;

typedef struct ag_actor {
	struct ag_object obj;
	pthread_mutex_t lock;
	int flags;
#define AG_ACTOR_SAVED_FLAGS	0x00
	void *parent;			/* Parent object (NULL=unset) */
	enum ag_actor_type {
		AG_ACTOR_NONE,
		AG_ACTOR_MAP,
		AG_ACTOR_SCENE
	} type;
	union {
		struct {
			int x, y, l0, l1;	/* Destination position */
			int x0, y0, x1, y1;	/* Extent */
			int xmot, ymot;		/* Motion offset */
			int da, dv;		/* Direction (CW degs from W) */
		} map;
		struct {
			double x, y, z;		/* Position */
			double dx, dy, dz;	/* Direction vector */
		} scene;
	} data;
#define g_map data.map
#define g_scene data.scene
	TAILQ_ENTRY(ag_actor) actors;
} AG_Actor;

#define AGACTOR(ob) ((AG_Actor *)(ob))
#define AGACTOR_OPS(ob) ((AG_ActorOps *)AGOBJECT(ob)->ops)

__BEGIN_DECLS
void	AG_ActorInit(void *, const char *, const char *, const AG_ActorOps *);
void	AG_ActorReinit(void *);
void	AG_ActorDestroy(void *);
int	AG_ActorLoad(void *, AG_Netbuf *);
int	AG_ActorSave(void *, AG_Netbuf *);
void	AG_ActorEdit(AG_Actor *, void *);
void	AG_ActorUpdate(void *);

int	AG_ActorMapSprite(void *, int, int, int, void *, const char *);
void	AG_ActorUnmapSprite(void *);
void	AG_ActorMoveSprite(void *, int, int);
__inline__ int AG_ActorSetSprite(void *, int, int, int, void *, const char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_ACTOR_H_ */
