/*	$Csoft: malloc.h,v 1.3 2004/04/21 00:15:24 vedge Exp $	*/
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
	M_MAP_NODEREF,	/* map noderefs */
	M_MAPEDIT,	/* map edition */
	M_NODEXFORM,	/* map node transform */
	M_NODEMASK,	/* map node mask */
	M_WIDGET,	/* widget */
	M_VG,		/* vector graphics */
	M_VIEW,		/* view interface */
	M_NETBUF,	/* network i/o */
	M_TTF,		/* font loader */
	M_XCF,		/* xcf image loader */
	M_DEN,		/* den archive loader */
	M_TEXT,		/* text rendering */
	M_TYPESW,	/* type switch */
	M_INPUT,	/* input devices */
	M_CAD,		/* cad applications */
	M_EDA,		/* eda applications */
	M_LAST
};

struct error_mement {
	size_t msize;			/* Total allocated memory */
	size_t rsize;			/* Total reallocated memory */
	unsigned int nallocs;		/* Total number of allocations */
	unsigned int nreallocs;		/* Total number of reallocations */
	unsigned int nfrees;		/* Total number of frees */
};

__BEGIN_DECLS
__inline__ void	*error_malloc(size_t, int);
__inline__ void	*error_realloc(void *, size_t, int);
void		 error_free(void *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_ERROR_MALLOC_H_ */
