/*	$Csoft: mediasel.h,v 1.1 2004/03/09 06:16:18 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MEDIASEL_H_
#define _AGAR_MEDIASEL_H_

#include <engine/object.h>
#include <engine/widget/combo.h>
#include <engine/widget/button.h>

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
struct mediasel *mediasel_new(void *, enum mediasel_type, struct object *);
void		 mediasel_refresh(struct mediasel *);
void		 mediasel_scan_dens(struct mediasel *, const char *,
		                    const char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_MEDIASEL_H */
