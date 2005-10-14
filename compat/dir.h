/*	$Csoft: dir.h,v 1.3 2004/04/24 04:33:32 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_COMPAT_DIR_H_
#define _AGAR_COMPAT_DIR_H_
#include "begin_code.h"

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
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_COMPAT_DIR_H_ */
