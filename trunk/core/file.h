/*	Public domain	*/

#ifndef _AGAR_CORE_FILE_H_
#define _AGAR_CORE_FILE_H_
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
} AG_FileInfo;

typedef struct ag_file_ext_mapping {
	const char *ext;			/* Extension */
	const char *descr;			/* Type description */
	void *cls;				/* Related Agar class */
	int editDirect;				/* Directly editable */
} AG_FileExtMapping;

__BEGIN_DECLS
extern AG_FileExtMapping *agFileExtMap;
extern Uint               agFileExtCount;

int AG_GetFileInfo(const char *, AG_FileInfo *);
int AG_GetSystemTempDir(char *, size_t)
        BOUNDED_ATTRIBUTE(__string__, 1, 2);
int AG_FileExists(const char *);
int AG_FileDelete(const char *);
const char *AG_ShortFilename(const char *);

void AG_RegisterFileExtMappings(const AG_FileExtMapping *, Uint);
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_FILE_H_ */
