/*	$Csoft: string.h,v 1.1 2003/06/19 01:53:38 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
char	*read_string_len(struct netbuf *, size_t);
#define	 read_string(nb) read_string_len((nb), 32767)
void	 write_string(struct netbuf *, const char *);
size_t	 copy_string(char *, struct netbuf *, size_t);
__END_DECLS

#include "close_code.h"
