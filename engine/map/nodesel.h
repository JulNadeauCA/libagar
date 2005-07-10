/*	$Csoft: select.h,v 1.3 2005/06/21 08:09:07 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_NODESEL_H_
#define _AGAR_MAP_NODESEL_H_

__BEGIN_DECLS
void nodesel_begin(struct mapview *);
void nodesel_end(struct mapview *);

void nodesel_begin_move(struct mapview *);
void nodesel_update_move(struct mapview *, int, int);
void nodesel_end_move(struct mapview *);

int nodesel_copy(struct tool *, SDLKey, int, void *);
int nodesel_paste(struct tool *, SDLKey, int, void *);
int nodesel_cut(struct tool *, SDLKey, int, void *);
int nodesel_kill(struct tool *, SDLKey, int, void *);
__END_DECLS

#endif /* _AGAR_MAP_NODESEL_H_ */
