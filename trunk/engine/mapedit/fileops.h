/*	$Csoft: fileops.h,v 1.3 2002/07/08 03:15:40 vedge Exp $	*/
/*	Public domain	*/

void		 fileops_new_map(int, union evarg *);
struct window	*fileops_new_map_window(struct mapedit *);
void		 fileops_load_map(int, union evarg *);
struct window	*fileops_load_map_window(struct mapedit *);
void		 fileops_save_map(int, union evarg *);
void		 fileops_revert_map(int, union evarg *);
void		 fileops_clear_map(int, union evarg *);

