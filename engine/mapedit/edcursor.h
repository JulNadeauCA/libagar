/*	$Csoft$	*/
/*	Public domain	*/

struct mapedit;
struct map;
struct mapdir;

struct edcursor {
	struct	 object obj;

	Uint32	 flags;
#define EDCURSOR_INSERT	0x01	/* Insert mode */

	struct	 mapedit *med;	/* Back pointer to map editor */
	struct	 mapview *mv;	/* Back pointer to map view widget */

	struct	 map *map;	/* Location on map */
	Uint32	 x, y;

	struct	 mapdir dir;
};

struct edcursor	*edcursor_new(int, struct mapview *, struct map *);
void		 edcursor_init(struct edcursor *, int, struct mapview *,
		     struct map *);
void		 edcursor_move(struct edcursor *, Uint32, Uint32);
void		 edcursor_set(struct edcursor *, int);

/* Anims */
enum {
	EDCURSOR_SELECT
};

/* Sprites */
enum {
	EDCURSOR_ICON
};

