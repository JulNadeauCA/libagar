/*
 * Copyright (c) 2005-2020 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * File browser widget. It shows directories and shortcuts on the left and
 * those files contained in the selected directory on the right. It provides
 * a glob(3)-supporting textbox for manual input and a file-type selector.
 */

#include <agar/core/core.h>
#if defined(AG_WIDGETS)

#include <agar/core/config.h>

#include <agar/gui/file_dlg.h>
#include <agar/gui/box.h>
#include <agar/gui/numerical.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/separator.h>
#include <agar/gui/icons.h>

#ifdef __NetBSD__
#define _NETBSD_SOURCE
#endif

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#ifdef _XBOX
# include <agar/core/xbox.h>
#elif _WIN32
# include <agar/core/win32.h>
#else
# include <sys/types.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
#endif

#include <agar/gui/file_dlg_common.h>

#include <agar/config/have_jpeg.h>
#include <agar/config/have_png.h>

static int  FileIsExecutable(AG_FileDlg *_Nonnull, char *_Nonnull);
static void RefreshListing(AG_FileDlg *_Nonnull);

void
AG_FileDlgSetOptionContainer(AG_FileDlg *fd, void *ctr)
{
	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	AG_ObjectLock(fd);

	fd->optsCtr = ctr;

	AG_ObjectUnlock(fd);
}

/*
 * Filter files by:
 *
 * - by filename extension:   ".txt"
 * - by executability:        "<-x>"
 * - exact filename match:    "<=hello.txt>"
 * - case-insensitive match:  "<=LICENSE/i>"
 * - TODO regular expression: "</image.(png|jpeg)/>"
 */
static int
FilterByExtension(AG_FileDlg *_Nonnull fd, char *_Nonnull file)
{
	char *ext, *s;
	AG_FileType *ft;
	Uint i;

	ext = strrchr(file, '.');

	TAILQ_FOREACH(ft, &fd->types, types) {
		for (i = 0; i < ft->nExts; i++) {
			const char *ftExt = ft->exts[i];

			if (ftExt[0] == '.' && ext &&
			    (s = strrchr(ftExt, '.')) != NULL &&
			    Strcasecmp(s, ext) == 0) {
				break;
			}
			if (strcmp(ftExt, "<-x>") == 0 &&
			    FileIsExecutable(fd, file)) {
				break;
			}
			/* Match "<=exact>" or "<=case-insensitive/i>" name */
			if (ftExt[0] == '<' && ftExt[1] == '=') {
				char pat[AG_FILENAME_MAX], *c;
				AG_Size patLen;
				
				Strlcpy(pat, &ftExt[2], sizeof(pat));
				if ((c = strrchr(pat, '>')) != NULL) {
					*c = '\0';
					patLen = strlen(pat);
					if (patLen > 2 &&
					    pat[patLen-1] == 'i' &&
					    pat[patLen-2] == '/') {
						pat[patLen-2] = '\0';
						if (Strcasecmp(file, pat) == 0)
							break;
					} else {
						if (strcmp(file, pat) == 0)
							break;
					}
				}
			}

			/* TODO: Regular expression */
		}
		if (i < ft->nExts)
			break;
	}
	return (ft == NULL);
}

/* Filter executable files (handle special "<-x>" attribute) */
static int
FileIsExecutable(AG_FileDlg *_Nonnull fd, char *_Nonnull file)
{
	char path[AG_PATHNAME_MAX];
	AG_FileInfo inf;

	Strlcpy(path, fd->cwd, sizeof(path));
	if (path[strlen(path) - 1] != AG_PATHSEPCHAR) {
		Strlcat(path, AG_PATHSEP, sizeof(path));
	}
	Strlcat(path, file, sizeof(path));

	AG_GetFileInfo(path, &inf);
	return (inf.perms & AG_FILE_EXECUTABLE);
}

static void
ExpandedFileSelect(AG_Event *_Nonnull event)
{
	char path[AG_PATHNAME_MAX];
	AG_FileDlg *fdExpand = AG_FILEDLG_SELF();
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);

	AG_FileDlgCopyFilename(fdExpand, path, sizeof(path));
	AG_TextboxSetString(fd->textbox, path);
	AG_TextboxSetCursorPos(fd->textbox, strlen(path));
	AG_WidgetFocus(fd->textbox);
	
	/* FileDlg will close the window for us (AG_FILEDLG_CLOSEWIN) */
	fd->winExpand = NULL;
	fd->fdExpand = NULL;
}

static void
CollapseFromExpanded(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);
	
	fd->fdExpand = NULL;
	fd->winExpand = NULL;

	AG_ObjectDetach(win);
}

/* Expand a FileDlg in COMPACT mode */
static void
ExpandFromCompact(AG_Event *_Nonnull event)
{
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);
	AG_Window *win = fd->winExpand;

	if (win) {
		AG_ObjectDetach(win);
		fd->winExpand = NULL;
		fd->fdExpand = NULL;
	}
	if ((fd->winExpand = win = AG_WindowNew(0)) == NULL)
		return;

	AG_WindowSetCaption(win, _("Please select a file"));

	fd->fdExpand = AG_FileDlgNew(win,
	    (fd->flags & AG_FILEDLG_INHERITED_FLAGS) |
	    (AG_FILEDLG_CLOSEWIN | AG_FILEDLG_EXPAND));

	AG_FileDlgCopyTypes(fd->fdExpand, fd);

	AG_SetEvent(fd->fdExpand, "file-chosen", ExpandedFileSelect, "%p", fd);
	AG_SetEvent(win, "window-close", CollapseFromExpanded, "%p", fd);
	AG_WindowShow(win);
}

static int
OnDirectoryEvent(AG_EventSink *_Nonnull es, AG_Event *_Nonnull event)
{
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);
	AG_Dir *dir = AG_PTR(2);

	if (fd->fdDir != -1) {
		AG_Debug(fd, "Directory event (dir = %d)\n", fd->fdDir);
		if (fd->esFollow) {
			AG_DelEventSink(fd->esFollow);
			fd->esFollow = NULL;
		}
		fd->fdDir = -1;
		AG_CloseDir(dir);
	}
	RefreshListing(fd);
	return (0);
}

