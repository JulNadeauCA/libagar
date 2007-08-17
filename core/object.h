/*	Public domain	*/

#ifndef _AGAR_OBJECT_H_
#define _AGAR_OBJECT_H_

#define AG_OBJECT_TYPE_MAX 32
#define AG_OBJECT_NAME_MAX 128
#define AG_OBJECT_PATH_MAX 1024
#define AG_OBJECT_DIGEST_MAX 170

#ifdef _AGAR_INTERNAL
#include <core/timeout.h>
#include <core/prop.h>
#else
#include <agar/core/timeout.h>
#include <agar/core/prop.h>
#endif

#include "begin_code.h"

struct ag_event;

/* Generic object operation vector */
typedef struct ag_object_ops {
	const char *type;				/* Class name */
	size_t size;					/* Structure size */
	AG_Version ver;					/* Version numbers */

	void (*init)(void *, const char *);		/* Initialize */
	void (*free_dataset)(void *);			/* Free dataset */
	void (*destroy)(void *);			/* Free resources */
	int (*load)(void *, AG_Netbuf *);		/* Load from network */
	int (*save)(void *, AG_Netbuf *);		/* Save to network */
	void *(*edit)(void *);				/* Edit object */
} AG_ObjectOps;

/* Dependency with respect to another object. */
typedef struct ag_object_dep {
	char		 *path;		/* Unresolved object path */
	struct ag_object *obj;		/* Resolved object */
	Uint32		  count;	/* Reference count */
#define AG_OBJECT_DEP_MAX (0xffffffff-2) /* If reached, stay resident */
	TAILQ_ENTRY(ag_object_dep) deps;
} AG_ObjectDep;

TAILQ_HEAD(ag_objectq, ag_object);

typedef struct ag_object {
	char name[AG_OBJECT_NAME_MAX];	/* Object ID (unique in parent) */
	char *archivePath;		/* Path to archive (app-specific) */
	char *save_pfx;			/* Prefix for default save paths */
	const AG_ObjectOps *ops;	/* Object class data */
	Uint flags;
#define AG_OBJECT_RELOAD_PROPS	 0x001	/* Don't free props before load */
#define AG_OBJECT_NON_PERSISTENT 0x002	/* Never include in saves */
#define AG_OBJECT_INDESTRUCTIBLE 0x004	/* Not destructible (advisory) */
#define AG_OBJECT_RESIDENT	 0x008	/* Data part is resident */
#define AG_OBJECT_PRESERVE_DEPS	 0x010	/* Preserve cnt=0 dependencies */
#define AG_OBJECT_STATIC	 0x020	/* Don't free() after detach */
#define AG_OBJECT_READONLY	 0x040	/* Disallow edition (advisory) */
#define AG_OBJECT_WAS_RESIDENT	 0x080	/* Used internally by ObjectLoad() */
#define AG_OBJECT_REOPEN_ONLOAD	 0x200	/* Recreate editor UI on ObjectLoad() */
#define AG_OBJECT_REMAIN_DATA	 0x400	/* Keep user data resident */
#define AG_OBJECT_DEBUG		 0x800	/* Enable debugging */
#define AG_OBJECT_SAVED_FLAGS	(AG_OBJECT_RELOAD_PROPS|\
 				 AG_OBJECT_INDESTRUCTIBLE|\
				 AG_OBJECT_PRESERVE_DEPS|\
				 AG_OBJECT_READONLY|\
				 AG_OBJECT_REOPEN_ONLOAD|\
				 AG_OBJECT_REMAIN_DATA|\
				 AG_OBJECT_DEBUG)
