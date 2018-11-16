/*	Public domain	*/
#ifdef _XBOX
# include <agar/core/queue_close.h>		/* Naming conflicts */
# include <xtl.h>
# include <agar/core/queue_close.h>		/* Naming conflicts */
# include <agar/core/queue.h>

typedef LONG NTSTATUS;

typedef struct {
	USHORT Length;
	USHORT MaximumLength;
	PSTR   Buffer;
} ANSI_STRING, *PANSI_STRING;

typedef struct {
    HANDLE RootDirectory;
    PANSI_STRING ObjectName;
    ULONG Attributes;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct {
	union {
		NTSTATUS Status;
		PVOID Pointer;
	};
	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

#define InitializeObjectAttributes( p, n, a, r ) { \
	(p)->RootDirectory = r;		\
	(p)->Attributes = a;		\
	(p)->ObjectName = n;		\
}

#define FILE_OPEN                    0x00000001
#define FILE_SYNCHRONOUS_IO_ALERT    0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT 0x00000020
#define FILE_DIRECTORY_FILE          0x00000001
#define FILE_NON_DIRECTORY_FILE      0x00000040
#define OBJ_CASE_INSENSITIVE         0x00000040L

/* Undocumented externals */
extern PANSI_STRING XeImageFileName;
XBOXAPI LONG WINAPI RtlInitAnsiString(OUT PANSI_STRING DestinationString, IN LPCSTR SourceString);
XBOXAPI INT WINAPI XWriteTitleInfoAndRebootA(LPVOID,LPVOID,DWORD,DWORD,LPVOID);
XBOXAPI NTSTATUS WINAPI NtOpenSymbolicLinkObject(OUT PHANDLE LinkHandle, IN POBJECT_ATTRIBUTES ObjectAttributes);
XBOXAPI NTSTATUS WINAPI NtQuerySymbolicLinkObject(IN HANDLE LinkHandle, IN OUT PANSI_STRING LinkTarget,
												  OUT PULONG ReturnedLength OPTIONAL);
XBOXAPI NTSTATUS WINAPI NtCreateFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess,
									 IN POBJECT_ATTRIBUTES ObjectAttributes,  OUT PIO_STATUS_BLOCK IoStatusBlock,
									 IN PLARGE_INTEGER AllocationSize OPTIONAL, IN ULONG FileAttributes,
									 IN ULONG ShareAccess, IN ULONG CreateDisposition, IN ULONG CreateOptions);

// Custom LAUNCH_DATA struct for external XBE execution from AG_Execute()
#define AG_LAUNCH_MAGIC 0x41474152

typedef struct {
	DWORD magic;               // Test this against AG_LAUNCH_MAGIC to know this special struct was used
	DWORD dwID;                // The Title ID of the launcher XBE
	CHAR  szLauncherXBE[256];  // The full path to the launcher XBE
	CHAR  szLaunchedXBE[256];  // The full path to the launched XBE
	CHAR  szCmdLine[MAX_LAUNCH_DATA_SIZE - 520]; // The command-line parameters
} AG_LAUNCH_DATA, *PAG_LAUNCH_DATA;

#include <agar/core/begin.h>
__BEGIN_DECLS
DWORD           AG_XBOX_GetXbeTitleId(const char *_Nullable);
char *_Nullable AG_XBOX_GetDeviceFromLogicalDrive(const char *_Nullable);
BOOL            AG_XBOX_DriveIsMounted(const char);
BOOL            AG_XBOX_PathIsValid(const char *_Nullable);
DWORD           AG_XBOX_GetLogicalDrives();
__END_DECLS
#include <agar/core/close.h>

#endif
