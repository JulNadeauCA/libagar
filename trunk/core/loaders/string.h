/*	$Csoft: string.h,v 1.3 2005/09/17 04:48:40 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
char	*AG_ReadStringLen(AG_Netbuf *, size_t);
char	*AG_ReadNulStringLen(AG_Netbuf *, size_t);
#define	 AG_ReadString(nb) AG_ReadStringLen((nb), 32767)
#define	 AG_ReadNulString(nb) AG_ReadNulStringLen((nb), 32767)
void	 AG_WriteString(AG_Netbuf *, const char *);
size_t	 AG_CopyString(char *, AG_Netbuf *, size_t);
size_t	 AG_CopyNulString(char *, AG_Netbuf *, size_t);
__END_DECLS

#include "close_code.h"
