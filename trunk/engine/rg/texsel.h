/*	$Csoft: texsel.h,v 1.10 2005/05/24 08:12:48 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_TEXSEL_H_
#define _AGAR_RG_TEXSEL_H_

#include <engine/widget/widget.h>
#include <engine/widget/tlist.h>

#include <engine/rg/tileset.h>

#include "begin_code.h"

struct texsel {
	struct tlist tl;		/* Superclass */
	struct tileset *tset;		/* Attached tileset */
	char texname[TEXTURE_NAME_MAX];	/* Default texture name binding */
	int flags;
};

__BEGIN_DECLS
struct texsel *texsel_new(void *, struct tileset *, int);
void texsel_init(struct texsel *, struct tileset *, int);
void texsel_scale(void *, int, int);
void texsel_destroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_RG_TEXSEL_H_ */
