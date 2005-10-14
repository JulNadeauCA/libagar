/*	$Csoft: dir.h,v 1.3 2004/04/24 04:33:32 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_COMPAT_FILE_H_
#define _AGAR_COMPAT_FILE_H_
#include "begin_code.h"

enum ag_file_info_type {
	AG_FILE_REGULAR,
	AG_FILE_DIRECTORY,
	AG_FILE_DEVICE,
	AG_FILE_FIFO,
	AG_FILE_SYMLINK,
	AG_FILE_SOCKET
};

typedef struct ag_file_info {
	enum ag_file_info_type type;
	int flags;
#define AG_FILE_SUID		0x001
#define AG_FILE_SGID		0x002
#define AG_FILE_ARCHIVE		0x004
#define AG_FILE_COMPRESSED	0x008
#define AG_FILE_ENCRYPTED	0x010
#define AG_FILE_HIDDEN		0x020
#define AG_FILE_READONLY	0x040
#define AG_FILE_REPARSE_POINT	0x080
#define AG_FILE_SPARSE		0x100
#define AG_FILE_TEMPORARY	0x200
#define AG_FILE_SYSTEM		0x400
#define AG_FILE_NOT_CONTENT_IDX	0x800
} AG_FileInfo;

__BEGIN_DECLS
int AG_GetFileInfo(const char *, AG_FileInfo *);
int AG_FileExists(const char *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_COMPAT_FILE_H_ */

