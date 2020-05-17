/*	Public domain	*/

#include <agar/map/begin.h>

typedef struct map_editor {
	AG_Object obj;
	MAP copybuf;		/* Copy/paste buffer */
} MAP_Editor;

__BEGIN_DECLS
extern AG_ObjectClass mapEditorClass;
extern AG_ObjectClass mapEditorPseudoClass;
extern MAP_Editor mapEditor;

void MAP_EditorInit(void);
void MAP_EditorLoad(AG_DataSource *_Nonnull);
void MAP_EditorSave(AG_DataSource *_Nonnull);
__END_DECLS

#include <agar/map/close.h>
