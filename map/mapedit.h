/*	$Csoft: mapedit.h,v 1.2 2005/09/19 01:25:18 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_MAPEDIT_H_
#define _AGAR_MAP_MAPEDIT_H_

#include <agar/map/mapview.h>

#include <agar/gui/window.h>
#include <agar/gui/button.h>

#include "begin_code.h"

typedef struct map_editor {
	AG_Object obj;
	MAP copybuf;		/* Copy/paste buffer */
	AG_Object pseudo;	/* Pseudo-object (for depkeeping) */
} MAP_Editor;

extern MAP_Editor mapEditor;

__BEGIN_DECLS
void	 MAP_EditorInit(void);
void	 MAP_EditorDestroy(void *);
void	 MAP_EditorLoad(AG_Netbuf *);
void	 MAP_EditorSave(AG_Netbuf *);
void	*MAP_EditorConfig(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAP_MAPEDIT_H_ */
