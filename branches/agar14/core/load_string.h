/*	Public domain	*/

#define AG_LOAD_STRING_MAX 32767

#include "begin_code.h"
__BEGIN_DECLS
char	*AG_ReadStringLen(AG_DataSource *, size_t);
char	*AG_ReadNulStringLen(AG_DataSource *, size_t);
#define	 AG_ReadString(nb) AG_ReadStringLen((nb), AG_LOAD_STRING_MAX)
#define	 AG_ReadNulString(nb) AG_ReadNulStringLen((nb), AG_LOAD_STRING_MAX)
void	 AG_WriteString(AG_DataSource *, const char *);
size_t	 AG_CopyString(char *, AG_DataSource *, size_t);
size_t	 AG_CopyNulString(char *, AG_DataSource *, size_t);
__END_DECLS
#include "close_code.h"