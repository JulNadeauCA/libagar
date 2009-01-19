/*
 * Copyright (c) 2001-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Properties dialog for AG_Object.
 */

#include <core/core.h>
#include <core/md5.h>
#include <core/sha1.h>
#include <core/rmd160.h>
#include <core/config.h>

#include <gui/window.h>
#include <gui/box.h>
#include <gui/label.h>
#include <gui/tlist.h>
#include <gui/textbox.h>
#include <gui/notebook.h>
#include <gui/separator.h>
#include <gui/checkbox.h>

#ifdef AG_NETWORK
#include <core/rcs.h>
#endif

#include "dev.h"

#include <config/ag_lockdebug.h>

const AG_FlagDescr devObjectFlags[] = {
	{ AG_OBJECT_DEBUG,		N_("Debugging"),		  1 },
	{ AG_OBJECT_READONLY,		N_("Read-only"),		  1 },
	{ AG_OBJECT_INDESTRUCTIBLE,	N_("Indestructible"),		  1 },
	{ AG_OBJECT_NON_PERSISTENT,	N_("Non-persistent"),		  1 },
	{ AG_OBJECT_RELOAD_PROPS,	N_("Don't clear props on load"),  1 },
	{ AG_OBJECT_PRESERVE_DEPS,	N_("Preserve null dependencies"), 1 },
	{ AG_OBJECT_REMAIN_DATA,	N_("Keep data resident"),	  1 },
	{ AG_OBJECT_RESIDENT,		N_("Data part is resident"),	  0 },
	{ AG_OBJECT_STATIC,		N_("Statically allocated"),	  0 },
	{ AG_OBJECT_REOPEN_ONLOAD,	N_("Recreate UI on load"),	  0 },
	{ AG_OBJECT_NAME_ONATTACH,	N_("Generate name upon attach"),  0 },
	{ 0,				"",				  0 }
};

static void
PollDeps(AG_Event *event)
{
	char path[AG_OBJECT_PATH_MAX];
	AG_Tlist *tl = AG_SELF();
	AG_Object *ob = AG_PTR(1);
	AG_ObjectDep *dep;

	AG_TlistClear(tl);
	AG_LockVFS(ob);
	TAILQ_FOREACH(dep, &ob->deps, deps) {
		char label[AG_TLIST_LABEL_MAX];
	
		if (dep->obj != NULL) {
			AG_ObjectCopyName(dep->obj, path, sizeof(path));
		} else {
			Strlcpy(path, "(NULL)", sizeof(path));
		}
		if (dep->count == AG_OBJECT_DEP_MAX) {
			Snprintf(label, sizeof(label), "%s (wired)", path);
		} else {
			Snprintf(label, sizeof(label), "%s (%u)", path,
			    (Uint)dep->count);
		}
		AG_TlistAddPtr(tl, AG_ObjectIcon(dep->obj), label, dep);
	}
	AG_UnlockVFS(ob);
	AG_TlistRestore(tl);
}

static void
PollProps(AG_Event *event)
{
	char val[AG_TLIST_LABEL_MAX];
	AG_Tlist *tl = AG_SELF();
	AG_Object *ob = AG_PTR(1);
	Uint i;
	
	AG_TlistClear(tl);
	for (i = 0; i < ob->nVars; i++) {
		AG_Variable *V = &ob->vars[i];
		AG_PropPrint(val, sizeof(val), ob, V->name);
		AG_TlistAdd(tl, NULL, "%s = %s", V->name, val);
	}
	AG_TlistRestore(tl);
}

#if defined(AG_THREADS) && defined(AG_LOCKDEBUG)
static void
PollLocks(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Object *ob = AG_PTR(1);
	int i;
	
	AG_TlistClear(tl);
	for (i = 0; i < ob->nlockinfo; i++) {
		AG_TlistAdd(tl, NULL, "%s", ob->lockinfo[i]);
	}
	AG_TlistRestore(tl);
}
#endif /* AG_THREADS and AG_LOCKDEBUG */

