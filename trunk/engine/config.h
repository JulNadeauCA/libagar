/*	$Csoft: config.h,v 1.15 2003/06/13 03:59:50 vedge Exp $	*/
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
void	  config_init(struct config *);
void	  config_destroy(void *);
void	  config_window(struct config *);
char	 *config_search_file(const char *, const char *, const char *);
char	**config_search_files(const char *, const char *);
__END_DECLS

#include "close_code.h"

