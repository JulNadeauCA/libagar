/*	Public domain	*/

#ifndef _AGAR_WIDGET_FILE_DLG_H_
#define _AGAR_WIDGET_FILE_DLG_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/window.h>
#include <agar/gui/tlist.h>
#include <agar/gui/combo.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/pane.h>
#include <agar/gui/label.h>

#include <agar/gui/begin.h>

struct ag_file_dlg;

enum ag_file_type_option_type {
	AG_FILEDLG_BOOL,
	AG_FILEDLG_INT,
	AG_FILEDLG_FLOAT,
	AG_FILEDLG_DOUBLE,
	AG_FILEDLG_STRING
};

typedef struct ag_file_type_option {
	char *_Nonnull descr;			/* Display text */
	char *_Nonnull key;			/* Option identifier string */
	char *_Nullable unit;			/* Unit for AG_Numerical(3) */
	enum ag_file_type_option_type type;
	Uint32 _pad;
	AG_TAILQ_ENTRY(ag_file_type_option) opts;
	union {
		struct { int val, min, max; } i;
		struct { float val, min, max; } flt;
		struct { double val, min, max; } dbl;
		char s[AG_SMALL_LABEL_MAX];
	} data;
} AG_FileOption;

typedef struct ag_file_type {
	struct ag_file_dlg *_Nonnull fd;   /* Back pointer to FileDlg */
	char *_Nonnull descr;              /* Display text */
	char *_Nonnull allExts;            /* Filename extensions/patterns */
	char *_Nonnull *_Nonnull exts;     /* Extensions/patterns as an array */
	Uint                    nExts;
	Uint32 _pad;
	AG_Event *_Nullable action;                /* Load/save callback */
	AG_TAILQ_HEAD_(ag_file_type_option) opts;  /* Type-specific options */
	AG_TAILQ_ENTRY(ag_file_type) types;
} AG_FileType;

typedef struct ag_file_dlg {
	AG_Widget wid;
	Uint flags;
#define AG_FILEDLG_MULTI           0x0001 /* Return a set of files */
#define AG_FILEDLG_CLOSEWIN        0x0002 /* Close parent window */
#define AG_FILEDLG_LOAD            0x0004 /* File must exist and be readable */
#define AG_FILEDLG_SAVE            0x0008 /* File must be writeable */
#define AG_FILEDLG_ASYNC           0x0010 /* Separate thread for load/save fn */
#define AG_FILEDLG_RESET_ONSHOW    0x0020 /* Reset listing/locations on show */
#define AG_FILEDLG_NOBUTTONS       0x0040 /* No OK/Cancel buttons */
#define AG_FILEDLG_MASK_EXT        0x0080 /* Mask files by extension */
#define AG_FILEDLG_MASK_HIDDEN     0x0100 /* Mask hidden files */
#define AG_FILEDLG_NOMASKOPTS      0x0200 /* No "Mask files" checkboxes */
#define AG_FILEDLG_NOTYPESELECT    0x0400 /* No "Type" dropbox */
#define AG_FILEDLG_COMPACT         0x0800 /* Compact (one-line) mode */
#define AG_FILEDLG_HFILL           0x4000
#define AG_FILEDLG_VFILL           0x8000
#define AG_FILEDLG_EXPAND          (AG_FILEDLG_HFILL|AG_FILEDLG_VFILL)
#define AG_FILEDLG_INHERITED_FLAGS (AG_FILEDLG_MULTI | AG_FILEDLG_LOAD | \
				    AG_FILEDLG_SAVE | AG_FILEDLG_ASYNC | \
				    AG_FILEDLG_RESET_ONSHOW | AG_FILEDLG_NOBUTTONS | \
				    AG_FILEDLG_MASK_EXT | AG_FILEDLG_MASK_HIDDEN | \
				    AG_FILEDLG_NOMASKOPTS | AG_FILEDLG_NOTYPESELECT) 

	char cwd[AG_PATHNAME_MAX];		/* Current working directory */
	char cfile[AG_PATHNAME_MAX];		/* Current file path */
	int fdDir;				/* Open directory FD */

	AG_EventSink *_Nullable esFollow;	/* Open directory event sink */
	AG_Pane *_Nonnull hPane;		/* Horizontal split container */
	AG_Tlist *_Nonnull tlDirs;		/* List of directories */
	AG_Tlist *_Nonnull tlFiles;		/* List of files */
	AG_Label  *_Nonnull lbCwd;		/* "Current directory" label */
	AG_Textbox *_Nonnull tbFile;		/* Manual file/dir/glob entry */
	AG_Combo   *_Nullable comTypes;		/* File types and extensions */
	AG_Checkbox *_Nullable cbMaskExt;	/* "Mask files by extension" */
	AG_Checkbox *_Nullable cbMaskHidden;	/* "Mask hidden files" */
	AG_Button *_Nullable btnOk;		/* "OK" button */
	AG_Button *_Nullable btnCancel;		/* "Cancel" button */
	AG_Event *_Nullable okAction;		/* "OK" callback */
	AG_Event *_Nullable cancelAction;	/* "Cancel" callback */

	char *_Nullable dirMRU;			/* MRU directory */
	void *_Nullable optsCtr;		/* Extra "Option Container" */

	AG_FileType *_Nullable curType;		/* Selected file type */
	AG_Combo *_Nonnull comLoc;		/* Locations and Shortcuts */
	AG_Textbox *_Nullable textbox;		/* Compact-mode Textbox */
	AG_Button *_Nullable btnExpand;		/* Compact-mode "..." button */
	AG_Window *_Nullable winExpand;		/* Expanded window from compact */
	struct ag_file_dlg *_Nullable fdExpand; /* Instance in expanded window */

	AG_TAILQ_HEAD_(ag_file_type) types;	/* File type handlers */
} AG_FileDlg;

