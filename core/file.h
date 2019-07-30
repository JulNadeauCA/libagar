/*	Public domain	*/

#ifndef _AGAR_CORE_FILE_H_
#define _AGAR_CORE_FILE_H_

#include <agar/config/ag_serialization.h>
#ifdef AG_SERIALIZATION

#include <agar/core/begin.h>

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
	Uint perms;
#define AG_FILE_READABLE	0x01
#define AG_FILE_WRITEABLE	0x02
#define AG_FILE_EXECUTABLE	0x04
	Uint flags;
#define AG_FILE_SUID		0x001
#define AG_FILE_SGID		0x002
#define AG_FILE_ARCHIVE		0x004
#define AG_FILE_HIDDEN		0x020
#define AG_FILE_TEMPORARY	0x100
#define AG_FILE_SYSTEM		0x200
	Uint32 _pad;
} AG_FileInfo;

typedef struct ag_file_ext_mapping {
	const char *_Nonnull ext;		/* Extension */
	const char *_Nonnull descr;		/* Type description */
	void *_Nullable cls;			/* Related Agar class */
	int editDirect;				/* Directly editable */
	Uint32 _pad;
} AG_FileExtMapping;

__BEGIN_DECLS
extern AG_FileExtMapping *_Nullable agFileExtMap;
extern Uint                         agFileExtCount;

int AG_GetFileInfo(const char *_Nonnull, AG_FileInfo *_Nonnull);
int AG_GetSystemTempDir(char *_Nonnull, AG_Size);
int AG_FileExists(const char *_Nonnull);
int AG_FileDelete(const char *_Nonnull);

const char *_Nonnull AG_ShortFilename(const char *_Nonnull);

void AG_RegisterFileExtMappings(const AG_FileExtMapping *_Nonnull, Uint);
__END_DECLS

#include <agar/core/close.h>
#endif /* AG_SERIALIZATION */
#endif /* _AGAR_CORE_FILE_H_ */
