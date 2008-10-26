/*	Public domain	*/

#ifndef _AGAR_CORE_DIR_H_
#define _AGAR_CORE_DIR_H_
#include <agar/core/begin.h>

typedef struct ag_dir {
	char **ents;
	int nents;
} AG_Dir;

__BEGIN_DECLS
int	   AG_MkDir(const char *);
int	   AG_RmDir(const char *);
int	   AG_ChDir(const char *);
AG_Dir	  *AG_OpenDir(const char *);
void	   AG_CloseDir(AG_Dir *);
int	   AG_MkPath(const char *);
int	   AG_GetCWD(char *, size_t);
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_DIR_H_ */
