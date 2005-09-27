/*	$Csoft: mapedit.h,v 1.2 2005/09/19 01:25:18 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_MAPEDIT_H_
#define _AGAR_MAP_MAPEDIT_H_

#include <engine/map/mapview.h>

#include <engine/widget/window.h>
#include <engine/widget/button.h>

#include "begin_code.h"

typedef struct ag_mapeditor {
	AG_Object obj;
	AG_Map copybuf;		/* Copy/paste buffer */
	AG_Object pseudo;		/* Pseudo-object (for depkeeping) */
} AG_MapEditor;

extern AG_MapEditor agMapEditor;
extern int agEditMode;

__BEGIN_DECLS
void	 AG_MapEditorInit(void);
void	 AG_MapEditorDestroy(void *);
void	 AG_MapEditorLoad(AG_Netbuf *);
void	 AG_MapEditorSave(AG_Netbuf *);
void	*AG_MapEditorConfig(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAP_MAPEDIT_H_ */