#define AG_OBJECT_DUPED_FLAGS	(AG_OBJECT_SAVED_FLAGS|\
				 AG_OBJECT_NON_PERSISTENT|\
				 AG_OBJECT_REOPEN_ONLOAD|\
				 AG_OBJECT_REMAIN_DATA)

	AG_Mutex lock;
	Uint nevents;				/* Number of event handlers */
	TAILQ_HEAD(,ag_event) events;		/* Event handlers */
	TAILQ_HEAD(,ag_prop) props;		/* Generic property table */
	CIRCLEQ_HEAD(,ag_timeout) timeouts;	/* Timers tied to object */

	/* Uses linkage_lock */
	TAILQ_HEAD(,ag_object_dep) deps; /* Object dependencies */
	struct ag_objectq children;	 /* Child objects */
	void *parent;			 /* Back reference to parent object */
	TAILQ_ENTRY(ag_object) cobjs;	 /* Entry in child object queue */
	TAILQ_ENTRY(ag_object) tobjs;	 /* Entry in timeout queue */
} AG_Object;

enum ag_object_page_item {
	AG_OBJECT_DATA			/* User data */
};

enum ag_object_checksum_alg {
	AG_OBJECT_MD5,
	AG_OBJECT_SHA1,
	AG_OBJECT_RMD160
};

#define AGOBJECT(ob)		((struct ag_object *)(ob))
#define AGOBJECT_RESIDENT(ob)	(AGOBJECT(ob)->flags & AG_OBJECT_RESIDENT)
#define AGOBJECT_PERSISTENT(ob) ((AGOBJECT(ob)->flags & \
				 AG_OBJECT_NON_PERSISTENT) == 0)
#define AGOBJECT_DEBUG(ob)	(AGOBJECT(ob)->flags & AG_OBJECT_DEBUG)

#define AGOBJECT_INITIALIZER(ob, name, pfx, ops) {		\
		(name),(pfx),(ops),0,				\
		AG_MUTEX_INITIALIZER,				\
		NULL, 0, 0, 					\
		TAILQ_HEAD_INITIALIZER((ob)->events),		\
		TAILQ_HEAD_INITIALIZER((ob)->props),		\
		CIRCLEQ_HEAD_INITIALIZER((ob)->timeouts),	\
		TAILQ_HEAD_INITIALIZER((ob)->deps),		\
		TAILQ_HEAD_INITIALIZER((ob)->children),		\
		NULL						\
	}

#define AGOBJECT_FOREACH_CHILD(var, ob, type)				\
	for((var) = (struct type *)TAILQ_FIRST(&AGOBJECT(ob)->children); \
	    (var) != (struct type *)TAILQ_END(&AGOBJECT(ob)->children); \
	    (var) = (struct type *)TAILQ_NEXT(AGOBJECT(var), cobjs))

#define AGOBJECT_FOREACH_CLASS(var, ob, type, subclass)			\
	AGOBJECT_FOREACH_CHILD(var,ob,type)				\
		if (!AG_ObjectIsClass(var,(subclass))) {		\
			continue;					\
		} else

#define AGOBJECT_FOREACH_CHILD_REVERSE(var, ob, type)			\
	for((var) = (struct type *)TAILQ_LAST(&AGOBJECT(ob)->children, \
	    ag_objectq); \
	    (var) != (struct type *)TAILQ_END(&AGOBJECT(ob)->children); \
	    (var) = (struct type *)TAILQ_PREV(AGOBJECT(var), ag_objectq, \
	    cobjs))

#ifdef _AGAR_INTERNAL
#define OBJECT(ob)		AGOBJECT(ob)
#define OBJECT_RESIDENT(ob)	AGOBJECT_RESIDENT(ob)
#define OBJECT_PERSISTENT(ob)	AGOBJECT_PERSISTENT(ob)
#define OBJECT_DEBUG(ob) 	AGOBJECT_DEBUG(ob)
#define OBJECT_INITIALIZER(obj,n,pfx,ops) \
	AGOBJECT_INITIALIZER((obj),(n),(pfx),(ops))
#define OBJECT_FOREACH_CHILD(var,ob,type) \
	AGOBJECT_FOREACH_CHILD((var),(ob),type)
