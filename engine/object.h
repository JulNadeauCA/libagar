/*	$Csoft: object.h,v 1.135 2005/09/27 00:25:17 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJECT_H_
#define _AGAR_OBJECT_H_

#define AG_OBJECT_TYPE_MAX	32
#define AG_OBJECT_NAME_MAX	128
#define AG_OBJECT_PATH_MAX	1024
#define AG_OBJECT_DIGEST_MAX 170

#include <engine/timeout.h>
#include <engine/prop.h>
#include <engine/gfx.h>
#include <engine/audio.h>

#include "begin_code.h"

struct ag_map;
struct ag_input;
struct ag_event;

/* Generic object operation vector */
typedef struct ag_object_ops {
	void	(*init)(void *, const char *);		/* Initialize */
	void	(*reinit)(void *);			/* Reinitialize */
	void	(*destroy)(void *);			/* Free resources */
	int	(*load)(void *, AG_Netbuf *);		/* Load from network */
	int	(*save)(void *, AG_Netbuf *);		/* Save to network */
	void   *(*edit)(void *);			/* Edit object */
} AG_ObjectOps;

/* Dependency with respect to another object. */
typedef struct ag_object_dep {
	char		 *path;		/* Unresolved object path */
	struct ag_object *obj;		/* Resolved object */
	Uint32		  count;	/* Reference count */
#define AG_OBJECT_DEP_MAX (0xffffffff-2) /* If reached, stay resident */
	TAILQ_ENTRY(ag_object_dep) deps;
} AG_ObjectDep;

/* Structure returned by object_get_classinfo(). */
typedef struct ag_object_classinfo {
	char **classes;
	struct ag_object_type **types;
	u_int nclasses;
} AG_ObjectClassInfo;

TAILQ_HEAD(ag_objectq, ag_object);

typedef struct ag_object {
	char type[AG_OBJECT_TYPE_MAX];	/* Type of object */
	char name[AG_OBJECT_NAME_MAX];	/* Identifier */
	char *save_pfx;			/* Save dir prefix */
	const AG_ObjectOps *ops;	/* Generic operations */
	int flags;
#define AG_OBJECT_RELOAD_PROPS	 0x001	/* Don't free props before load */
#define AG_OBJECT_NON_PERSISTENT 0x002	/* Never include in saves */
#define AG_OBJECT_INDESTRUCTIBLE 0x004	/* Not destructible (advisory) */
#define AG_OBJECT_DATA_RESIDENT	 0x008	/* Dynamic data is resident */
#define AG_OBJECT_PRESERVE_DEPS	 0x010	/* Preserve cnt=0 dependencies */
#define AG_OBJECT_STATIC	 0x020	/* Don't free() after detach */
#define AG_OBJECT_READONLY	 0x040	/* Disallow edition (advisory) */
#define AG_OBJECT_WAS_RESIDENT	 0x080	/* Used internally by AG_ObjectLoad() */
#define AG_OBJECT_IN_SAVE	 0x100	/* Used internally by AG_ObjectLoad() */
#define AG_OBJECT_REOPEN_ONLOAD	 0x200	/* Close and reopen editor on load */
#define AG_OBJECT_REMAIN_DATA	 0x400	/* Keep dynamic data resident */
#define AG_OBJECT_REMAIN_GFX	 0x800	/* Keep graphics resident */
#define AG_OBJECT_SAVED_FLAGS	(AG_OBJECT_RELOAD_PROPS|\
 				 AG_OBJECT_INDESTRUCTIBLE|\
				 AG_OBJECT_PRESERVE_DEPS|\
				 AG_OBJECT_READONLY|\
				 AG_OBJECT_REOPEN_ONLOAD|\
				 AG_OBJECT_REMAIN_DATA|\
				 AG_OBJECT_REMAIN_GFX)
#define AG_OBJECT_DUPED_FLAGS	(AG_OBJECT_SAVED_FLAGS|\
				 AG_OBJECT_NON_PERSISTENT|\
				 AG_OBJECT_REOPEN_ONLOAD|\
				 AG_OBJECT_REMAIN_DATA|\
				 AG_OBJECT_REMAIN_GFX)

	pthread_mutex_t lock;
	AG_Gfx	 *gfx;			/* Associated graphics package */
	AG_Audio *audio;		/* Associated audio package */
	Uint32	  data_used;		/* Dynamic data is in use */

	TAILQ_HEAD(,ag_event)	 events;	/* Event handlers */
	TAILQ_HEAD(,ag_prop)	 props;		/* Generic properties */
	CIRCLEQ_HEAD(,ag_timeout) timeouts;	/* Scheduled function calls */

	/* Uses linkage_lock */
	TAILQ_HEAD(,ag_object_dep) deps;	/* Object dependencies */
	struct ag_objectq children;		/* Child objects */
	void *parent;				/* Parent object */

	TAILQ_ENTRY(ag_object) cobjs;	/* Entry in child object queue */
	TAILQ_ENTRY(ag_object) tobjs;	/* Entry in timed object queue */
} AG_Object;

