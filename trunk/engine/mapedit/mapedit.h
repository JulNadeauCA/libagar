/*	$Csoft: mapedit.h,v 1.38 2002/06/13 10:48:57 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/edcursor.h>

struct editref {
	int	animi;		/* Index into the object's real anim list. */
	int	spritei;	/* Index into the object's real sprite list */
	void	*p;
	enum {
		EDITREF_SPRITE,	/* SDL_Surface */
		EDITREF_ANIM	/* struct anim */
	} type;
	
	SIMPLEQ_ENTRY(editref) erefs;	/* Reference list */
};

SIMPLEQ_HEAD(erefs_head, editref);

struct editobj {
	struct	object *pobj;		/* Original object structure */
	struct	erefs_head erefsh;	/* Reference list */
	int	nrefs;
	int	nsprites;
	int	nanims;

	TAILQ_ENTRY(editobj) eobjs;	/* Editable object list */
	pthread_mutex_t	lock;		/* Lock on object list */
};

TAILQ_HEAD(eobjs_head, editobj);

struct mapdir;
struct gendir;

struct mapedit {
	struct	 object obj;

	Uint32	 flags;

	struct	 window *toolbar_win;
	struct	 window *new_map_win;
	struct	 window *settings_win;
	struct	 window *coords_win;
	struct	 label *coords_label;
	
	struct	 eobjs_head eobjsh;	/* Shadow objects */
	int	 neobjs;

	pthread_mutex_t	lock;		/* Lock on whole structure */
};

/* Editor sprites */
enum {
	/* Map editor */
	MAPEDIT_ICON,
	MAPEDIT_ORIGIN,
	MAPEDIT_CIRQSEL,
	MAPEDIT_GRID,
	MAPEDIT_BLOCKED,
	MAPEDIT_WALK,
	MAPEDIT_CLIMB,
	MAPEDIT_SLIP,
	MAPEDIT_BIO,
	MAPEDIT_REGEN,
	MAPEDIT_SLOW,
	MAPEDIT_HASTE,
	MAPEDIT_ANIM,
	MAPEDIT_OVERLAP,
	MAPEDIT_NVEL,
	MAPEDIT_SVEL,
	MAPEDIT_WVEL,
	MAPEDIT_EVEL,
	MAPEDIT_STATE,
	MAPEDIT_INSERT_TXT,
	MAPEDIT_REPLACE_TXT,
	MAPEDIT_PROPS_TXT,
	MAPEDIT_GRID_TXT,
	MAPEDIT_ANIM_TXT,
	MAPEDIT_ANIM_INDEPENDENT_TXT,
	MAPEDIT_ANIM_DELTA_TXT,
	/* Toolbar */
	MAPEDIT_TOOL_MAP,
	MAPEDIT_FILEOP_NEW_MAP,
	MAPEDIT_FILEOP_LOAD_MAP,
	MAPEDIT_FILEOP_SAVE_MAP,
	MAPEDIT_FILEOP_SAVE_MAP_AS,
};

void	mapedit_init(struct mapedit *, char *);
int	mapedit_load(void *, int);
int	mapedit_save(void *, int);

void	mapedit_key(struct mapedit *, SDL_Event *);
void	mapedit_move(struct mapedit *, Uint32, Uint32);

void	mapedit_sticky(struct mapedit *);

extern struct mapedit *curmapedit;	/* Controlled map editor */

