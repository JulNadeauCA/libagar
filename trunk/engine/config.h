/*	$Csoft: config.h,v 1.18 2003/06/22 23:48:36 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct config {
	struct object obj;
	struct window	*settings;
	pthread_mutex_t	lock;
	char		*save_path;		/* Data file save path */
};

extern struct config *config;

__BEGIN_DECLS
void	  config_init(struct config *);
void	  config_destroy(void *);
void	  config_window(struct config *);
int	  config_search_file(const char *, const char *, const char *,
	                     char *, size_t);
__END_DECLS

#include "close_code.h"