enum ag_object_page_item {
	AG_OBJECT_GFX,		/* Shared graphics */
	AG_OBJECT_AUDIO,	/* Shared audio samples */
	AG_OBJECT_DATA		/* Dynamic data */
};

enum ag_object_checksum_alg {
	AG_OBJECT_MD5,
	AG_OBJECT_SHA1,
	AG_OBJECT_RMD160
};

#define AGOBJECT_INITIALIZER(ob, type, name, ops) {		\
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

#define AGOBJECT(ob)		 ((struct ag_object *)(ob))
#define AGOBJECT_TYPE(ob, t)	 (strcmp(AGOBJECT(ob)->type,(t))==0)
#define AGOBJECT_SUBCLASS(ob, t) AG_ObjectSubclass(AGOBJECT(ob), t, \
				  sizeof(t)-1)

#define AGOBJECT_FOREACH_CHILD(var, ob, type)				\
	for((var) = (struct type *)TAILQ_FIRST(&AGOBJECT(ob)->children); \
	    (var) != (struct type *)TAILQ_END(&AGOBJECT(ob)->children); \
	    (var) = (struct type *)TAILQ_NEXT(AGOBJECT(var), cobjs))

#define AGOBJECT_FOREACH_CHILD_REVERSE(var, ob, type)			\
	for((var) = (struct type *)TAILQ_LAST(&AGOBJECT(ob)->children, \
	    ag_objectq); \
	    (var) != (struct type *)TAILQ_END(&AGOBJECT(ob)->children); \
	    (var) = (struct type *)TAILQ_PREV(AGOBJECT(var), ag_objectq, \
	    cobjs))

#define AG_ObjectLock(ob) pthread_mutex_lock(&(ob)->lock)
#define AG_ObjectUnlock(ob) pthread_mutex_unlock(&(ob)->lock)

__BEGIN_DECLS
AG_Object *AG_ObjectNew(void *, const char *);
void	   AG_ObjectAttach(void *, void *);
int	   AG_ObjectAttachPath(const char *, void *);
void	   AG_ObjectDetach(void *);
void	   AG_ObjectMove(void *, void *);

void	 AG_ObjectInit(void *, const char *, const char *, const void *);
void	 AG_ObjectFreeData(void *);
void	 AG_ObjectRemain(void *, int);
int	 AG_ObjectCopyName(const void *, char *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 3);
int	 AG_ObjectCopyDirname(const void *, char *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 3);
int	 AG_ObjectCopyFilename(const void *, char *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 3);
size_t	 AG_ObjectCopyChecksum(const void *, enum ag_object_checksum_alg,
	                       char *);
int	 AG_ObjectCopyDigest(const void *, size_t *, char *);
int	 AG_ObjectChanged(void *);
int	 AG_ObjectChangedAll(void *);

void		*AG_ObjectFind(const char *);
void		*AG_ObjectFindF(const char *, ...);
__inline__ void	*AG_ObjectRoot(const void *);
__inline__ void *AG_ObjectFindParent(void *, const char *, const char *);
int		 AG_ObjectInUse(const void *);
void		 AG_ObjectSetType(void *, const char *);
void		 AG_ObjectSetName(void *, const char *);
void		 AG_ObjectSetOps(void *, const void *);

__inline__ SDL_Surface	*AG_ObjectIcon(void *);

__inline__ int AG_ObjectSubclass(AG_Object *, const char *, size_t);
void	       AG_ObjectGetClassInfo(const char *, AG_ObjectClassInfo *);
void	       AG_ObjectFreeClassinfo(AG_ObjectClassInfo *);

void	 AG_ObjectMoveUp(void *);
void	 AG_ObjectMoveDown(void *);
void	*AG_ObjectDuplicate(void *);
void	 AG_ObjectDestroy(void *);
void	 AG_ObjectUnlinkDatafiles(void *);
void	 AG_ObjectSetSavePfx(void *, char *);

void	 AG_ObjectFreeChildren(AG_Object *);
void	 AG_ObjectFreeProps(AG_Object *);
void 	 AG_ObjectFreeEvents(AG_Object *);
void	 AG_ObjectFreeDeps(AG_Object *);
void	 AG_ObjectFreeZerodeps(AG_Object *);
void 	 AG_ObjectCancelTimeouts(void *, int);

int	 AG_ObjectPageIn(void *, enum ag_object_page_item);
int	 AG_ObjectPageOut(void *, enum ag_object_page_item);
int	 AG_ObjectSave(void *);
int	 AG_ObjectSaveAll(void *);

int	 AG_ObjectLoad(void *);
int	 AG_ObjectLoadGeneric(void *);
int	 AG_ObjectReloadData(void *);
int	 AG_ObjectResolveDeps(void *);
int	 AG_ObjectLoadData(void *);

AG_ObjectDep	*AG_ObjectAddDep(void *, void *);
__inline__ int	 AG_ObjectFindDep(const void *, Uint32, void **);
void		 AG_ObjectDelDep(void *, const void *);
Uint32		 AG_ObjectEncodeName(const void *, const void *);

#ifdef EDITION
void	*AG_ObjectEdit(void *);
#endif
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_OBJECT_H */
