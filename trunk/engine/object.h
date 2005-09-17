/*	$Csoft: object.h,v 1.129 2005/09/09 02:11:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJECT_H_
#define _AGAR_OBJECT_H_

#define OBJECT_TYPE_MAX	32
#define OBJECT_NAME_MAX	128
#define OBJECT_PATH_MAX	1024
#define OBJECT_DIGEST_MAX 170

#include <engine/timeout.h>
#include <engine/prop.h>
#include <engine/gfx.h>
#include <engine/audio.h>

#include "begin_code.h"

struct map;
struct input;
struct event;

/* Generic object operation vector */
struct object_ops {
	void	(*init)(void *, const char *);		/* Initialize */
	void	(*reinit)(void *);			/* Reinitialize */
	void	(*destroy)(void *);			/* Free resources */
	int	(*load)(void *, struct netbuf *);	/* Load from network */
	int	(*save)(void *, struct netbuf *);	/* Save to network */
	struct window *(*edit)(void *);			/* Edit object */
};

/* Dependency with respect to another object. */
struct object_dep {
	char		*path;		/* Unresolved object path */
	struct object	*obj;		/* Resolved object */
	Uint32		 count;		/* Reference count */
#define OBJECT_DEP_MAX	(0xffffffff-2)	/* If reached, stay resident */
	TAILQ_ENTRY(object_dep) deps;
};

/* Structure returned by object_get_classinfo(). */
struct object_classinfo {
	char **classes;
	struct object_type **types;
	u_int nclasses;
};

TAILQ_HEAD(objectq, object);

struct object {
	char	 type[OBJECT_TYPE_MAX];		/* Type of object */
	char	 name[OBJECT_NAME_MAX];		/* Identifier */
	char	*save_pfx;			/* Save dir prefix */
	const struct object_ops *ops;		/* Generic operation vector */
	int	 flags;
#define OBJECT_RELOAD_PROPS	0x001	/* Don't free props before load */
#define OBJECT_NON_PERSISTENT	0x002	/* Never include in saves */
#define OBJECT_INDESTRUCTIBLE	0x004	/* Not destructible (advisory) */
#define OBJECT_DATA_RESIDENT	0x008	/* Object data is resident */
#define OBJECT_PRESERVE_DEPS	0x010	/* Preserve cnt=0 dependencies */
#define OBJECT_STATIC		0x020	/* Don't free() after detach */
#define OBJECT_READONLY		0x040	/* Disallow edition (advisory) */
#define OBJECT_WAS_RESIDENT	0x080	/* Used internally by object_load() */
#define OBJECT_IN_SAVE		0x100	/* Used internally by object_load() */
#define OBJECT_REOPEN_ONLOAD	0x200	/* Close and reopen editor on load */
#define OBJECT_REMAIN_DATA	0x400	/* Keep data resident */
#define OBJECT_REMAIN_GFX	0x800	/* Keep graphics resident */
#define OBJECT_SAVED_FLAGS	(OBJECT_RELOAD_PROPS|OBJECT_INDESTRUCTIBLE|\
				 OBJECT_PRESERVE_DEPS|OBJECT_READONLY|\
				 OBJECT_REOPEN_ONLOAD|OBJECT_REMAIN_DATA|\
				 OBJECT_REMAIN_GFX)
#define OBJECT_DUPED_FLAGS	(OBJECT_SAVED_FLAGS|OBJECT_NON_PERSISTENT|\
				 OBJECT_REOPEN_ONLOAD|OBJECT_REMAIN_DATA|\
				 OBJECT_REMAIN_GFX)

	pthread_mutex_t	 lock;
	struct gfx	*gfx;		/* Associated graphics package */
	struct audio	*audio;		/* Associated audio package */
	Uint32		 data_used;	/* Referenced object derivate data */

	TAILQ_HEAD(,event)	 events;	/* Event handlers */
	TAILQ_HEAD(,prop)	 props;		/* Generic properties */
	CIRCLEQ_HEAD(,timeout)	 timeouts;	/* Scheduled function calls */

	/* Uses linkage_lock */
	TAILQ_HEAD(,object_dep)	 deps;		/* Object dependencies */
	struct objectq		 children;	/* Child objects */
	void			*parent;	/* Parent object */

	TAILQ_ENTRY(object) cobjs;	/* Entry in child object queue */
	TAILQ_ENTRY(object) tobjs;	/* Entry in timed object queue */
};

