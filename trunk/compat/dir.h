/*	$Csoft: dir.h,v 1.2 2004/04/23 11:45:29 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_COMPAT_DIR_H_
#define _AGAR_COMPAT_DIR_H_
#include "begin_code.h"

#ifdef WIN32
#include <windows.h>
#undef SLIST_ENTRY
#undef INPUT_KEYBOARD
#undef INPUT_MOUSE
#endif

struct compat_dirent {
#ifdef WIN32
	WIN32_FIND_DATA ent;
#else
	struct dirent *ent;
#endif
	char *name;
};

struct compat_dir {
#ifdef WIN32
	HANDLE dir;
	WIN32_FIND_DATA ent;
#else
	DIR *dir;
	struct dirent *ent;
#endif
};

#define Mkdir(d) compat_mkdir(d)
#define Rmdir(d) compat_rmdir(d)

int	compat_mkdir(const char *);
int	compat_rmdir(const char *);

#if 0
int		      compat_opendir(const char *, struct compat_dir *);
struct compat_dirent *compat_readdir(struct compat_dir *);
int		      compat_closedir(struct compat_dir *);
#endif

#include "close_code.h"
#endif /* _AGAR_COMPAT_DIR_H_ */
