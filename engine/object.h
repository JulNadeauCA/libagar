/*	$Csoft: object.h,v 1.13 2002/02/14 06:29:50 vedge Exp $	*/

struct object {
	char	*name;		/* Name string (key) */
	char	*desc;		/* Optional description */
	int	 id;		/* Unique identifier at runtime */

	int	 flags;
#define OBJ_INVISIBLE	0x0001	/* Object is not visible on the map */
#define OBJ_DEFERGC	0x0002	/* Defer garbage collection */
#define OBJ_EDITABLE	0x0004	/* The map editor might use this object */
#define TIME_HOOK	0x0010	/* Call every tick */
#define DESTROY_HOOK	0x0020	/* Pre-destroy hook */
#define LOAD_FUNC	0x0100	/* Load routine */
#define SAVE_FUNC	0x0200	/* Save routine */

	struct	 anim **anims;	/* Animation structures */
	int	 nanims;
	int	 maxanims;
	SDL_Surface **sprites;	/* Single surfaces */
	int	 nsprites;
	int	 maxsprites;

	void	 (*destroy_hook)(void *);
	int	 (*load)(void *, char *);
	int	 (*save)(void *, char *);

	SLIST_ENTRY(object) wobjs;	/* All objects */
};

int	 object_create(struct object *, char *, char *, int);
int	 object_addanim(struct object *, struct anim *);
int	 object_addsprite(struct object *, SDL_Surface *);
int	 object_destroy(void *);
void	 object_lategc(void);
int	 object_init(void);
int	 object_link(void *);
int	 object_unlink(void *);
void	 increase(int *, int, int);
void	 decrease(int *, int, int);
#ifdef DEBUG
void	 object_dump(struct object *);
#endif

struct object	*object_strfind(char *);

