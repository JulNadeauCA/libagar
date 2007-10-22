/*	Public domain	*/

#ifndef _AGAR_CORE_ERROR_PUB_H_
#define _AGAR_CORE_ERROR_PUB_H_
#include "begin_code.h"
enum {
	M_GENERIC,	/* generic */
	M_OBJECT,	/* object */
	M_POSITION,	/* object position */
	M_DEP,		/* dependency table entry */
	M_PROP,		/* property table entry */
	M_EVENT,	/* event queue entry */
	M_GFX,		/* gfx structure */
	M_AUDIO,	/* audio structure */
	M_MAP,		/* map nodes and layers */
	M_MAP_NITEM,	/* map noderefs */
	M_MAPEDIT,	/* map edition */
	M_NODEXFORM,	/* map node transform */
	M_NODEMASK,	/* map node mask */
	M_WIDGET,	/* widget */
	M_VG,		/* vector graphics */
	M_RG,		/* raster graphics */
	M_VIEW,		/* view interface */
	M_NETBUF,	/* network i/o */
	M_LOADER,	/* file loaders */
	M_TEXT,		/* text rendering */
	M_TYPESW,	/* type switch */
	M_INPUT,	/* input devices */
	M_CAD,		/* cad applications */
	M_EDA,		/* eda applications */
	M_GAME,		/* game applications */
	M_MATH,		/* math routines */
	M_SG,		/* scene graph */
	M_GI,		/* graphics interface */
	M_LAST
};

__BEGIN_DECLS
void		 AG_InitError(void);
void		 AG_DestroyError(void);
char		*AG_Strdup(const char *);
const char	*AG_GetError(void);
void		 AG_SetError(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_FatalError(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_DebugPrintf(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_Debug(int, const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);
void		 AG_DebugObj(void *, const char *, ...)
		     NONNULL_ATTRIBUTE(1)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);
void		 AG_DebugN(int, const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);

void		 AG_Verbose(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);

void		*AG_PtrMismatch(void);
void		*AG_ObjectMismatch(const char *, const char *);
int		 AG_IntMismatch(void);
float		 AG_FloatMismatch(void);

static __inline__ void *
AG_Malloc(size_t len, int type)
{
	void *p;
	
	if ((p = malloc(len)) == NULL) {
		AG_FatalError("malloc");
	}
	return (p);
}

static __inline__ void *
AG_Realloc(void *pOld, size_t len)
{
	void *pNew;

	/* XXX redundant on some systems */
	if (pOld == NULL) {
		if ((pNew = malloc(len)) == NULL)
			AG_FatalError("malloc");
	} else {
		if ((pNew = realloc(pOld, len)) == NULL)
			AG_FatalError("realloc");
	}
	return (pNew);
}

static __inline__ void
AG_Free(void *p, int type)
{
	/* XXX redundant on some systems */
	if (p == NULL) {
		return;
	}
	free(p);
}

__END_DECLS

#include "close_code.h"
#endif /* _AGAR_CORE_ERROR_PUB_H_ */
