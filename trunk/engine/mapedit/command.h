/*	$Csoft: command.h,v 1.7 2002/06/09 10:08:07 vedge Exp $	*/
/*	Public domain	*/

void	mapedit_push(struct mapedit *, struct node *, int, int);
void	mapedit_pop(struct mapedit *, struct node *, int);
void	mapedit_clearmap(struct mapedit *);
void	mapedit_fillmap(struct mapedit *);
void	mapedit_set_origin(struct mapedit *, int *, int *);
void	mapedit_editflags(struct mapedit *, int);
void	mapedit_nodeflags(struct mapedit *, struct node *, Uint32);

