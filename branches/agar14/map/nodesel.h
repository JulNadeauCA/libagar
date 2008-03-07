/*	Public domain	*/

#ifndef _AGAR_MAP_NODESEL_H_
#define _AGAR_MAP_NODESEL_H_

__BEGIN_DECLS
void MAP_NodeselBegin(MAP_View *);
void MAP_NodeselEnd(MAP_View *);

void MAP_NodeselBeginMove(MAP_View *);
void MAP_NodeselUpdateMove(MAP_View *, int, int);
void MAP_NodeselEndMove(MAP_View *);

int MAP_NodeselCopy(MAP_Tool *, SDLKey, int, void *);
int MAP_NodeselPaste(MAP_Tool *, SDLKey, int, void *);
int MAP_NodeselCut(MAP_Tool *, SDLKey, int, void *);
int MAP_NodeselKill(MAP_Tool *, SDLKey, int, void *);
__END_DECLS

#endif /* _AGAR_MAP_NODESEL_H_ */
