/*	$Csoft: config.h,v 1.16 2003/06/17 23:30:42 vedge Exp $	*/
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
char	 *config_search_file(const char *, const char *, const char *);
char	**config_search_files(const char *, const char *);
__END_DECLS

#include "close_code.h"

