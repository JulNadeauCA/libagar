/*	Public domain	*/

#ifndef	_AGAR_CORE_LOAD_STRING_H_
#define	_AGAR_CORE_LOAD_STRING_H_

#define AG_LOAD_STRING_MAX 32767

#include <agar/core/begin.h>

__BEGIN_DECLS
char	*AG_ReadStringLen(AG_DataSource *, size_t);
int	 AG_ReadStringLenv(AG_DataSource *, size_t, char **);
char	*AG_ReadNulStringLen(AG_DataSource *, size_t);
#define	 AG_ReadString(nb) \
	 AG_ReadStringLen((nb),AG_LOAD_STRING_MAX)
#define	 AG_ReadStringv(nb,s) \
	 AG_ReadStringLenv((nb),AG_LOAD_STRING_MAX,(s))
#define	 AG_ReadNulString(nb) \
	 AG_ReadNulStringLen((nb),AG_LOAD_STRING_MAX)

void	 AG_WriteString(AG_DataSource *, const char *);
int	 AG_WriteStringv(AG_DataSource *, const char *);
size_t	 AG_CopyString(char *, AG_DataSource *, size_t);
size_t	 AG_CopyNulString(char *, AG_DataSource *, size_t);
void	 AG_SkipString(AG_DataSource *);
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_LOAD_STRING_H_ */
