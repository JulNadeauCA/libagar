/*	$Csoft: actor.h,v 1.6 2005/08/27 04:39:59 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ACTOR_H_
#define _AGAR_ACTOR_H_

#include <engine/map/map.h>

#include "begin_code.h"

struct actor_ops {
	struct object_ops ops;
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
};

struct actor {
	struct object obj;
	pthread_mutex_t lock;
	int flags;
#define ACTOR_SAVED_FLAGS	0x00
	void *parent;			/* Parent object (NULL=unset) */
	enum actor_type {
		ACTOR_NONE,
		ACTOR_MAP,
		ACTOR_SCENE
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
	TAILQ_ENTRY(actor) actors;
};

#define ACTOR(ob) ((struct actor *)(ob))
#define ACTOR_OPS(ob) ((const struct actor_ops *)OBJECT(ob)->ops)

__BEGIN_DECLS
void actor_init(void *, const char *, const char *, const struct actor_ops *);
void actor_reinit(void *);
void actor_destroy(void *);
int  actor_load(void *, struct netbuf *);
int  actor_save(void *, struct netbuf *);
void actor_edit(struct actor *, void *);
void actor_update(void *);

int  actor_map_sprite(void *, int, int, int, void *, const char *);
void actor_unmap_sprite(void *);
void actor_move_sprite(void *, int, int);
__inline__ int actor_set_sprite(void *, int, int, int, void *, const char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_ACTOR_H_ */
