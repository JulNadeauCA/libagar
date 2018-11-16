/*	Public domain	*/

#ifndef _AGAR_CORE_DIR_H_
#define _AGAR_CORE_DIR_H_
#include <agar/core/begin.h>

typedef struct ag_dir {
	char *_Nullable *_Nonnull ents;
	Uint                     nents;
} AG_Dir;

__BEGIN_DECLS
int     AG_MkDir(const char *_Nonnull);
int     AG_RmDir(const char *_Nonnull);
int     AG_ChDir(const char *_Nonnull);
AG_Dir *_Nullable AG_OpenDir(const char *_Nonnull);
void    AG_CloseDir(AG_Dir *_Nonnull);
int     AG_MkPath(const char *_Nonnull);
int     AG_GetCWD(char *_Nonnull, AG_Size);
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_DIR_H_ */
