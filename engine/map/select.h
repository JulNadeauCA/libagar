/*	$Csoft: select.h,v 1.1 2005/06/16 05:19:32 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_SELECT_H_
#define _AGAR_MAP_SELECT_H_

__BEGIN_DECLS
void select_begin_nodesel(struct mapview *);
void select_end_nodesel(struct mapview *);
void select_begin_nodemove(struct mapview *);
void select_update_nodemove(struct mapview *, int, int);
void select_end_nodemove(struct mapview *);

int select_copy_nodes(struct tool *, SDLKey, int, void *);
int select_paste_nodes(struct tool *, SDLKey, int, void *);
int select_cut_nodes(struct tool *, SDLKey, int, void *);
int select_kill_nodes(struct tool *, SDLKey, int, void *);
__END_DECLS

#endif /* _AGAR_MAP_SELECT_H_ */