#define OBJECT_FOREACH_CHILD_REVERSE(var,ob,type) \
	AGOBJECT_FOREACH_CHILD_REVERSE((var),(ob),type)
#define OBJECT_FOREACH_CLASS(var,ob,type,subclass) \
	AGOBJECT_FOREACH_CLASS((var),(ob),(type),(subclass))
#endif /* _AGAR_INTERNAL */

#define AG_ObjectLock(ob) AG_MutexLock(&(ob)->lock)
#define AG_ObjectUnlock(ob) AG_MutexUnlock(&(ob)->lock)

__BEGIN_DECLS
extern const AG_ObjectOps agObjectOps;

void	*AG_ObjectNew(void *, const char *, const AG_ObjectOps *);
void	 AG_ObjectAttach(void *, void *);
int	 AG_ObjectAttachPath(const char *, void *);
void	 AG_ObjectDetach(void *);
void	 AG_ObjectMove(void *, void *);

void	 AG_ObjectInit(void *, const char *, const void *);
void	*AG_ObjectCreate(const AG_ObjectOps *, const char *);
void	 AG_ObjectFreeDataset(void *);
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
__inline__ void	*AG_ObjectFindChild(void *, const char *);
int		 AG_ObjectInUse(const void *);
void		 AG_ObjectSetName(void *, const char *);
void		 AG_ObjectSetArchivePath(void *, const char *);
void		 AG_ObjectGetArchivePath(void *, char *, size_t)
		     BOUNDED_ATTRIBUTE(__string__, 2, 3);
void		 AG_ObjectSetOps(void *, const void *);

__inline__ SDL_Surface	*AG_ObjectIcon(void *);
__inline__ int		 AG_ObjectIsClass(void *, const char *);

void	 AG_ObjectMoveUp(void *);
void	 AG_ObjectMoveDown(void *);
void	*AG_ObjectDuplicate(void *, const char *);
void	 AG_ObjectDestroy(void *);
void	 AG_ObjectUnlinkDatafiles(void *);
void	 AG_ObjectSetSavePfx(void *, char *);

void	 AG_ObjectFreeChildren(AG_Object *);
void	 AG_ObjectFreeProps(AG_Object *);
void 	 AG_ObjectFreeEvents(AG_Object *);
void	 AG_ObjectFreeDeps(AG_Object *);
void	 AG_ObjectFreeDummyDeps(AG_Object *);
void 	 AG_ObjectCancelTimeouts(void *, Uint);

int	 AG_ObjectPageIn(void *);
int	 AG_ObjectPageOut(void *);
int	 AG_ObjectSaveToFile(void *, const char *);
#define	 AG_ObjectSave(p) AG_ObjectSaveToFile((p),NULL)
int	 AG_ObjectSaveAll(void *);

#define	 AG_ObjectLoad(p)         AG_ObjectLoadFromFile((p),NULL)
#define	 AG_ObjectLoadData(o,f)   AG_ObjectLoadDataFromFile((o),(f),NULL)
#define	 AG_ObjectLoadGeneric(p)  AG_ObjectLoadGenericFromFile((p),NULL)

int	 AG_ObjectLoadFromFile(void *, const char *);
int	 AG_ObjectLoadGenericFromFile(void *, const char *);

int	 AG_ObjectResolveDeps(void *);
int	 AG_ObjectLoadDataFromFile(void *, int *, const char *);

AG_ObjectDep	*AG_ObjectAddDep(void *, void *);
__inline__ int	 AG_ObjectFindDep(const void *, Uint32, void **);
void		 AG_ObjectDelDep(void *, const void *);
Uint32		 AG_ObjectEncodeName(const void *, const void *);
void		*AG_ObjectEdit(void *);
void		 AG_ObjectGenName(AG_Object *, const AG_ObjectOps *, char *,
		                  size_t);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_OBJECT_H */
