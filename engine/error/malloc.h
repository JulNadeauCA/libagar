/*	$Csoft: malloc.h,v 1.10 2005/03/11 08:59:32 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ERROR_MALLOC_H_
#define _AGAR_ERROR_MALLOC_H_
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
	M_LAST
};

struct ag_malloc_type {
	size_t msize;			/* Total allocated memory */
	u_int nallocs;			/* Total number of allocations */
	u_int nfrees;			/* Total number of frees */
};

__BEGIN_DECLS
__inline__ void	*AG_Malloc(size_t, int);
__inline__ void	*AG_Realloc(void *, size_t);
void		 AG_Free(void *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_ERROR_MALLOC_H_ */
