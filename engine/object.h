/*	$Csoft: object.h,v 1.23 2002/03/17 09:15:00 vedge Exp $	*/

#ifndef _AGAR_OBJECT_H_
#define _AGAR_OBJECT_H_

struct map;
struct noderef;
struct mapdir;
struct mappos;
struct input;
struct anim;

struct obvec {
	int	(*destroy)(void *);
	int	(*load)(void *, int);
	int	(*save)(void *, int);
	int	(*link)(void *);
	int	(*unlink)(void *);
};

struct object {
	char	*name;			/* Name string (key) */
	char	*desc;			/* Optional description */
	int	 id;			/* Unique identifier at runtime */
	char	 saveext[4];		/* File extension for state saves */
	struct	 obvec *vec;

	int	 flags;
#define OBJ_DEFERGC	0x0001		/* Defer garbage collection */
#define OBJ_EDITABLE	0x0002		/* Allow map edition */

	struct	 anim **anims;		/* Animation structures */
	SDL_Surface **sprites;		/* Single surfaces */
	int	 nanims, maxanims;
	int	 nsprites, maxsprites;
	
	struct	 mappos *pos;		/* Position on the map */

	SLIST_ENTRY(object) wobjs;	/* Linked objects */
};

#define OBJECT(ob)	((struct object *)(ob))
#define SPRITE(ob, sp)	OBJECT((ob))->sprites[(sp)]

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
void	 increase_uint32(Uint32 *, Uint32, Uint32);
void	 decrease_uint32(Uint32 *, Uint32, Uint32);
void	 object_dump(void *);

struct object	*object_strfind(char *);
struct mappos	*object_addpos(void *, Uint32, Uint32, struct input *,
		    struct map *, Uint32, Uint32);
void		 object_delpos(void *);

#endif	/* _AGAR_OBJECT_H_ */
