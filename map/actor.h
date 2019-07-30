/*	Public domain	*/

#include <agar/map/begin.h>

typedef struct map_actor_class {
	struct ag_object_class _inherit;
	void (*_Nullable map)(void *_Nonnull, void *_Nonnull);
	void (*_Nullable unmap)(void *_Nonnull, void *_Nonnull);
	void (*_Nullable update)(void *_Nonnull, void *_Nonnull);
	int  (*_Nullable keydown)(void *_Nonnull, int ks, int km);
	int  (*_Nullable keyup)(void *_Nonnull, int ks, int km);
	int  (*_Nullable mousemotion)(void *_Nonnull, int x, int y,
	                              int xrel, int yrel, int btn);
	int  (*_Nullable mousebuttondown)(void *_Nonnull, int x, int y, int btn);
	int  (*_Nullable mousebuttonup)(void *_Nonnull, int x, int y, int btn);
	void (*_Nullable joyaxis)(void *_Nonnull, int dev, int axis, int value);
	void (*_Nullable joyball)(void *_Nonnull, int dev, int ball,
	                          int xrel, int yrel);
	void (*_Nullable joyhat)(void *_Nonnull, int dev, int hat, int value);
	void (*_Nullable joybutton)(void *_Nonnull, int dev, int button,
	                            int state);
} MAP_ActorClass;

enum map_actor_type {
	AG_ACTOR_NONE,
	AG_ACTOR_MAP,
	AG_ACTOR_SCENE
};

typedef struct map_actor {
	struct ag_object obj;
	enum map_actor_type type;		/* Type of actor */
	Uint flags;
#define AG_ACTOR_SAVED_FLAGS	0x00
	MAP *_Nullable parent;			/* Current map */
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
#define g_map   data.map
#define g_scene data.scene
	AG_TAILQ_ENTRY(map_actor) actors;
} MAP_Actor;

#define MAP_ACTOR(ob) ((MAP_Actor *)(ob))
#define MAP_ACTOR_OPS(ob) ((MAP_ActorClass *)AGOBJECT(ob)->cls)

__BEGIN_DECLS
extern AG_ObjectClass mapActorClass;

MAP_Actor *_Nonnull MAP_ActorNew(void *_Nullable, const char *_Nonnull);

int  MAP_ActorSetTile(void *_Nonnull, int,int, int,
                      RG_Tileset *_Nonnull, const char *_Nonnull);
int  MAP_ActorMapTile(void *_Nonnull, int,int, int,
                      RG_Tileset *_Nonnull, const char *_Nonnull);

void MAP_ActorUnmapTile(void *_Nonnull);
void MAP_ActorMoveTile(void *_Nonnull, int,int);
__END_DECLS

#include <agar/map/close.h>