#define AGFILEDLG(obj)            ((AG_FileDlg *)(obj))
#define AGCFILEDLG(obj)           ((const AG_FileDlg *)(obj))
#define AG_FILEDLG_SELF()          AGFILEDLG( AG_OBJECT(0,"AG_Widget:AG_FileDlg:*") )
#define AG_FILEDLG_PTR(n)          AGFILEDLG( AG_OBJECT((n),"AG_Widget:AG_FileDlg:*") )
#define AG_FILEDLG_NAMED(n)        AGFILEDLG( AG_OBJECT_NAMED((n),"AG_Widget:AG_FileDlg:*") )
#define AG_CONST_FILEDLG_SELF()   AGCFILEDLG( AG_CONST_OBJECT(0,"AG_Widget:AG_FileDlg:*") )
#define AG_CONST_FILEDLG_PTR(n)   AGCFILEDLG( AG_CONST_OBJECT((n),"AG_Widget:AG_FileDlg:*") )
#define AG_CONST_FILEDLG_NAMED(n) AGCFILEDLG( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_FileDlg:*") )

__BEGIN_DECLS
extern AG_WidgetClass agFileDlgClass;

AG_FileDlg *_Nonnull AG_FileDlgNew(void *_Nullable, Uint);
AG_FileDlg *_Nonnull AG_FileDlgNewMRU(void *_Nullable, const char *_Nonnull,
                                      Uint);
AG_FileDlg *_Nonnull AG_FileDlgNewCompact(void *_Nullable, const char *_Nonnull,
                                          Uint);
AG_FileDlg *_Nonnull AG_FileDlgNewCompactMRU(void *_Nullable,
                                             const char *_Nullable,
                                             const char *_Nonnull, Uint);

void AG_FileDlgSetOptionContainer(AG_FileDlg *_Nonnull, void *_Nullable);

char   *AG_FileDlgGetFilename(AG_FileDlg *_Nonnull);
char   *AG_FileDlgGetDirectory(AG_FileDlg *_Nonnull);
AG_Size AG_FileDlgCopyFilename(AG_FileDlg *_Nonnull, char *_Nonnull, AG_Size);
AG_Size AG_FileDlgCopyDirectory(AG_FileDlg *_Nonnull, char *_Nonnull, AG_Size);