enum object_page_item {
	OBJECT_GFX,		/* Shared graphics */
	OBJECT_AUDIO,		/* Shared audio samples */
	OBJECT_DATA		/* Object data */
};

enum object_checksum_alg {
	OBJECT_MD5,
	OBJECT_SHA1,
	OBJECT_RMD160
};

#define OBJECT_INITIALIZER(ob, type, name, ops) {		\
		(type), (name), "/world", (ops), 0,		\
		PTHREAD_MUTEX_INITIALIZER,			\
		NULL, NULL, NULL, 0, NULL, NULL, 0, 0, NULL,	\
		TAILQ_HEAD_INITIALIZER((ob)->events),		\
		TAILQ_HEAD_INITIALIZER((ob)->props),		\
		CIRCLEQ_HEAD_INITIALIZER((ob)->timeouts),	\
		TAILQ_HEAD_INITIALIZER((ob)->deps),		\
		TAILQ_HEAD_INITIALIZER((ob)->children),		\
		NULL						\
	}

#define OBJECT(ob)		((struct object *)(ob))
#define OBJECT_TYPE(ob, t)	(strcmp(OBJECT(ob)->type,(t))==0)
#define OBJECT_SUBCLASS(ob, t)	object_subclass(OBJECT(ob), t, sizeof(t)-1)

#define OBJECT_FOREACH_CHILD(var, ob, type)				\
	for((var) = (struct type *)TAILQ_FIRST(&OBJECT(ob)->children);	\
	    (var) != (struct type *)TAILQ_END(&OBJECT(ob)->children);	\
	    (var) = (struct type *)TAILQ_NEXT(OBJECT(var), cobjs))

#define OBJECT_FOREACH_CHILD_REVERSE(var, ob, type)			\
	for((var) = (struct type *)TAILQ_LAST(&OBJECT(ob)->children, objectq);\
	    (var) != (struct type *)TAILQ_END(&OBJECT(ob)->children);	\
	    (var) = (struct type *)TAILQ_PREV(OBJECT(var), objectq, cobjs))

#define object_lock(ob) pthread_mutex_lock(&(ob)->lock)
#define object_unlock(ob) pthread_mutex_unlock(&(ob)->lock)

__BEGIN_DECLS
struct object	*object_new(void *, const char *);
void		 object_init(void *, const char *, const char *, const void *);
void		 object_free_data(void *);
void		 object_remain(void *, int);

int	 object_copy_name(const void *, char *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 3);
int	 object_copy_dirname(const void *, char *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 3);
int	 object_copy_filename(const void *, char *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 3);
size_t	 object_copy_checksum(const void *, enum object_checksum_alg, char *);
int	 object_copy_digest(const void *, size_t *, char *);
int	 object_changed(void *);

void		*object_find(const char *);
void		*object_findf(const char *, ...);
__inline__ void	*object_root(const void *);
__inline__ void *object_find_parent(void *, const char *, const char *);
int		 object_in_use(const void *);
void		 object_set_type(void *, const char *);
void		 object_set_name(void *, const char *);
void		 object_set_ops(void *, const void *);

__inline__ SDL_Surface	*object_icon(void *);

__inline__ int object_subclass(struct object *, const char *, size_t);
void	       object_get_classinfo(const char *, struct object_classinfo *);
void	       object_free_classinfo(struct object_classinfo *);

void	 object_move_up(void *);
void	 object_move_down(void *);
void	*object_duplicate(void *);
void	 object_destroy(void *);
void	 object_unlink_datafiles(void *);

void	 object_free_children(struct object *);
void	 object_free_props(struct object *);
void 	 object_free_events(struct object *);
void	 object_free_deps(struct object *);
void	 object_free_zerodeps(struct object *);
void 	 object_cancel_timeouts(void *, int);

int	 object_page_in(void *, enum object_page_item);
int	 object_page_out(void *, enum object_page_item);
int	 object_save(void *);
int	 object_load(void *);
int	 object_load_generic(void *);
int	 object_reload_data(void *);
int	 object_resolve_deps(void *);
int	 object_load_data(void *);

void	 object_attach(void *, void *);
int	 object_attach_path(const char *, void *);
void	 object_detach(void *);
void	 object_move(void *, void *);

struct object_dep	 *object_add_dep(void *, void *);
__inline__ int		  object_find_dep(const void *, Uint32, void **);
void			  object_del_dep(void *, const void *);
Uint32			  object_encode_name(const void *, const void *);

#ifdef EDITION
struct window	*object_edit(void *);
#endif
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_OBJECT_H */