static void
PollEvents(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Object *ob = AG_PTR(1);
	AG_Event *ev;
	
	AG_TlistClear(tl);
	TAILQ_FOREACH(ev, &ob->events, events) {
		char args[AG_TLIST_LABEL_MAX], arg[16];
		int i;

		args[0] = '(';
		args[1] = '\0';
		for (i = 1; i < ev->argc; i++) {
			AG_Variable *V = &ev->argv[i];

			if (V->name != NULL && V->name[0] != '\0') {
				Strlcat(args, V->name, sizeof(args));
				Strlcat(args, "=", sizeof(args));
			}
			switch (V->type) {
			case AG_VARIABLE_POINTER:
				Snprintf(arg, sizeof(arg), "%p", ev->argv[i].data.p);
				break;
			case AG_VARIABLE_STRING:
				Snprintf(arg, sizeof(arg), "\"%s\"", ev->argv[i].data.s);
				break;
			case AG_VARIABLE_INT:
				Snprintf(arg, sizeof(arg), "%d", ev->argv[i].data.i);
				break;
			case AG_VARIABLE_UINT:
				Snprintf(arg, sizeof(arg), "%u", (Uint)ev->argv[i].data.i);
				break;
			case AG_VARIABLE_FLOAT:
				Snprintf(arg, sizeof(arg), "<%.04f>", ev->argv[i].data.flt);
				break;
			case AG_VARIABLE_DOUBLE:
				Snprintf(arg, sizeof(arg), "<%.04f>", ev->argv[i].data.dbl);
				break;
			default:
				Snprintf(arg, sizeof(arg), "???");
				break;
			}
			Strlcat(args, arg, sizeof(args));
			if (i < ev->argc-1) {
				Strlcat(args, ", ", sizeof(args));
			}
		}
		Strlcat(args, ")", sizeof(args));

		AG_TlistAdd(tl, NULL, "%s%s%s %s", ev,
		    (ev->flags & AG_EVENT_ASYNC) ? " <async>" : "",
		    (ev->flags & AG_EVENT_PROPAGATE) ? " <propagate>" : "",
		    args);

	}
	AG_TlistRestore(tl);
}

static void
RenameObject(AG_Event *event)
{
	AG_Textbox *tb = AG_SELF();
	AG_Object *ob = AG_PTR(1);

	if (AG_ObjectPageIn(ob) == 0) {
		AG_ObjectUnlinkDatafiles(ob);
		Strlcpy(ob->name, tb->ed->string, sizeof(ob->name));
		AG_ObjectPageOut(ob);
	}
	AG_PostEvent(NULL, ob, "renamed", NULL);
}

static void
RefreshSums(AG_Event *event)
{
	char checksum[128];
	AG_Object *ob = AG_PTR(1);
	AG_Textbox *tbMD5 = AG_PTR(2);
	AG_Textbox *tbSHA1 = AG_PTR(3);
	AG_Textbox *tbRMD160 = AG_PTR(4);

	if (AG_ObjectCopyChecksum(ob, AG_OBJECT_MD5, checksum) > 0) {
		AG_TextboxPrintf(tbMD5,  "%s", checksum);
	} else {
		AG_TextboxPrintf(tbMD5,  "(%s)", AG_GetError());
	}
	if (AG_ObjectCopyChecksum(ob, AG_OBJECT_SHA1, checksum) > 0) {
		AG_TextboxPrintf(tbSHA1,  "%s", checksum);
	} else {
		AG_TextboxPrintf(tbSHA1,  "(%s)", AG_GetError());
	}
	if (AG_ObjectCopyChecksum(ob, AG_OBJECT_RMD160, checksum) > 0) {
		AG_TextboxPrintf(tbRMD160,  "%s", checksum);
	} else {
		AG_TextboxPrintf(tbRMD160,  "(%s)", AG_GetError());
	}
}

#ifdef AG_NETWORK

static void
RefreshRepoStatus(AG_Event *event)
{
	char objdir[AG_OBJECT_PATH_MAX];
	char digest[AG_OBJECT_DIGEST_MAX];
	AG_Object *ob = AG_PTR(1);
	AG_Label *lblStatus = AG_PTR(2);
	AG_Tlist *tl = AG_PTR(3);
	extern const char *agRcsStatusStrings[];
	enum ag_rcs_status status;
	AG_RCSLog log;
	size_t len;
	Uint working_rev, repo_rev;
	int i;

	if (AG_ObjectCopyName(ob, objdir, sizeof(objdir)) == -1 ||
	    AG_ObjectCopyDigest(ob, &len, digest) == -1) {
		return;
	}
	if (AG_RcsConnect() == -1) {
		return;
	}
	status = AG_RcsStatus(ob, objdir, digest, NULL, NULL, &repo_rev,
	    &working_rev);
	AG_LabelPrintf(lblStatus,
	    _("RCS status: %s\n"
	      "Working revision: #%u\n"
	      "Repository revision: #%u\n"),
	    agRcsStatusStrings[status],
	    (status!=AG_RCS_UNKNOWN && status!=AG_RCS_ERROR) ? working_rev : 0,
	    (status!=AG_RCS_UNKNOWN && status!=AG_RCS_ERROR) ? repo_rev: 0);

	if (AG_RcsGetLog(objdir, &log) == -1)
		goto out;
	
	AG_TlistClear(tl);
	for (i = 0; i < log.nEnts; i++) {
		AG_RCSLogEntry *lent = &log.ents[i];

		AG_TlistAdd(tl, NULL, "[#%s.%s] %s", lent->rev, lent->author,
		    lent->msg);
	}
	AG_TlistRestore(tl);
	AG_RcsFreeLog(&log);
out:
	AG_RcsDisconnect();
}

