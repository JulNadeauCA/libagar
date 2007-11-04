/*	Public domain	*/

#ifndef _AGAR_MAP_MAPEDIT_H_
#define _AGAR_MAP_MAPEDIT_H_

#ifdef _AGAR_INTERNAL
#include <map/mapview.h>
#include <gui/window.h>
#include <gui/button.h>
#endif

#include "begin_code.h"

typedef struct map_editor {
	AG_Object obj;
	MAP copybuf;		/* Copy/paste buffer */
	AG_Object pseudo;	/* Pseudo-object (for depkeeping) */
} MAP_Editor;

__BEGIN_DECLS
extern const AG_ObjectOps mapEditorOps;
extern MAP_Editor mapEditor;

void	 MAP_EditorInit(void);
void	 MAP_EditorLoad(AG_DataSource *);
void	 MAP_EditorSave(AG_DataSource *);
void	*MAP_EditorConfig(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAP_MAPEDIT_H_ */
