/*	$Csoft$	*/
/*	Public domain	*/

struct stamp {
	struct	 tool tool;
	int	 flags;
	enum {
		STAMP_REPLACE = 0,
		STAMP_INSERT_HIGHEST = 1,
		STAMP_INSERT_LOWEST = 2
	} mode;
};

struct stamp	*stamp_new(struct mapedit *, int);
void		 stamp_init(struct stamp *, struct mapedit *, int);
struct window	*stamp_window(void *);
void		 stamp_effect(void *, struct map *, Uint32, Uint32);

