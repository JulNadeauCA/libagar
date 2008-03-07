/*	Public domain	*/

#ifndef _AGAR_MAP_ACTOR_H_
#define _AGAR_MAP_ACTOR_H_

#ifdef _AGAR_INTERNAL
#include <map/map.h>
#else
#include <agar/map/map.h>
#endif

#include "begin_code.h"

typedef struct map_actor_class {
	struct ag_object_class _inherit;
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
} MAP_ActorClass;

enum map_actor_type {
	AG_ACTOR_NONE,
	AG_ACTOR_MAP,
	AG_ACTOR_SCENE
};

typedef struct map_actor {
	struct ag_object obj;
	AG_Mutex lock;
	int flags;
#define AG_ACTOR_SAVED_FLAGS	0x00
	MAP *parent;			/* Parent map (or NULL) */
	enum map_actor_type type;
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
	AG_TAILQ_ENTRY(map_actor) actors;
} MAP_Actor;

#define MAP_ACTOR(ob) ((MAP_Actor *)(ob))
#define MAP_ACTOR_OPS(ob) ((MAP_ActorClass *)AGOBJECT(ob)->cls)

__BEGIN_DECLS
extern AG_ObjectClass mapActorClass;

MAP_Actor *MAP_ActorNew(void *, const char *);
void	   MAP_ActorUpdate(void *);
int	   MAP_ActorSetTile(void *, int, int, int, RG_Tileset *, const char *);
int	   MAP_ActorMapTile(void *, int, int, int, RG_Tileset *, const char *);
void	   MAP_ActorUnmapTile(void *);
void	   MAP_ActorMoveTile(void *, int, int);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_MAP_ACTOR_H_ */