/* Update the file / directory listing */
static void
RefreshListing(AG_FileDlg *_Nonnull fd)
{
	AG_TlistItem *it;
	AG_FileInfo info;
	AG_Dir *dir;
	char **dirs, **files;
	Uint i, nDirs=0, nFiles=0;

	if (fd->tlDirs == NULL /* || fd->tlFiles == NULL */) {
		return;
	}
	if ((dir = AG_OpenDir(fd->cwd)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", fd->cwd, AG_GetError());
		return;
	}
	if (dir->fd != -1) {
		fd->fdDir = dir->fd;
		if (fd->esFollow) {
			AG_DelEventSink(fd->esFollow);
		}
		fd->esFollow = AG_AddEventSink(AG_SINK_FSEVENT, dir->fd,
		    (AG_FSEVENT_WRITE | AG_FSEVENT_DELETE | AG_FSEVENT_LINK |
		     AG_FSEVENT_RENAME | AG_FSEVENT_REVOKE),
		    OnDirectoryEvent, "%p,%p", fd, dir);
		AG_Debug(fd, "Following %s (es=%p)\n", fd->cwd, fd->esFollow);
	}
	
	dirs = Malloc(sizeof(char *));
	files = Malloc(sizeof(char *));
	
	AG_ObjectLock(fd->tlDirs);
	AG_ObjectLock(fd->tlFiles);

	for (i = 0; i < dir->nents; i++) {
		char *ent = dir->ents[i];
		char path[AG_FILENAME_MAX];
		
		Strlcpy(path, fd->cwd, sizeof(path));
		if (path[strlen(path) - 1] != AG_PATHSEPCHAR) {
			Strlcat(path, AG_PATHSEP, sizeof(path));
		}
		Strlcat(path, ent, sizeof(path));

		if (AG_PathIsFilesystemRoot(fd->cwd) &&
		    strcmp(ent, "..")==0) {
			continue;
		}
		if (AG_GetFileInfo(path, &info) == -1) {
			continue;
		}
		/* XXX TODO: check for symlinks to directories */
		if (info.type == AG_FILE_DIRECTORY) {
			dirs = Realloc(dirs, (nDirs + 1) * sizeof(char *));
			dirs[nDirs++] = Strdup(ent);
		} else {
			AG_Size entLen;
			char *s;

			if (fd->flags & AG_FILEDLG_MASK_HIDDEN &&
			    ent[0] == '.') {
				continue;
			}
			if (fd->flags & AG_FILEDLG_MASK_EXT &&
			    FilterByExtension(fd, ent)) {
				continue;
			}
			files = Realloc(files, (nFiles+1)*sizeof(char *));
			entLen = strlen(ent);
			s = files[nFiles] = Malloc(entLen+2);
			memcpy(s, ent, entLen);

			/* Tack on a character to remember the perm mode. */
			if (info.perms & AG_FILE_EXECUTABLE) {
				s[entLen] = 'x';
			} else if ((info.perms & AG_FILE_READABLE) == 0) {
				s[entLen] = 'R';
			} else {
				s[entLen] = '?';
			}
			s[entLen+1] = '\0';
			nFiles++;
		}
	}
	qsort(dirs, nDirs, sizeof(char *), AG_FilenameCompare);
	qsort(files, nFiles, sizeof(char *), AG_FilenameCompare);

	AG_TlistClear(fd->tlDirs);
	AG_TlistClear(fd->tlFiles);
	for (i = 0; i < nDirs; i++) {
		char *s = dirs[i];

		it = AG_TlistAddS(fd->tlDirs, agIconDirectory.s, s);
		it->cat = "dir";
		it->p1 = it;
		free(s);
	}
	for (i = 0; i < nFiles; i++) {
		char *s = files[i];
		char *pAttr = &s[strlen(s)-1], attr = *pAttr;

		*pAttr = '\0';
		it = AG_TlistAddS(fd->tlFiles, agIconDoc.s, s);
		it->cat = "file";
		it->p1 = it;
		switch (attr) {
		case 'x':
			it->fontFlags |= AG_FONT_BOLD;
			break;
		case 'R':
			it->fontFlags |= AG_FONT_ITALIC;
			break;
		}
		free(s);
	}
	free(dirs);
	free(files);
	AG_TlistRestore(fd->tlDirs);
	AG_TlistRestore(fd->tlFiles);

	AG_TlistScrollToStart(fd->tlFiles);
	
	AG_ObjectUnlock(fd->tlFiles);
	AG_ObjectUnlock(fd->tlDirs);
}

/* Update the shortcuts. */
static void
RefreshShortcuts(AG_FileDlg *_Nonnull fd, int init)
{
	AG_Tlist *tl;

	if (fd->comLoc == NULL)
		return;

	tl = fd->comLoc->list;

	AG_TlistClear(tl);
#ifdef _WIN32
	{
		char path[4];
		int drive;
# ifdef _XBOX
		DWORD d = AG_XBOX_GetLogicalDrives();
# else
		DWORD d = GetLogicalDrives();
# endif
		/* Add the Windows drives */
		for (drive = 0; drive < 26; drive++) {
			AG_TlistItem *ti;

			if (!(d & (1 << drive))) {
				continue;
			}
			path[0] = 'A'+drive;
			path[1] = ':';
			path[2] = '\\';
			path[3] = '\0';
			ti = AG_TlistAddS(tl, agIconDirectory.s, path);

			if (init &&
			    toupper(fd->cwd[0]) == path[0] &&
			    fd->cwd[1] == ':') {
				AG_ComboSelect(fd->comLoc, ti);
			}
#if 0
			/* TODO icons, etc */
			switch (GetDriveType(path)) {
			case DRIVE_UNKNOWN:
			case DRIVE_NO_ROOT_DIR:
			case DRIVE_REMOVABLE:
			case DRIVE_FIXED:
			case DRIVE_REMOTE:
			case DRIVE_CDROM:
			case DRIVE_RAMDISK:
				break;
			}
#endif
		}
	}
#else /* !_WIN32 */
	{
		char path[AG_PATHNAME_MAX];
		AG_ConfigPath *loadPath;
		AG_User *sysUser;
	
		AG_TlistAddS(tl, agIconDirectory.s, "/");

		if ((sysUser = AG_GetRealUser()) != NULL) {
			AG_TlistAddS(tl, agIconDirectory.s, sysUser->home);
			AG_UserFree(sysUser);
		}
		if (AG_GetCWD(path, sizeof(path)) == 0) {
			AG_TlistAddS(tl, agIconDirectory.s, path);
		}
		if (fd->flags & AG_FILEDLG_SAVE) {
			if (AG_ConfigGetPath(AG_CONFIG_PATH_DATA, 0,
			    path, sizeof(path)) < sizeof(path))
				AG_TlistAddS(tl, agIconDirectory.s, path);
		} else {
			AG_ConfigPathQ *pathGroup =
			    &agConfig->paths[AG_CONFIG_PATH_DATA];

			TAILQ_FOREACH(loadPath, pathGroup, paths)
				AG_TlistAddS(tl, agIconDirectory.s, loadPath->s);
		}
		AG_ComboSelectText(fd->comLoc, fd->cwd);
	}
	AG_TlistUniq(tl);
#endif /* _WIN32 */

	AG_TlistRestore(tl);
}

void
AG_FileDlgRefresh(AG_FileDlg *fd)
{
	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	RefreshListing(fd);
	RefreshShortcuts(fd, 0);
}

static void
DirSelected(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);
	AG_TlistItem *ti;

	AG_ObjectLock(fd);
	AG_ObjectLock(tl);

	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		if (AG_FileDlgSetDirectoryS(fd, ti->text) == -1) {
			/* AG_TextMsgFromError() */
		} else {
			AG_PostEvent(fd, "dir-selected", "%s", fd->cwd);
			RefreshListing(fd);
		}
	}

	AG_ObjectUnlock(tl);
	AG_ObjectUnlock(fd);
}

static void
LocSelected(AG_Event *_Nonnull event)
{
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);
	const AG_TlistItem *ti = AG_TLIST_ITEM_PTR(2);

	if (ti == NULL) {
		return;
	}
	if (AG_FileDlgSetDirectoryS(fd, ti->text) == -1) {
		/* AG_TextMsgFromError() */
	} else {
		AG_PostEvent(fd, "dir-selected", "%s", fd->cwd);
		RefreshListing(fd);
	}
}

static void
ChooseFile(AG_FileDlg *_Nonnull fd, AG_Window *_Nonnull pwin)
{
	AG_TlistItem *it;
	AG_FileType *ft = NULL;
	char *ext;
	int i;

	AG_ObjectLock(fd);

	if (fd->comTypes &&
	    (it = AG_TlistSelectedItem(fd->comTypes->list)) != NULL) {
		ft = it->p1;
	} else if ((ext = strrchr(fd->cfile, '.')) != NULL) {
		TAILQ_FOREACH(ft, &fd->types, types) {
			for (i = 0; i < ft->nExts; i++) {
				char *s;
				if ((s = strrchr(ft->exts[i], '.')) != NULL &&
				    Strcasecmp(s, ext) == 0)
					break;
			}
			if (i < ft->nExts)
				break;
		}
	}
	if (ft && ft->action) {
		AG_PostEventByPtr(fd, ft->action, "%s,%p", fd->cfile, ft);
	}
	AG_PostEvent(fd, "file-chosen", "%s,%p", fd->cfile, ft);

	if (fd->flags & AG_FILEDLG_CLOSEWIN) {
/*		AG_PostEvent(pwin, "window-close", NULL); */
		AG_ObjectDetach(pwin);
	}
	AG_ObjectUnlock(fd);
}

static void
ReplaceFileConfirm(AG_Event *_Nonnull event)
{
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);
	AG_Window *qwin = AG_WINDOW_PTR(2);
	AG_Window *pwin = AG_WINDOW_PTR(3);

	ChooseFile(fd, pwin);
	AG_ObjectDetach(qwin);
}

static void
ReplaceFileDlg(AG_FileDlg *_Nonnull fd, AG_Window *_Nonnull pwin)
{
	AG_Window *win;
	AG_Button *btn;
	AG_Box *hb;

	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOTITLE);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);

	AG_LabelNew(win, 0, _("File %s exists. Overwrite?"), fd->cfile);

	hb = AG_BoxNewHoriz(win, AG_BOX_HOMOGENOUS | AG_BOX_HFILL);
	{
		AG_ButtonNewFn(hb, 0, _("Yes"),
		    ReplaceFileConfirm, "%p,%p,%p", fd, win, pwin);
		btn = AG_ButtonNewFn(hb, 0, _("Cancel"), AGWINDETACH(win));
		AG_WidgetFocus(btn);
	}
//	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

int
AG_FileDlgCheckReadAccess(AG_FileDlg *fd)
{
	AG_FileInfo info;

	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	AG_ObjectLock(fd);

	if (AG_GetFileInfo(fd->cfile, &info) == -1) {
		goto fail;
	}
	if ((info.perms & AG_FILE_READABLE) == 0) {
		AG_SetError(_("%s: Read permission denied"), fd->cfile);
		goto fail;
	}

	AG_ObjectUnlock(fd);
	return (0);
fail:
	AG_ObjectUnlock(fd);
	return (-1);
}

