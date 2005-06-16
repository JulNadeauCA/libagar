/*	$Csoft: mapview.h,v 1.8 2005/06/16 02:54:40 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_SELECT_H_
#define _AGAR_MAP_SELECT_H_

__BEGIN_DECLS
void select_begin_nodesel(struct mapview *);
void select_end_nodesel(struct mapview *);
void select_begin_nodemove(struct mapview *);
void select_update_nodemove(struct mapview *, int, int);
void select_end_nodemove(struct mapview *);

void select_copy_nodes(struct tool *, int);
void select_paste_nodes(struct tool *, int);
void select_cut_nodes(struct tool *, int);
void select_kill_nodes(struct tool *, int);
__END_DECLS

#endif /* _AGAR_MAP_SELECT_H_ */