int  AG_FileDlgSetDirectoryS(AG_FileDlg *_Nonnull, const char *_Nonnull);
int  AG_FileDlgSetDirectory(AG_FileDlg *_Nonnull, const char *_Nonnull, ...)
                            FORMAT_ATTRIBUTE(printf,2,3);
void AG_FileDlgSetDirectoryMRU(AG_FileDlg *_Nonnull, const char *_Nonnull,
                               const char *_Nonnull);

void AG_FileDlgSetFilenameS(AG_FileDlg *_Nonnull, const char *_Nonnull);
void AG_FileDlgSetFilename(AG_FileDlg *_Nonnull, const char *_Nonnull, ...)
                          FORMAT_ATTRIBUTE(printf,2,3);

void AG_FileDlgOkAction(AG_FileDlg *_Nonnull,
                        _Nonnull AG_EventFn, const char *_Nullable, ...);

void AG_FileDlgCancelAction(AG_FileDlg *_Nonnull,
                            _Nonnull AG_EventFn, const char *_Nullable, ...);

int  AG_FileDlgCheckReadAccess(AG_FileDlg *_Nonnull);
int  AG_FileDlgCheckWriteAccess(AG_FileDlg *_Nonnull);
void AG_FileDlgRefresh(AG_FileDlg *_Nonnull);

void AG_FileDlgAddImageTypes(AG_FileDlg *, AG_EventFn, const char *, ...);

void AG_FileDlgCopyTypes(AG_FileDlg *_Nonnull _Restrict,
                         const AG_FileDlg *_Nonnull _Restrict);

AG_FileType *_Nonnull AG_FileDlgAddType(AG_FileDlg *_Nonnull, const char *_Nonnull,
					const char *_Nonnull,
                                        _Nullable AG_EventFn,
					const char *_Nullable, ...);
			       
AG_FileOption *_Nonnull AG_FileOptionNewBool(AG_FileType *_Nonnull,
                                             const char *_Nonnull,
					     const char *_Nonnull, int);

AG_FileOption *_Nonnull AG_FileOptionNewInt(AG_FileType *_Nonnull,
                                            const char *_Nonnull,
					    const char *_Nonnull,
					    int, int,int);

AG_FileOption *_Nonnull AG_FileOptionNewFlt(AG_FileType *_Nonnull,
                                            const char *_Nonnull,
					    const char *_Nonnull,
					    float, float,float,
					    const char *_Nullable);

AG_FileOption *_Nonnull AG_FileOptionNewDbl(AG_FileType *_Nonnull,
                                            const char *_Nonnull,
					    const char *_Nonnull,
                                            double, double,double,
					    const char *_Nullable);

AG_FileOption *_Nonnull AG_FileOptionNewString(AG_FileType *_Nonnull,
                                               const char *_Nonnull,
					       const char *_Nonnull,
                                               const char *_Nonnull);

AG_FileOption *_Nullable AG_FileOptionGet(AG_FileType *_Nonnull,
                                          const char *_Nonnull)
                                         _Pure_Attribute;

int AG_FileOptionInt(AG_FileType *_Nonnull, const char *_Nonnull)
                    _Pure_Attribute_If_Unthreaded;
int AG_FileOptionBool(AG_FileType *_Nonnull, const char *_Nonnull)
                     _Pure_Attribute_If_Unthreaded;

float  AG_FileOptionFlt(AG_FileType *_Nonnull, const char *_Nonnull)
                       _Pure_Attribute_If_Unthreaded;
double AG_FileOptionDbl(AG_FileType *_Nonnull, const char *_Nonnull)
                       _Pure_Attribute_If_Unthreaded;

char *_Nonnull AG_FileOptionString(AG_FileType *_Nonnull, const char *_Nonnull)
                                  _Pure_Attribute_If_Unthreaded;
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_FILE_DLG_H_ */