int
AG_FileDlgCheckWriteAccess(AG_FileDlg *fd)
{
	AG_FileInfo info;
	
	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	AG_ObjectLock(fd);

	if (AG_GetFileInfo(fd->cfile, &info) == -1) {
		goto fail;
	}
	if ((info.perms & AG_FILE_WRITEABLE) == 0) {
		AG_SetError(_("%s: Write permission denied"), fd->cfile);
		goto fail;
	}

	AG_ObjectUnlock(fd);
	return (0);
fail:
	AG_ObjectUnlock(fd);
	return (-1);
}

static void
CheckAccessAndChoose(AG_FileDlg *_Nonnull fd)
{
	AG_Window *pwin = AG_ParentWindow(fd);
	char *s;

	for (s = &fd->cfile[0]; *s != '\0'; s++) {
		if (!isspace((int) *s))
			break;
	}
	if (*s == '\0')
		return;

	if (fd->flags & AG_FILEDLG_LOAD) {
		if (AG_FileDlgCheckReadAccess(fd) == -1) {
			AG_TextMsgFromError();
			return;
		}
	} else if (fd->flags & AG_FILEDLG_SAVE) {
		AG_FileInfo info;

		/*
		 * Display a "replace file?" dialog if this file
		 * already exists.
		 */
		if (AG_GetFileInfo(fd->cfile, &info) == 0) {
			if (info.perms & AG_FILE_WRITEABLE) {
				ReplaceFileDlg(fd, pwin);
			} else {
				AG_TextMsg(AG_MSG_ERROR,
				    _("%s: File exists and is non-writeable"),
				    fd->cfile);
			}
			return;
		}
	}
	ChooseFile(fd, pwin);
}

static void
FileSelected(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);
	AG_TlistItem *ti;
	char *ext;

	AG_ObjectLock(fd);

	AG_ObjectLock(tl);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		AG_FileDlgSetFilenameS(fd, ti->text);
		AG_PostEvent(fd, "file-selected", "%s", fd->cfile);
	}
	AG_ObjectUnlock(tl);

	if (fd->comTypes && (ext = strrchr(fd->cfile, '.')) != NULL) {
		AG_ObjectLock(fd->comTypes->list);
		TAILQ_FOREACH(ti, &fd->comTypes->list->items, items) {
			AG_FileType *ft = ti->p1;
			char *ftext;
			Uint i;

			for (i = 0; i < ft->nExts; i++) {
				if ((ftext = strrchr(ft->exts[i], '.'))
				    == NULL) {
					continue;
				}
				if (Strcasecmp(ftext, ext) == 0)
					break;
			}
			if (i < ft->nExts) {
				AG_ComboSelect(fd->comTypes, ti);
				AG_PostEvent(fd->comTypes, "combo-selected",
				    "%p", ti);
				break;
			}
		}
		AG_ObjectUnlock(fd->comTypes->list);
	}

	AG_ObjectUnlock(fd);
}

static void
FileDblClicked(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);
	AG_TlistItem *itFile;

	AG_ObjectLock(fd);
	AG_ObjectLock(tl);

	if ((itFile = AG_TlistSelectedItem(tl)) != NULL) {
		AG_FileDlgSetFilenameS(fd, itFile->text);

		if (fd->okAction) {
			AG_PostEventByPtr(fd, fd->okAction, "%s,%p", fd->cfile,
			    fd->comTypes ? AG_TlistSelectedItemPtr(fd->comTypes->list) : NULL);
		} else {
			CheckAccessAndChoose(fd);
		}
	}

	AG_ObjectUnlock(tl);
	AG_ObjectUnlock(fd);
}

static void
PressedOK(AG_Event *_Nonnull event)
{
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);

	AG_ObjectLock(fd);

	if (fd->okAction) {
		AG_PostEventByPtr(fd, fd->okAction, "%s", fd->cfile);
	} else {
		CheckAccessAndChoose(fd);
	}

	AG_ObjectUnlock(fd);
}

static void
SetFilename(AG_FileDlg *_Nonnull fd, const char *_Nonnull file)
{
	if (file[0] == AG_PATHSEPCHAR) {
		Strlcpy(fd->cfile, file, sizeof(fd->cfile));
	} else {
		Strlcpy(fd->cfile, fd->cwd, sizeof(fd->cfile));
		if (!AG_PathIsFilesystemRoot(fd->cwd) &&
		    (fd->cfile[0] != '\0' &&
		     fd->cfile[strlen(fd->cfile)-1] != AG_PATHSEPCHAR)) {
			Strlcat(fd->cfile, AG_PATHSEP, sizeof(fd->cfile));
		}
		Strlcat(fd->cfile, file, sizeof(fd->cfile));
	}
}

static void
TextboxChanged(AG_Event *_Nonnull event)
{
	char path[AG_PATHNAME_MAX];
	AG_Textbox *tb = AG_TEXTBOX_SELF();
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);

	AG_ObjectLock(fd);

	AG_TextboxCopyString(tb, path, sizeof(path));
	SetFilename(fd, path);

	AG_ObjectUnlock(fd);
}

#ifdef HAVE_GLOB
static void
SelectGlobResult(AG_Event *_Nonnull event)
{
	char file[AG_PATHNAME_MAX];
	AG_Window *win = AG_WINDOW_PTR(1);
	AG_FileDlg *fd = AG_FILEDLG_PTR(2);
	const AG_TlistItem *ti = AG_TLIST_ITEM_PTR(3);
	AG_Textbox *tb = fd->tbFile;
	AG_FileInfo info;
	int endSep;

	AG_ObjectLock(fd);

	Strlcpy(file, ti->text, sizeof(file));
	endSep = (file[strlen(file)-1]==AG_PATHSEPCHAR) ? 1 : 0;
	AG_TextboxSetString(tb, file);

	if (endSep ||
	    (AG_GetFileInfo(file,&info)==0 && info.type == AG_FILE_DIRECTORY)) {
		if (AG_FileDlgSetDirectoryS(fd, file) == 0) {
			RefreshListing(fd);
		} else {
			/* AG_TextMsgFromError() */
			goto out;
		}
	} else {
		AG_FileDlgSetFilenameS(fd, file);
		CheckAccessAndChoose(fd);
	}
out:
	AG_ObjectUnlock(fd);
	AG_ObjectDetach(win);
}

static void
CloseGlobResults(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_PTR(1);

	AG_ObjectDetach(win);
}

static void
ExpandGlobResults(AG_FileDlg *_Nonnull fd, glob_t *_Nonnull gl,
    const char *_Nonnull pattern)
{
	AG_Window *winParent = WIDGET(fd)->window;
	AG_Window *win;
	Uint wView, hView;
	int x, y, w, h;
	int wMax = 0, hMax = 0;
	AG_Button *btn;
	AG_Tlist *tl;
	int i;
	
	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowAttach(winParent, win);
	AG_WindowSetCaption(win, _("Matching \"%s\""), pattern);

	tl = AG_TlistNew(win, AG_TLIST_EXPAND);
	AG_SetEvent(tl, "tlist-selected", SelectGlobResult, "%p,%p", win, fd);
	btn = AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Dismiss"),
	    CloseGlobResults, "%p", win);
	AG_WidgetFocus(btn);

	for (i = 0; i < gl->gl_pathc; i++) {
		char *p = gl->gl_pathv[i];
		AG_FileInfo fi;
		AG_TlistItem *ti;

		if (AG_GetFileInfo(p, &fi) != 0) {
			continue;
		}
		if (fi.type == AG_FILE_DIRECTORY) {
			if (p[strlen(p)-1] != AG_PATHSEPCHAR) {
				ti = AG_TlistAdd(tl, agIconDirectory.s,
				    "%s%c", p, AG_PATHSEPCHAR);
			} else {
				ti = AG_TlistAddS(tl, agIconDirectory.s, p);
			}
		} else {
			ti = AG_TlistAddS(tl, agIconDoc.s, p);
		}
		ti->p1 = &gl->gl_pathv[i];
		AG_TextSize(p, &w, NULL);
		if (w > wMax) { wMax = w; }
		hMax++;
	}

	/* Compute geometry. */
	w = wMax + 100 + tl->item_h+2 +
	    agTextFontHeight; /* scrollbar size */
	h = hMax*tl->item_h + 32;
	x = WIDGET(fd->tbFile)->rView.x2 - w;
	y = WIDGET(fd->tbFile)->rView.y1;
	if (AGDRIVER_MULTIPLE(WIDGET(fd)->drv) &&
	    winParent) {
		x += WIDGET(winParent)->x;
		y += WIDGET(winParent)->y;
	}

	/* Limit to display area. */
	AG_GetDisplaySize(WIDGET(fd)->drv, &wView, &hView);
	if (x < 0) { x = 0; }
	if (y < 0) { y = 0; }
	if (x+w > wView) { w = wView - x; }
	if (y+h > hView) { h = hView - y; }
	if (w < 5 || h < 5) {
		AG_ObjectDetach(win);
		return;
	}

	AG_WindowSetGeometry(win, x, y, w, h);
	AG_WindowShow(win);
}

