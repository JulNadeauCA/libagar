/*	$Csoft: config.h,v 1.14 2003/06/06 03:20:29 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct config {
	struct object	 obj;
	struct window	*settings;

	pthread_mutex_t	lock;
	char		*save_path;		/* Data file save path */
};
extern struct config *config;

__BEGIN_DECLS
extern DECLSPEC void		 config_init(struct config *);
extern DECLSPEC void		 config_destroy(void *);
extern DECLSPEC void		 config_window(struct config *);
extern DECLSPEC char		*config_search_file(const char *, const char *,
				                    const char *);
__END_DECLS

#include "close_code.h"

