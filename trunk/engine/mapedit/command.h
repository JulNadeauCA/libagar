/*	$Csoft: command.h,v 1.6 2002/05/08 09:42:28 vedge Exp $	*/
/*	Public domain	*/

void	mapedit_push(struct mapedit *, struct node *, int, int);
void	mapedit_pop(struct mapedit *, struct node *, int);
void	mapedit_clearmap(struct mapedit *);
void	mapedit_fillmap(struct mapedit *);
void	mapedit_setorigin(struct mapedit *, int *, int *);
void	mapedit_loadmap(struct mapedit *);
void	mapedit_savemap(struct mapedit *);
void	mapedit_examine(struct map *, int, int);
void	mapedit_editflags(struct mapedit *, int);
void	mapedit_nodeflags(struct mapedit *, struct node *, Uint32);