static int
GlobExpansion(AG_FileDlg *_Nonnull fd, char *_Nonnull path, AG_Size pathLen)
{
	char *pathOrig;
	glob_t gl;

	if ((pathOrig = TryStrdup(path)) == NULL) {
		return (0);
	}
	if (glob(path, GLOB_TILDE, NULL, &gl) != 0) {
		goto out;
	}
	if (gl.gl_pathc == 1) {
		Strlcpy(path, gl.gl_pathv[0], pathLen);
	} else if (gl.gl_pathc > 1) {
		ExpandGlobResults(fd, &gl, pathOrig);
		free(pathOrig);
		return (1);
	}
	globfree(&gl);
out:
	free(pathOrig);
	return (0);
}
#endif /* HAVE_GLOB */

static void
TextboxReturn(AG_Event *_Nonnull event)
{
	char file[AG_PATHNAME_MAX];
	AG_Textbox *tb = AG_TEXTBOX_SELF();
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);
	AG_FileInfo info;
	int endSep;
	
	AG_ObjectLock(fd);

	AG_TextboxCopyString(tb, file, sizeof(file));
#ifdef HAVE_GLOB
	if (GlobExpansion(fd, file, sizeof(file)))
		goto out;
#endif
	if (file[0] == '\0') {
		goto out;
	}
	endSep = (file[strlen(file)-1]==AG_PATHSEPCHAR) ? 1 : 0;
	if (ProcessFilename(file, sizeof(file)) == -1) {
		goto out;
	}
	AG_TextboxSetString(tb, file);

	if (endSep ||
	    (AG_GetFileInfo(file,&info)==0 && info.type == AG_FILE_DIRECTORY)) {
		if (AG_FileDlgSetDirectoryS(fd, file) == 0) {
			RefreshListing(fd);
		} else {
			/* AG_TextMsgFromError() */
			goto out;
		}
	} else {
		AG_FileDlgSetFilenameS(fd, file);
		CheckAccessAndChoose(fd);
	}
out:
	AG_ObjectUnlock(fd);
}

static void
PressedCancel(AG_Event *_Nonnull event)
{
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);
	AG_Window *pwin;

	AG_ObjectLock(fd);

	if (fd->cancelAction) {
		AG_PostEventByPtr(fd, fd->cancelAction, NULL);
	} else if (fd->flags & AG_FILEDLG_CLOSEWIN) {
		if ((pwin = AG_ParentWindow(fd)) != NULL) {
			AG_PostEvent(pwin, "window-close", NULL);
/*			AG_ObjectDetach(pwin); */
		}
	}

	AG_ObjectUnlock(fd);
}

static void
SelectedType(AG_Event *_Nonnull event)
{
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);
	const AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	AG_FileType *ft;
	AG_FileOption *fo;
	AG_Numerical *num;
	AG_Textbox *tbox;

	AG_ObjectLock(fd);

	if (it) {
		ft = it->p1;
	} else {
		ft = TAILQ_FIRST(&fd->types);
	}
	fd->curType = ft;

	if (fd->optsCtr == NULL)
		goto no_change;

	AG_ObjectFreeChildren(fd->optsCtr);
	
	TAILQ_FOREACH(fo, &ft->opts, opts) {
		switch (fo->type) {
		case AG_FILEDLG_BOOL:
			AG_CheckboxNewInt(fd->optsCtr, AG_CHECKBOX_EXCL,
			    fo->descr,
			    &fo->data.i.val);
			break;
		case AG_FILEDLG_INT:
			num = AG_NumericalNewS(fd->optsCtr, AG_NUMERICAL_HFILL,
			    NULL, fo->descr);
			AG_BindInt(num, "value", &fo->data.i.val);
			AG_BindInt(num, "min", &fo->data.i.min);
			AG_BindInt(num, "max", &fo->data.i.max);
			break;
		case AG_FILEDLG_FLOAT:
			num = AG_NumericalNewS(fd->optsCtr, AG_NUMERICAL_HFILL,
			    fo->unit, fo->descr);
			AG_BindFloat(num, "value", &fo->data.flt.val);
			AG_BindFloat(num, "min", &fo->data.flt.min);
			AG_BindFloat(num, "max", &fo->data.flt.max);
			break;
		case AG_FILEDLG_DOUBLE:
			num = AG_NumericalNewS(fd->optsCtr, AG_NUMERICAL_HFILL,
			    fo->unit, fo->descr);
			AG_BindDouble(num, "value", &fo->data.dbl.val);
			AG_BindDouble(num, "min", &fo->data.dbl.min);
			AG_BindDouble(num, "max", &fo->data.dbl.max);
			break;
		case AG_FILEDLG_STRING:
			tbox = AG_TextboxNewS(fd->optsCtr,
			    AG_TEXTBOX_EXCL | AG_TEXTBOX_HFILL,
			    fo->descr);
#ifdef AG_UNICODE
			AG_TextboxBindUTF8(tbox, fo->data.s, sizeof(fo->data.s));
#else
			AG_TextboxBindASCII(tbox, fo->data.s, sizeof(fo->data.s));
#endif
			break;
		}
	}
	AG_SetStyle(fd->optsCtr, "font-size", "90%");
	WIDGET(fd)->flags |= AG_WIDGET_UPDATE_WINDOW;
	AG_Redraw(fd);
no_change:
	AG_ObjectUnlock(fd);
}

static void
OnShow(AG_Event *_Nonnull event)
{
	AG_FileDlg *fd = AG_FILEDLG_SELF();
	AG_TlistItem *it;
	AG_Combo *comTypes;
	int w, wMax = 0, nItems = 0;

	if (!(fd->flags & AG_FILEDLG_RESET_ONSHOW)) {
		return;
	}
	fd->flags &= ~(AG_FILEDLG_RESET_ONSHOW);

	if (fd->tbFile) {
		AG_WidgetFocus(fd->tbFile);
	}
	RefreshListing(fd);
	RefreshShortcuts(fd, 1);

	if ((comTypes = fd->comTypes) != NULL) {
		AG_PostEvent(fd->comTypes, "combo-selected", "%p", NULL);
		AG_COMBO_FOREACH(it, fd->comTypes) {
			AG_TextSize(it->text, &w, NULL);
			if (w > wMax) { wMax = w; }
			nItems++;
		}
		AG_ComboSizeHintPixels(fd->comTypes, wMax, nItems);
	}
}

static void
OnHide(AG_Event *_Nonnull event)
{
	AG_FileDlg *fd = AG_FILEDLG_SELF();

	if (fd->esFollow) {
		AG_Debug(fd, "Unfollowing %s (es=%p)\n", fd->cwd, fd->esFollow);
		AG_DelEventSink(fd->esFollow);
		fd->esFollow = NULL;
	}
}

/* Get an auto-allocated copy of the current filename. */
char *
AG_FileDlgGetFilename(AG_FileDlg *fd)
{
	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	return Strdup(fd->cfile);
}

/* Get an auto-allocated copy of the working directory. */
char *
AG_FileDlgGetDirectory(AG_FileDlg *fd)
{
	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	return Strdup(fd->cwd);
}

/* Copy the current filename into a fixed-size buffer. */
AG_Size
AG_FileDlgCopyFilename(AG_FileDlg *fd, char *dst, AG_Size dstSize)
{
	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	return AG_Strlcpy(dst, fd->cfile, dstSize);
}

/* Copy the working directory into a fixed-size buffer. */
AG_Size
AG_FileDlgCopyDirectory(AG_FileDlg *fd, char *dst, AG_Size dstSize)
{
	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	return AG_Strlcpy(dst, fd->cwd, dstSize);
}

/* Move to the specified directory (format string). */
int
AG_FileDlgSetDirectory(AG_FileDlg *fd, const char *fmt, ...)
{
	char path[AG_PATHNAME_MAX];
	va_list ap;

	va_start(ap, fmt);
	Vsnprintf(path, sizeof(path), fmt, ap);
	va_end(ap);
	
	return AG_FileDlgSetDirectoryS(fd, path);
}

