/*	$Csoft: object.h,v 1.15 2002/02/17 08:02:05 vedge Exp $	*/

struct obvec {
	int	(*destroy)(void *);
	void	(*event)(void *, SDL_Event *);
	int	(*load)(void *, int);
	int	(*save)(void *, int);
	int	(*link)(void *);
	int	(*unlink)(void *);
};

struct object {
	char	*name;		/* Name string (key) */
	char	*desc;		/* Optional description */
	int	 id;		/* Unique identifier at runtime */
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
#ifdef DEBUG
void	 object_dump(struct object *);
#endif

struct object	*object_strfind(char *);