#endif /* AG_NETWORK */

void *
DEV_ObjectEdit(void *p)
{
	AG_Object *ob = p;
	AG_Window *win;
	AG_Textbox *tbox;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_Tlist *tl;
	AG_Box *box;
	AG_Button *btn;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Object %s"), ob->name);
	AG_WindowSetPosition(win, AG_WINDOW_UPPER_RIGHT, 1);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	ntab = AG_NotebookAddTab(nb, _("Infos"), AG_BOX_VERT);
	{
		AG_Textbox *tbMD5, *tbSHA1, *tbRMD160;

		tbox = AG_TextboxNew(ntab, 0, _("Name: "));
		AG_TextboxPrintf(tbox, ob->name);
		AG_WidgetFocus(tbox);
		AG_SetEvent(tbox, "textbox-return", RenameObject, "%p", ob);
		
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);
	
		AG_LabelNew(ntab, 0, _("Class: %s"), ob->cls->hier);
		AG_CheckboxSetFromFlags(ntab, 0, &ob->flags, devObjectFlags);
#if 0
		AG_LabelNewPolledMT(ntab, AG_LABEL_HFILL, &agLinkageLock,
		    _("Parent: %[obj]"), &ob->parent);
#endif
		AG_LabelNew(ntab, 0, _("Save prefix: %s"),
		    ob->save_pfx != NULL ? ob->save_pfx : AG_PATHSEP);

		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);

		tbMD5 = AG_TextboxNew(ntab, AG_TEXTBOX_READONLY, "MD5: ");
		AG_WidgetDisable(tbMD5);
		tbSHA1 = AG_TextboxNew(ntab, AG_TEXTBOX_READONLY, "SHA1: ");
		AG_WidgetDisable(tbSHA1);
		tbRMD160 = AG_TextboxNew(ntab, AG_TEXTBOX_READONLY, "RMD160: ");
		AG_WidgetDisable(tbRMD160);
		AG_TextboxSizeHint(tbMD5, "888888888888888888888888888888888");

		box = AG_BoxNew(ntab, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|
					            AG_BOX_HFILL);
		{
			btn = AG_ButtonNewFn(box, 0, _("Refresh checksums"),
			    RefreshSums, "%p,%p,%p,%p", ob,
			    tbMD5, tbSHA1, tbRMD160);
			AG_PostEvent(NULL, btn, "button-pushed", NULL);
		}
	}

#ifdef AG_NETWORK
	ntab = AG_NotebookAddTab(nb, _("RCS"), AG_BOX_VERT);
	{
		AG_Label *lblStatus;
		AG_Tlist *tl;

		lblStatus = AG_LabelNewString(ntab, AG_LABEL_HFILL, "...");
		AG_LabelSizeHint(lblStatus, 3, _("Repository revision: #0000"));

		AG_LabelNewString(ntab, 0, _("Revision history:"));
		tl = AG_TlistNew(ntab, AG_TLIST_EXPAND);

		btn = AG_ButtonNewFn(ntab, AG_BUTTON_HFILL, _("Refresh status"),
		    RefreshRepoStatus, "%p,%p,%p", ob, lblStatus, tl);

		if (agRcsMode)
			AG_PostEvent(NULL, btn, "button-pushed", NULL);
	}
#endif /* AG_NETWORK */

	ntab = AG_NotebookAddTab(nb, _("Deps"), AG_BOX_VERT);
	{
		tl = AG_TlistNew(ntab, AG_TLIST_POLL|AG_TLIST_EXPAND);
		AG_TlistSizeHint(tl, "XXXXXXXXXXXX", 6);
		AG_SetEvent(tl, "tlist-poll", PollDeps, "%p", ob);
	}
	
	ntab = AG_NotebookAddTab(nb, _("Events"), AG_BOX_VERT);
	{
		tl = AG_TlistNew(ntab, AG_TLIST_POLL|AG_TLIST_EXPAND);
		AG_SetEvent(tl, "tlist-poll", PollEvents, "%p", ob);
	}
	
	ntab = AG_NotebookAddTab(nb, _("Properties"), AG_BOX_VERT);
	{
		tl = AG_TlistNew(ntab, AG_TLIST_POLL|AG_TLIST_EXPAND);
		AG_SetEvent(tl, "tlist-poll", PollProps, "%p", ob);
	}
	
#if defined(AG_THREADS) && defined(AG_LOCKDEBUG)
	ntab = AG_NotebookAddTab(nb, _("Locks"), AG_BOX_VERT);
	{
		tl = AG_TlistNew(ntab, AG_TLIST_POLL|AG_TLIST_EXPAND);
		AG_SetEvent(tl, "tlist-poll", PollLocks, "%p", ob);
	}
#endif
	return (win);
}
