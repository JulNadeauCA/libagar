/*	$Csoft: mapedit.h,v 1.99 2004/04/10 02:43:43 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_H_
#define _AGAR_MAPEDIT_H_

#include <engine/mapedit/mapview.h>

#include <engine/widget/window.h>
#include <engine/widget/button.h>

#include "begin_code.h"

struct mapedit {
	struct object obj;
	struct map copybuf;		/* Copy/paste buffer */
	struct object pseudo;		/* Pseudo-object (for depkeeping) */
};

extern struct mapedit mapedit;
extern int mapedition;

__BEGIN_DECLS
void		 mapedit_init(void);
void		 mapedit_destroy(void *);
int		 mapedit_load(void *, struct netbuf *);
int		 mapedit_save(void *, struct netbuf *);
struct window	*mapedit_settings(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_H_ */
