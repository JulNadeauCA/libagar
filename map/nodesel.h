/*	Public domain	*/

#ifndef _AGAR_MAP_NODESEL_H_
#define _AGAR_MAP_NODESEL_H_
#include <agar/map/begin.h>

typedef struct map_nodesel_tool {
	MAP_Tool tool;				/* MAP_Tool -> MAP_NodeselTool */
	MAP mapCopy;				/* Copy Buffer for Copy/Paste */
} MAP_NodeselTool;

__BEGIN_DECLS
void MAP_NodeselBegin(MAP_View *_Nonnull);
void MAP_NodeselEnd(MAP_View *_Nonnull);

void MAP_NodeselMoveBegin(MAP_View *_Nonnull);
void MAP_NodeselMoveUpdate(MAP_View *_Nonnull, int,int);
void MAP_NodeselMoveEnd(MAP_View *_Nonnull);

int MAP_NodeselCopy(MAP_Tool *_Nonnull, AG_KeySym, int, void *_Nullable);
int MAP_NodeselPaste(MAP_Tool *_Nonnull, AG_KeySym, int, void *_Nullable);
int MAP_NodeselCut(MAP_Tool *_Nonnull, AG_KeySym, int, void *_Nullable);
int MAP_NodeselClear(MAP_Tool *_Nonnull, AG_KeySym, int, void *_Nullable);
__END_DECLS

#include <agar/map/close.h>
#endif /* _AGAR_MAP_NODESEL_H_ */