/* Move to the specified directory (C string). */
int
AG_FileDlgSetDirectoryS(AG_FileDlg *fd, const char *dir)
{
	AG_FileInfo info;
	char ncwd[AG_PATHNAME_MAX], *c;
	
	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	AG_ObjectLock(fd);

	if (dir[0] == '.' && dir[1] == '\0') {
		Strlcpy(ncwd, fd->cwd, sizeof(ncwd));
	} else if (dir[0] == '.' && dir[1] == '.' && dir[2] == '\0') {
		if (!AG_PathIsFilesystemRoot(fd->cwd)) {
			Strlcpy(ncwd, fd->cwd, sizeof(ncwd));
			if ((c = strrchr(ncwd, AG_PATHSEPCHAR)) != NULL) {
				*c = '\0';
			}
			if (c == &ncwd[0]) {
				ncwd[0] = AG_PATHSEPCHAR;
				ncwd[1] = '\0';
			}
#ifdef _XBOX
			if (AG_PathIsFilesystemRoot(ncwd) &&
			    ncwd[2] != AG_PATHSEPCHAR) {
				Strlcat(ncwd, AG_PATHSEP, sizeof(ncwd));
			}
#endif
		}
	} else if (!AG_PathIsAbsolute(dir)) {
		Strlcpy(ncwd, fd->cwd, sizeof(ncwd));
		if (!(ncwd[0] == AG_PATHSEPCHAR &&
		      ncwd[1] == '\0') &&
			  ncwd[strlen(ncwd) - 1] != AG_PATHSEPCHAR) {
			Strlcat(ncwd, AG_PATHSEP, sizeof(ncwd));
		}
		Strlcat(ncwd, dir, sizeof(ncwd));
	} else {
		Strlcpy(ncwd, dir, sizeof(ncwd));
	}

	if (AG_GetFileInfo(ncwd, &info) == -1) {
		goto fail;
	}
	if (info.type != AG_FILE_DIRECTORY) {
		AG_SetError(_("%s: Not a directory"), ncwd);
		goto fail;
	}
	if ((info.perms & (AG_FILE_READABLE | AG_FILE_EXECUTABLE)) == 0) {
		AG_SetError(_("%s: Permission denied"), ncwd);
		goto fail;
	}
	if (Strlcpy(fd->cwd, ncwd, sizeof(fd->cwd)) >= sizeof(fd->cwd)) {
		AG_SetError(_("Path is too long: `%s'"), ncwd);
		goto fail;
	}
	if (fd->dirMRU) {
		AG_SetString(agConfig, fd->dirMRU, fd->cwd);
		AG_ConfigSave();
	}

	if (fd->tlDirs) { AG_TlistScrollToStart(fd->tlDirs); }
	if (fd->tlFiles) { AG_TlistScrollToStart(fd->tlFiles); }

	AG_ObjectUnlock(fd);
	return (0);
fail:
	AG_ObjectUnlock(fd);
	return (-1);
}

/* Set the current directory (fetch default from specified MRU). */
void
AG_FileDlgSetDirectoryMRU(AG_FileDlg *fd, const char *key, const char *dflt)
{
	AG_Config *cfg = agConfig;
	char *s;

	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	AG_ObjectLock(fd);
	AG_ObjectLock(cfg);

	if (AG_Defined(cfg,key) && (s = AG_GetStringDup(cfg,key))) {
		AG_FileDlgSetDirectoryS(fd, s);
		Free(s);
	} else {
		AG_SetString(cfg, key, dflt);
		if (AG_ConfigSave() == -1) {
			Verbose("Saving MRU(%s): %s\n", key, AG_GetError());
		}
		AG_FileDlgSetDirectoryS(fd, dflt);
	}
	Free(fd->dirMRU);
	fd->dirMRU = Strdup(key);
	
	AG_ObjectUnlock(cfg);
	AG_ObjectUnlock(fd);
}

/* Set the current filename (format string). */
void
AG_FileDlgSetFilename(AG_FileDlg *fd, const char *fmt, ...)
{
	char file[AG_FILENAME_MAX];
	va_list ap;
	
	va_start(ap, fmt);
	Vsnprintf(file, sizeof(file), fmt, ap);
	va_end(ap);

	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	AG_ObjectLock(fd);

	SetFilename(fd, file);
	if (fd->tbFile) {
		AG_TextboxSetString(fd->tbFile, file);
/*		AG_TextboxSetCursorPos(fd->tbFile, -1); */
	}

	AG_ObjectUnlock(fd);
}

/* Set the current filename (C string). */
void
AG_FileDlgSetFilenameS(AG_FileDlg *fd, const char *s)
{
	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	AG_ObjectLock(fd);

	SetFilename(fd, s);
	if (fd->tbFile) {
		AG_TextboxSetString(fd->tbFile, s);
/*		AG_TextboxSetCursorPos(fd->tbFile, -1); */
	}

	AG_ObjectUnlock(fd);
}

static void
MaskOptionSelected(AG_Event *_Nonnull event)
{
	AG_FileDlg *fd = AG_FILEDLG_PTR(1);

	RefreshListing(fd);
}

AG_FileDlg *
AG_FileDlgNewMRU(void *parent, const char *mruKey, Uint flags)
{
	char path[AG_PATHNAME_MAX];
	AG_FileDlg *fd;

	fd = AG_FileDlgNew(parent, flags);

	if (AG_ConfigGetPath(AG_CONFIG_PATH_DATA, 0, path, sizeof(path))
	    < sizeof(path)) {
		AG_FileDlgSetDirectoryMRU(fd, mruKey, path);
	}
	return (fd);
}

AG_FileDlg *
AG_FileDlgNewCompact(void *parent, const char *label, Uint flags)
{
	AG_FileDlg *fd;

	fd = AG_FileDlgNew(parent, (flags | AG_FILEDLG_COMPACT));
	if (label) {
		AG_TextboxSetLabelS(fd->textbox, label);
	}
	return (fd);
}

AG_FileDlg *
AG_FileDlgNewCompactMRU(void *parent, const char *mruKey, const char *label,
    Uint flags)
{
	char path[AG_PATHNAME_MAX];
	AG_FileDlg *fd;

	fd = AG_FileDlgNew(parent, (flags | AG_FILEDLG_COMPACT));
	if (label)
		AG_TextboxSetLabelS(fd->textbox, label);

	if (AG_ConfigGetPath(AG_CONFIG_PATH_DATA, 0, path, sizeof(path))
	    < sizeof(path)) {
		AG_FileDlgSetDirectoryMRU(fd, mruKey, path);
	}
	return (fd);
}

