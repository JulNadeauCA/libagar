/*	$Csoft: command.h,v 1.1 2002/02/10 01:36:30 vedge Exp $	*/

void	mapedit_push(struct mapedit *, struct node *);
void	mapedit_pop(struct mapedit *, struct node *);
void	mapedit_clearmap(struct mapedit *);
void	mapedit_fillmap(struct mapedit *);
void	mapedit_setorigin(struct mapedit *, int *, int *);
void	mapedit_load(struct mapedit *);
void	mapedit_save(struct mapedit *);
void	mapedit_examine(struct map *, int, int);
void	mapedit_editflags(struct mapedit *, int);
void	mapedit_nodeflags(struct mapedit *, struct node *, int);

