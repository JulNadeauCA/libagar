/*	$Csoft: config.h,v 1.13 2003/04/25 09:47:05 vedge Exp $	*/
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
extern DECLSPEC struct config	*config_new(void);
extern DECLSPEC void		 config_init(struct config *);
extern DECLSPEC void		 config_destroy(void *);
extern DECLSPEC void		 config_window(struct config *);
extern DECLSPEC char		*config_search_file(const char *, const char *,
				                    const char *);
__END_DECLS

#include "close_code.h"

