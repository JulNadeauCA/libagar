/*	$Csoft$	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
char	*read_string(struct netbuf *);
void	 write_string(struct netbuf *, const char *);
size_t	 copy_string(char *, struct netbuf *, size_t);
__END_DECLS

#include "close_code.h"
