/*	$Csoft: object.h,v 1.8 2002/02/05 05:46:54 vedge Exp $	*/

#ifndef _ENGINE_OBJECT_H_
#define _ENGINE_OBJECT_H_

struct object {
	char	*name;		/* Optional name */
	char	*desc;		/* Optional description */
	int	 id;		/* Equivalent of a PID */

	int	 wmask;		/* Asynchronous wait mask */

	int	 flags;
#define OBJ_INVISIBLE	0x0001	/* Object is not visible on the map */
#define OBJ_DEFERGC	0x0002	/* Defer garbage collection */
#define OBJ_EDITABLE	0x0004	/* The map editor might use this object */
#define TIME_HOOK	0x0010	/* Call every tick */
#define DESTROY_HOOK	0x0020	/* Pre-destroy hook */
#define LOAD_FUNC	0x0100	/* Load routine */
#define SAVE_FUNC	0x0200	/* Save routine */

	/*
	 * Sprite and animation reference offsets are independent
	 * (ie. offset 0 means animation 0 if the reference has the
	 * MAPREF_ANIM flag set, or sprite 0 if the reference has
	 * the MAPREF_SPRITE flag set).
	 *
	 * Animation and sprite lists are assumed to be constructed
	 * at load-time, and not modified subsequently.
	 */
	GSList	*anims;		/* Animation structures */
	int	 nanims;	/* Animation count */
	GSList	*sprites;	/* Sprite surfaces */
	int	 nsprites;	/* Sprite count */
	pthread_mutex_t lock;	/* Lock on sprite list */

	void	 (*destroy_hook)(void *);
	int	 (*load)(void *, char *);
	int	 (*save)(void *, char *);

	SLIST_ENTRY(object) wobjs;	/* All objects */
};

#define WMASK_DOWN	0x01	/* Y axis - */
#define WMASK_UP	0x02	/* Y axis + */
#define WMASK_LEFT	0x04	/* X axis - */
#define WMASK_RIGHT	0x08	/* X axis + */
#define WMASK_ZIN	0x10	/* Z axis - */
#define WMASK_ZOUT	0x20	/* Z axis + */

int	 object_create(struct object *, char *, char *, int);
int	 object_destroy(void *);
void	 object_lategc(void);
int	 object_init(void);
int	 object_link(void *);
int	 object_unlink(void *);
void	 increase(int *, int, int);
void	 decrease(int *, int, int);
int	 object_wait(void *, int);
#ifdef DEBUG
void	 object_dump(struct object *);
#endif

struct object	*object_strfind(char *);

#endif /* !_ENGINE_OBJECT_H_ */
