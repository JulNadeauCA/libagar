/*	$Csoft: mapedit.h,v 1.1 2005/04/14 06:19:40 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_MAPEDIT_H_
#define _AGAR_MAP_MAPEDIT_H_

#include <engine/map/mapview.h>

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
void	 mapedit_init(void);
void	 mapedit_destroy(void *);
void	 mapedit_load(struct netbuf *);
void	 mapedit_save(struct netbuf *);
void	*mapedit_settings(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAP_MAPEDIT_H_ */
