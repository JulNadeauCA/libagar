/*	$Csoft: object.h,v 1.18 2002/02/21 02:19:25 vedge Exp $	*/

#include <engine/physics.h>

/*
 * Back reference to map entry. This holds additional information
 * for structures which move on the map.
 */
struct map_bref {
	struct	map *map;	/* Map */
	Uint32	x, y;		/* Map coordinates */
	struct	noderef *nref;	/* Node reference */

	struct	mapdir dir;	/* Direction */
	Uint32	speed;		/* Current speed in ms */
	struct	input *input;	/* Controller (or NULL) */

	SLIST_ENTRY(map_bref) brefs;	/* Map back references */
};

struct obvec {
	int	(*destroy)(void *);
	int	(*load)(void *, int);
	int	(*save)(void *, int);
	int	(*link)(void *);
	int	(*unlink)(void *);
	void	(*dump)(void *);
	int	(*madd)(void *, struct map_bref *);
	int	(*mmove)(void *, struct map_bref *);
	int	(*mdel)(void *, struct map_bref *);
};

struct object {
	char	*name;		/* Name string (key) */
	char	*desc;		/* Optional description */
	int	 id;		/* Unique identifier at runtime */
	char	 saveext[4];	/* File extension for state saves */
	struct	 obvec *vec;

	int	 flags;
#define OBJ_DEFERGC	0x0001	/* Defer garbage collection */
#define OBJ_EDITABLE	0x0002	/* The map editor might use this object */

	struct	 anim **anims;	/* Animation structures */
	int	 nanims;
	int	 maxanims;
	SDL_Surface **sprites;	/* Single surfaces */
	int	 nsprites;
	int	 maxsprites;

	SLIST_HEAD(, map_bref) brefsh;	/* Map back references */
	SLIST_ENTRY(object) wobjs;	/* All objects */
};

int	 object_init(struct object *, char *, int, struct obvec *vec);
int	 object_addanim(struct object *, struct anim *);
int	 object_addsprite(struct object *, SDL_Surface *);
int	 object_destroy(void *);
void	 object_lategc(void);
int	 object_link(void *);
int	 object_unlink(void *);
int	 object_load(void *);
int	 object_loadfrom(void *, char *);
int	 object_save(void *);
void	 increase(int *, int, int);
void	 decrease(int *, int, int);
void	 object_dump(void *);

struct object	*object_strfind(char *);
struct map_bref	*object_madd(void *, Uint32, Uint32, struct map *, Uint32,
		     Uint32);
void		 object_mdel(void *, Uint32, Uint32, struct map *, Uint32,
		     Uint32);