AG_FileDlg *
AG_FileDlgNew(void *parent, Uint flags)
{
	AG_FileDlg *fd;

	fd = Malloc(sizeof(AG_FileDlg));
	AG_ObjectInit(fd, &agFileDlgClass);

	fd->flags |= flags;
	if (flags & AG_FILEDLG_HFILL) { WIDGET(fd)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_FILEDLG_VFILL) { WIDGET(fd)->flags |= AG_WIDGET_VFILL; }

	if (flags & AG_FILEDLG_MULTI) {
		if (fd->tlFiles)
			fd->tlFiles->flags |= AG_TLIST_MULTI;
	}
	if (flags & AG_FILEDLG_COMPACT) {
		/*
		 * Compact Mode
		 */
		fd->textbox = AG_TextboxNewS(fd,
		    AG_TEXTBOX_EXCL | AG_TEXTBOX_HFILL,
		    _("File:"));
		AG_TextboxBindUTF8(fd->textbox, fd->cfile, sizeof(fd->cfile));
		AG_TextboxSizeHint(fd->textbox, "<XXXXXXXXXXXXXXXXXXXXXXXXXXXXX>");

		fd->btnExpand = AG_ButtonNewS(fd, AG_BUTTON_NO_FOCUS, "...");
		AG_SetStyle(fd->btnExpand, "padding", "1");

		AG_SetEvent(fd->btnExpand, "button-pushed",
		    ExpandFromCompact, "%p", fd);

		AG_ObjectAttach(parent, fd);
		return (fd);
	}

	/* Horizontal divider */
	fd->hPane = AG_PaneNewHoriz(fd, AG_PANE_EXPAND);
	AG_PaneMoveDividerPct(fd->hPane, 50);
	AG_PaneResizeAction(fd->hPane, AG_PANE_DIVIDE_EVEN);

	/* Shortcuts combo. */
	fd->comLoc = AG_ComboNewS(fd->hPane->div[0], AG_COMBO_HFILL, NULL);
	AG_ComboSizeHint(fd->comLoc, "XXXXXXXXXXXXXXXXXXXXXXXXXXXX", 5);
	AG_TlistSetCompareFn(fd->comLoc->list, AG_TlistCompareStrings);
	AG_SetEvent(fd->comLoc, "combo-selected", LocSelected, "%p", fd);

	/* Directories list. */
	fd->tlDirs = AG_TlistNew(fd->hPane->div[0], AG_TLIST_EXPAND);
	AG_SetEvent(fd->tlDirs, "tlist-dblclick", DirSelected, "%p", fd);
	AG_TlistSizeHint(fd->tlDirs, "XXXXXXXXXXXXXX", 8);

	/* Files list. */
	fd->tlFiles = AG_TlistNew(fd->hPane->div[1], AG_TLIST_EXPAND);
	AG_TlistSizeHint(fd->tlFiles, "XXXXXXXXXXXXXXXXXX", 8);
	AG_SetEvent(fd->tlFiles, "tlist-selected", FileSelected, "%p", fd);
	AG_SetEvent(fd->tlFiles, "tlist-dblclick", FileDblClicked, "%p", fd);

	/* Current directory label. */
	fd->lbCwd = AG_LabelNewPolled(fd, AG_LABEL_HFILL, ("Directory: %s"), &fd->cwd[0]);
	AG_LabelSizeHint(fd->lbCwd, 1, _("Directory: XXXXXXXXXXXXX"));
	AG_SetStyle(fd->lbCwd, "font-size", "90%");

	/* Manual file/directory entry textbox. */
	fd->tbFile = AG_TextboxNewS(fd, AG_TEXTBOX_EXCL, _("File: "));
	AG_SetEvent(fd->tbFile, "textbox-postchg", TextboxChanged, "%p", fd);
	AG_SetEvent(fd->tbFile, "textbox-return", TextboxReturn, "%p", fd);

	/* File type selector */
	if (!(flags & AG_FILEDLG_NOTYPESELECT)) {
		fd->comTypes = AG_ComboNew(fd, AG_COMBO_HFILL, _("Type: "));
		AG_SetEvent(fd->comTypes, "combo-selected", SelectedType,"%p",fd);
	}
	/* "Mask files" checkboxes. */
	if (!(flags & AG_FILEDLG_NOMASKOPTS)) {
		AG_Checkbox *cb;

		fd->cbMaskExt = cb = AG_CheckboxNewFlag(fd, AG_CHECKBOX_EXCL,
		    _("Mask files by extension"),
		    &fd->flags, AG_FILEDLG_MASK_EXT);
		AG_SetEvent(cb, "checkbox-changed", MaskOptionSelected,"%p",fd);
		AG_SetStyle(cb, "font-size", "80%");
	
		fd->cbMaskHidden = cb = AG_CheckboxNewFlag(fd, AG_CHECKBOX_EXCL,
		    _("Mask hidden files"),
		    &fd->flags, AG_FILEDLG_MASK_HIDDEN);
		AG_SetEvent(cb, "checkbox-changed", MaskOptionSelected,"%p",fd);
		AG_SetStyle(cb, "font-size", "80%");
	}
	if (!(flags & AG_FILEDLG_NOBUTTONS)) {
		fd->btnOk = AG_ButtonNewS(fd, AG_BUTTON_EXCL, _("OK"));
		AG_SetEvent(fd->btnOk, "button-pushed", PressedOK, "%p", fd);

		fd->btnCancel = AG_ButtonNewS(fd, AG_BUTTON_EXCL, _("Cancel"));
		AG_SetEvent(fd->btnCancel, "button-pushed", PressedCancel, "%p", fd);
	}
	AG_ObjectAttach(parent, fd);
	return (fd);
}

static void
Init(void *_Nonnull obj)
{
	AG_FileDlg *fd = obj;

	fd->flags = AG_FILEDLG_RESET_ONSHOW;
	(void)AG_GetCWD(fd->cwd, sizeof(fd->cwd));
	fd->cfile[0] = '\0';
	fd->fdDir = -1;

	memset(&fd->esFollow, 0, sizeof(AG_EventSink *) + /* esFollow */
	                         sizeof(AG_Pane *) +      /* hPane */
	                         sizeof(AG_Tlist *) +     /* tlDirs */
	                         sizeof(AG_Tlist *) +     /* tlFiles */
	                         sizeof(AG_Label *) +     /* lbCwd */
	                         sizeof(AG_Textbox *) +   /* tbFile */
	                         sizeof(AG_Combo *) +     /* comTypes */
	                         sizeof(AG_Checkbox *) +  /* cbMaskExt */
	                         sizeof(AG_Checkbox *) +  /* cbMaskHidden */
	                         sizeof(AG_Button *) +    /* btnOk */
	                         sizeof(AG_Button *) +    /* btnCancel */
	                         sizeof(AG_Event *) +     /* okAction */
	                         sizeof(AG_Event *) +     /* cancelAction */
	                         sizeof(char *) +         /* dirMRU */
	                         sizeof(void *) +         /* optsCts */
	                         sizeof(AG_FileType *) +  /* curType */
	                         sizeof(AG_Combo *) +     /* comLoc */
	                         sizeof(AG_Textbox *) +   /* textbox */
	                         sizeof(AG_Button *) +    /* btnExpand */
	                         sizeof(AG_Window *) +    /* winExpand */
	                         sizeof(AG_FileDlg *));   /* fdExpand */

	TAILQ_INIT(&fd->types);

	AG_AddEvent(fd, "widget-shown", OnShow, NULL);
	AG_AddEvent(fd, "widget-hidden", OnHide, NULL);
}

/*
 * Register an event handler for the "OK" / "Cancel" buttons.
 * This overrides type-specific handlers.
 */
