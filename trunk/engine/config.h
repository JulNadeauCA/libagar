/*	$Csoft: config.h,v 1.10 2002/09/07 04:31:54 vedge Exp $	*/
/*	Public domain	*/

struct config {
	struct object	obj;
	struct window	*settings;
};

extern struct config *config;

struct config	*config_new(void);
void		 config_init(struct config *);
int		 config_load(void *, int);
int		 config_save(void *, int);
void		 config_destroy(void *);

void		 config_window(struct config *);

