/*	$Csoft: config.h,v 1.11 2002/12/24 10:28:23 vedge Exp $	*/
/*	Public domain	*/

struct config {
	struct object	obj;
	struct window	*settings;
};

extern struct config *config;

struct config	*config_new(void);
void		 config_init(struct config *);
int		 config_load(void *, struct netbuf *);
int		 config_save(void *, struct netbuf *);
void		 config_destroy(void *);

void		 config_window(struct config *);