void
AG_FileDlgOkAction(AG_FileDlg *fd, AG_EventFn fn, const char *fmt, ...)
{
	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	AG_ObjectLock(fd);

	if (fd->okAction) {
		AG_UnsetEvent(fd, fd->okAction->name);
	}
	fd->okAction = AG_SetEvent(fd, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(fd->okAction, fmt, ap);
		va_end(ap);
	}
#if 0
	if (fd->flags & AG_FILEDLG_ASYNC)
		fd->okAction->flags |= AG_EVENT_ASYNC;
#endif
	AG_ObjectUnlock(fd);
}
void
AG_FileDlgCancelAction(AG_FileDlg *fd, AG_EventFn fn, const char *fmt, ...)
{
	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");
	AG_ObjectLock(fd);

	if (fd->cancelAction) {
		AG_UnsetEvent(fd, fd->cancelAction->name);
	}
	fd->cancelAction = AG_SetEvent(fd, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(fd->cancelAction, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(fd);
}

static void
Destroy(void *_Nonnull obj)
{
	AG_FileDlg *fd = obj;
	AG_FileType *ft, *ft2;
	AG_FileOption *fo, *fo2;
	Uint i;

	for (ft = TAILQ_FIRST(&fd->types);
	     ft != TAILQ_END(&fd->types);
	     ft = ft2) {
		ft2 = TAILQ_NEXT(ft, types);
	
		for (fo = TAILQ_FIRST(&ft->opts);
		     fo != TAILQ_END(&ft->opts);
		     fo = fo2) {
			fo2 = TAILQ_NEXT(fo, opts);
			free(fo->descr);
			free(fo->key);
			free(fo);
		}
		free(ft->descr);
		for (i = 0; i < ft->nExts; i++) {
			free(ft->exts[i]);
		}
		free(ft->exts);
		free(ft->allExts);
		free(ft);
	}
	Free(fd->dirMRU);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, obj, ag_widget)
		AG_WidgetDraw(chld);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_FileDlg *fd = obj;
	AG_SizeReq rc;
	const int spacingVert = WIDGET(fd)->spacingVert;

	if (fd->flags & AG_FILEDLG_COMPACT) {               /* Compact mode */
		AG_WidgetSizeReq(fd->textbox, &rc);
		r->w = rc.w;
		r->h = rc.h;
		AG_WidgetSizeReq(fd->btnExpand, &rc);
		r->w += rc.w;
		if (rc.h > r->h) { r->h = rc.h; }
		return;
	}

	AG_WidgetSizeReq(fd->hPane, &rc);
	r->w = rc.w;
	r->h = rc.h + spacingVert;
	AG_WidgetSizeReq(fd->lbCwd, &rc);
	r->h += rc.h + spacingVert;
	AG_WidgetSizeReq(fd->tbFile, &rc);
	r->h += rc.h + spacingVert;
	if (fd->comTypes) {
		AG_WidgetSizeReq(fd->comTypes, &rc);
		r->h += rc.h + spacingVert;
	}
	if (!(fd->flags & AG_FILEDLG_NOMASKOPTS)) {
		AG_WidgetSizeReq(fd->cbMaskExt, &rc);
		r->h += rc.h + spacingVert;
		AG_WidgetSizeReq(fd->cbMaskHidden, &rc);
		r->h += rc.h + spacingVert;
	}
	if (!(fd->flags & AG_FILEDLG_NOBUTTONS)) {
		AG_SizeReq rOk, rCancel;

		AG_WidgetSizeReq(fd->btnOk, &rOk);
		AG_WidgetSizeReq(fd->btnCancel, &rCancel);

		r->h += MAX(rOk.h, rCancel.h) + 1;
	}
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull aFD)
{
	AG_FileDlg *fd = obj;
	AG_SizeReq r;
	AG_SizeAlloc a;
	const int paddingLeft  = WIDGET(fd)->paddingLeft;
	const int paddingRight = WIDGET(fd)->paddingRight;
	const int paddingTop   = WIDGET(fd)->paddingTop;
	const int spacingVert  = WIDGET(fd)->spacingVert;
	int hBtn;
	
	if (fd->flags & AG_FILEDLG_COMPACT) {               /* Compact mode */
		AG_WidgetSizeReq(fd->textbox, &r);
		hBtn = r.h;
		AG_WidgetSizeReq(fd->btnExpand, &r);
		hBtn = MAX(hBtn, r.h);
		a.x = 0;
		a.y = 0;
		a.w = aFD->w - r.w;
		a.h = MIN(hBtn, aFD->h);
		AG_WidgetSizeAlloc(fd->textbox, &a);
		a.x += a.w;                                /* aFD->w - r.w */
		a.w = r.w;
		AG_WidgetSizeAlloc(fd->btnExpand, &a);
		return (0);
	}

	if ((fd->flags & AG_FILEDLG_NOBUTTONS) == 0) {     /* OK and Cancel */
		AG_WidgetSizeReq(fd->btnOk, &r);
		hBtn = r.h;
		AG_WidgetSizeReq(fd->btnCancel, &r);
		hBtn = MAX(hBtn, r.h);
	} else {
		hBtn = 0;
	}

	a.x = paddingLeft;
	a.y = paddingTop;
	a.w = aFD->w - paddingLeft - paddingRight;
	a.h = aFD->h - hBtn - paddingTop - WIDGET(fd)->paddingBottom;

	AG_WidgetSizeReq(fd->lbCwd, &r);
	a.h -= r.h;
	AG_WidgetSizeReq(fd->tbFile, &r);
	a.h -= r.h;

	if (fd->comTypes) {
		AG_WidgetSizeReq(fd->comTypes, &r);
		a.h -= r.h + spacingVert;
	}
	if (!(fd->flags & AG_FILEDLG_NOMASKOPTS)) {
		AG_WidgetSizeReq(fd->cbMaskExt, &r);
		a.h -= r.h + spacingVert;
		AG_WidgetSizeReq(fd->cbMaskHidden, &r);
		a.h -= r.h + spacingVert;
	}
	a.h -= spacingVert;
	if (fd->comTypes)
		a.h -= spacingVert;
	if ((fd->flags & AG_FILEDLG_NOMASKOPTS) == 0)
		a.h -= spacingVert;

	AG_WidgetSizeAlloc(fd->hPane, &a);
	a.y += a.h + spacingVert;
	AG_WidgetSizeReq(fd->lbCwd, &r);
	a.h  = r.h;
	AG_WidgetSizeAlloc(fd->lbCwd, &a);
	a.y += a.h + spacingVert;
	AG_WidgetSizeReq(fd->tbFile, &r);
	a.h  = r.h;
	AG_WidgetSizeAlloc(fd->tbFile, &a);
	a.y += a.h + spacingVert;

	if (fd->comTypes) {                           /* File type selector */
		AG_WidgetSizeReq(fd->comTypes, &r);
		a.h  = r.h;
		AG_WidgetSizeAlloc(fd->comTypes, &a);
		a.y += a.h + spacingVert;
	}
	if ((fd->flags & AG_FILEDLG_NOMASKOPTS) == 0) {   /* Masking options */
		AG_WidgetSizeReq(fd->cbMaskExt, &r);
		a.h  = r.h;
		AG_WidgetSizeAlloc(fd->cbMaskExt, &a); 
		a.y += a.h + spacingVert;
		AG_WidgetSizeReq(fd->cbMaskHidden, &r);
		a.h  = r.h;
		AG_WidgetSizeAlloc(fd->cbMaskHidden, &a);
		a.y += a.h + spacingVert;
	}

	if ((fd->flags & AG_FILEDLG_NOBUTTONS) == 0) {      /* OK and Cancel */
		const int spacingHoriz = WIDGET(fd)->spacingHoriz;

		a.w = ((aFD->w - paddingLeft - paddingRight - spacingHoriz) >> 1);
		a.h = hBtn;
		AG_WidgetSizeAlloc(fd->btnOk, &a);
		a.x = paddingLeft + a.w + spacingHoriz;
		AG_WidgetSizeAlloc(fd->btnCancel, &a);
	}
	return (0);
}

/*
 * Register a common action for all image file formats supported by
 * AG_SurfaceFromFile().
 */
void
AG_FileDlgAddImageTypes(AG_FileDlg *fd, AG_EventFn fn, const char *fmt, ...)
{
	AG_FileType *ftBMP;

	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");

	ftBMP = AG_FileDlgAddType(fd, _("Windows Bitmap"), "*.bmp", NULL, NULL);
	if (fn) {
		ftBMP->action = AG_SetEvent(fd, NULL, fn, NULL);
		if (fmt) {
			va_list ap;

			va_start(ap, fmt);
			AG_EventGetArgs(ftBMP->action, fmt, ap);
			va_end(ap);
		}
	}
#ifdef HAVE_JPEG
	{
		AG_FileType *ft;

		ft = AG_FileDlgAddType(fd, _("JPEG image"), "*.jpg,*.jpeg", NULL, NULL);
		ft->action = AG_EventDup(ftBMP->action);
	}
#endif
#ifdef HAVE_PNG
	{
		AG_FileType *ft;

		ft = AG_FileDlgAddType(fd, _("PNG image"), "*.png", NULL, NULL);
		ft->action = AG_EventDup(ftBMP->action);
	}
#endif
}

/*
 * Inherit the set of FileTypes (and FileOptions) from another FileDlg.
 */
void
AG_FileDlgCopyTypes(AG_FileDlg *fdDst, const AG_FileDlg *fdSrc)
{
	AG_FileType *st, *dt;		/* Source type, destination type */
	const AG_FileOption *so;	/* Source option */

	AG_OBJECT_ISA(fdSrc, "AG_Widget:AG_FileDlg:*");
	AG_OBJECT_ISA(fdDst, "AG_Widget:AG_FileDlg:*");
		
	TAILQ_FOREACH(st, &fdSrc->types, types) {
		dt = AG_FileDlgAddType(fdDst, st->descr, st->allExts, NULL,NULL);
		if (st->action != NULL) {
			dt->action = Malloc(sizeof(AG_Event));
			AG_EventCopy(dt->action, st->action);
		}
	
		TAILQ_FOREACH(so, &st->opts, opts) {
			switch (so->type) {
			case AG_FILEDLG_BOOL:
				AG_FileOptionNewBool(dt, so->descr, so->key,
				    so->data.i.val);
				break;
			case AG_FILEDLG_INT:
				AG_FileOptionNewInt(dt, so->descr, so->key,
				    so->data.i.val,
				    so->data.i.min,
				    so->data.i.max);
				break;
			case AG_FILEDLG_FLOAT:
				AG_FileOptionNewFlt(dt, so->descr, so->key,
				    so->data.flt.val,
				    so->data.flt.min,
				    so->data.flt.max,
				    so->unit);
				break;
			case AG_FILEDLG_DOUBLE:
				AG_FileOptionNewDbl(dt, so->descr, so->key,
				    so->data.dbl.val,
				    so->data.dbl.min,
				    so->data.dbl.max,
				    so->unit);
				break;
			case AG_FILEDLG_STRING:
				AG_FileOptionNewString(dt, so->descr, so->key,
				    so->data.s);
				break;
			}
		}
	}
}

/* Register a new file type and callback function. */
AG_FileType *
AG_FileDlgAddType(AG_FileDlg *fd, const char *descr, const char *exts,
    AG_EventFn fn, const char *fmt, ...)
{
	char extsLbl[64];
	AG_FileType *ft;
	char *dexts, *ds, *ext;
	AG_TlistItem *it;

	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");

	ft = Malloc(sizeof(AG_FileType));
	ft->fd = fd;
	ft->descr = Strdup(descr);
	ft->exts = Malloc(sizeof(char *));
	ft->nExts = 0;
	TAILQ_INIT(&ft->opts);
	
	ft->allExts = Strdup(exts);
	ds = dexts = Strdup(exts);
	extsLbl[0] = '\0';
	while ((ext = AG_Strsep(&ds, ",;")) != NULL) {
		if (ext[0] == '*' && ext[1] == '.') {
			ext++;
		}
		ft->exts = Realloc(ft->exts, (ft->nExts+1)*sizeof(char *));
		ft->exts[ft->nExts++] = Strdup(ext);
		if (strcmp("<-x>", ext) == 0) {
			Strlcat(extsLbl, "* ", sizeof(extsLbl));
		} else if (ext[0] == '<' && ext[1] == '=') {
			ext[strlen(ext)-1] = '\0';
			Strlcat(extsLbl, &ext[2], sizeof(extsLbl));
			Strlcat(extsLbl, " ", sizeof(extsLbl));
		} else {
			Strlcat(extsLbl, "*", sizeof(extsLbl));
			Strlcat(extsLbl, ext, sizeof(extsLbl));
			Strlcat(extsLbl, " ", sizeof(extsLbl));
		}
	}
	free(dexts);
	
	AG_ObjectLock(fd);

	if (fn) {
		ft->action = AG_SetEvent(fd, NULL, fn, NULL);
		if (fmt) {
			va_list ap;

			va_start(ap, fmt);
			AG_EventGetArgs(ft->action, fmt, ap);
			va_end(ap);
		}
#if 0
		if (fd->flags & AG_FILEDLG_ASYNC)
			ft->action->flags |= AG_EVENT_ASYNC;
#endif
	} else {
		ft->action = NULL;
	}
	if (fd->comTypes) {
		if (extsLbl[0] != '\0') {
			it = AG_TlistAdd(fd->comTypes->list, NULL, "%s ( %s)",
			    descr, extsLbl);
		} else {
			it = AG_TlistAddS(fd->comTypes->list, NULL, descr);
		}
		it->p1 = ft;
		if (TAILQ_EMPTY(&fd->types))
			AG_ComboSelectPointer(fd->comTypes, ft);
	}
	TAILQ_INSERT_TAIL(&fd->types, ft, types);
	
	AG_ObjectUnlock(fd);
	return (ft);
}

AG_FileOption *
AG_FileOptionNewBool(AG_FileType *ft, const char *descr, const char *key,
    int dflt)
{
	AG_FileDlg *fd = ft->fd;
	AG_FileOption *fto;

	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");

	fto = Malloc(sizeof(AG_FileOption));
	fto->descr = Strdup(descr);
	fto->key = Strdup(key);
	fto->unit = NULL;
	fto->type = AG_FILEDLG_BOOL;
	fto->data.i.val = dflt;
	
	AG_ObjectLock(fd);
	TAILQ_INSERT_TAIL(&ft->opts, fto, opts);
	AG_ObjectUnlock(fd);

	return (fto);
}

AG_FileOption *
AG_FileOptionNewInt(AG_FileType *ft, const char *descr, const char *key,
    int dflt, int min, int max)
{
	AG_FileDlg *fd = ft->fd;
	AG_FileOption *fto;

	AG_OBJECT_ISA(fd, "AG_Widget:AG_FileDlg:*");

	fto = Malloc(sizeof(AG_FileOption));
	fto->descr = Strdup(descr);
	fto->key = Strdup(key);
	fto->unit = NULL;
	fto->type = AG_FILEDLG_INT;
	fto->data.i.val = dflt;
	fto->data.i.min = min;
	fto->data.i.max = max;
	
	AG_ObjectLock(fd);
	TAILQ_INSERT_TAIL(&ft->opts, fto, opts);
	AG_ObjectUnlock(fd);

	return (fto);
}

AG_FileOption *
AG_FileOptionNewFlt(AG_FileType *ft, const char *descr, const char *key,
    float dflt, float min, float max, const char *unit)
{
	AG_FileDlg *fd = ft->fd;
	AG_FileOption *fto;

	fto = Malloc(sizeof(AG_FileOption));
	fto->descr = Strdup(descr);
	fto->key = Strdup(key);
	fto->unit = (unit != NULL) ? Strdup(unit) : NULL;
	fto->type = AG_FILEDLG_FLOAT;
	fto->data.flt.val = dflt;
	fto->data.flt.min = min;
	fto->data.flt.max = max;
	
	AG_ObjectLock(fd);
	TAILQ_INSERT_TAIL(&ft->opts, fto, opts);
	AG_ObjectUnlock(fd);

	return (fto);
}

AG_FileOption *
AG_FileOptionNewDbl(AG_FileType *ft, const char *descr, const char *key,
    double dflt, double min, double max, const char *unit)
{
	AG_FileDlg *fd = ft->fd;
	AG_FileOption *fto;

	fto = Malloc(sizeof(AG_FileOption));
	fto->descr = Strdup(descr);
	fto->key = Strdup(key);
	fto->unit = (unit != NULL) ? Strdup(unit) : NULL;
	fto->type = AG_FILEDLG_DOUBLE;
	fto->data.dbl.val = dflt;
	fto->data.dbl.min = min;
	fto->data.dbl.max = max;
	
	AG_ObjectLock(fd);
	TAILQ_INSERT_TAIL(&ft->opts, fto, opts);
	AG_ObjectUnlock(fd);

	return (fto);
}

AG_FileOption *
AG_FileOptionNewString(AG_FileType *ft, const char *descr, const char *key,
    const char *dflt)
{
	AG_FileDlg *fd = ft->fd;
	AG_FileOption *fto;

	fto = Malloc(sizeof(AG_FileOption));
	fto->descr = Strdup(descr);
	fto->key = Strdup(key);
	fto->unit = NULL;
	fto->type = AG_FILEDLG_STRING;
	Strlcpy(fto->data.s, dflt, sizeof(fto->data.s));
	
	AG_ObjectLock(fd);
	TAILQ_INSERT_TAIL(&ft->opts, fto, opts);
	AG_ObjectUnlock(fd);

	return (fto);
}

/* The FileDlg must be locked. */
AG_FileOption *
AG_FileOptionGet(AG_FileType *ft, const char *key)
{
	AG_FileOption *fo;

	TAILQ_FOREACH(fo, &ft->opts, opts) {
		if (strcmp(fo->key, key) == 0)
			break;
	}
	return (fo);
}

int
AG_FileOptionInt(AG_FileType *ft, const char *key)
{
	AG_FileDlg *fd = ft->fd;
	AG_FileOption *fo;
	int rv;

	AG_ObjectLock(fd);

	fo = AG_FileOptionGet(ft, key);
	rv = (fo != NULL) ? fo->data.i.val : -1;

	AG_ObjectUnlock(fd);
	return (rv);
}

int
AG_FileOptionBool(AG_FileType *ft, const char *key)
{
	return AG_FileOptionInt(ft, key);
}

float
AG_FileOptionFlt(AG_FileType *ft, const char *key)
{
	AG_FileDlg *fd = ft->fd;
	AG_FileOption *fo;
	float rv;

	AG_ObjectLock(fd);

	fo = AG_FileOptionGet(ft, key);
	rv = (fo != NULL) ? fo->data.flt.val : 0.0;

	AG_ObjectUnlock(fd);
	return (rv);
}

double
AG_FileOptionDbl(AG_FileType *ft, const char *key)
{
	AG_FileDlg *fd = ft->fd;
	AG_FileOption *fo;
	double rv;

	AG_ObjectLock(fd);

	fo = AG_FileOptionGet(ft, key);
	rv = (fo != NULL) ? fo->data.dbl.val : 0.0;

	AG_ObjectUnlock(fd);
	return (rv);
}

char *
AG_FileOptionString(AG_FileType *ft, const char *key)
{
	AG_FileDlg *fd = ft->fd;
	AG_FileOption *fo;
	char *rv;

	AG_ObjectLock(fd);

	fo = AG_FileOptionGet(ft, key);
	rv = (fo != NULL) ? fo->data.s : "";

	AG_ObjectUnlock(fd);
	return (rv);
}

AG_WidgetClass agFileDlgClass = {
	{
		"Agar(Widget:FileDlg)",
		sizeof(AG_FileDlg),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */
