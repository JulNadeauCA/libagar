/*	$Csoft: config.h,v 1.12 2003/04/12 01:45:31 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct config {
	struct object	obj;
	struct window	*settings;
};
extern struct config *config;

__BEGIN_DECLS
extern DECLSPEC struct config	*config_new(void);
extern DECLSPEC void		 config_init(struct config *);
extern DECLSPEC int		 config_load(void *, struct netbuf *);
extern DECLSPEC int		 config_save(void *, struct netbuf *);
extern DECLSPEC void		 config_destroy(void *);
extern DECLSPEC void		 config_window(struct config *);
__END_DECLS

#include "close_code.h"

