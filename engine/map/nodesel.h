/*	$Csoft: nodesel.h,v 1.1 2005/07/10 15:42:17 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_NODESEL_H_
#define _AGAR_MAP_NODESEL_H_

__BEGIN_DECLS
void AG_NodeselBegin(AG_Mapview *);
void AG_NodeselEnd(AG_Mapview *);

void AG_NodeselBeginMove(AG_Mapview *);
void AG_NodeselUpdateMove(AG_Mapview *, int, int);
void AG_NodeselEndMove(AG_Mapview *);

int AG_NodeselCopy(AG_Maptool *, SDLKey, int, void *);
int AG_NodeselPaste(AG_Maptool *, SDLKey, int, void *);
int AG_NodeselCut(AG_Maptool *, SDLKey, int, void *);
int AG_NodeselKill(AG_Maptool *, SDLKey, int, void *);
__END_DECLS

#endif /* _AGAR_MAP_NODESEL_H_ */
