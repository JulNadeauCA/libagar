/*	$Csoft: mediasel.h,v 1.2 2004/03/12 04:06:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MEDIASEL_H_
#define _AGAR_MEDIASEL_H_

#include <engine/object.h>
#include <engine/widget/combo.h>
#include <engine/widget/button.h>
#include <engine/mapedit/mapview.h>

#include "begin_code.h"

enum mediasel_type {
	MEDIASEL_GFX,
	MEDIASEL_AUDIO
};

struct mediasel {
	enum mediasel_type type;
	struct object *obj;
	struct combo *com;
	struct button *rfbu;
};

__BEGIN_DECLS
void		 mediasel_init(struct mapview *);
void		 mediasel_destroy(struct mapview *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_MEDIASEL_H */
