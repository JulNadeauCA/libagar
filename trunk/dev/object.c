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

#include <compat/md5.h>
#include <compat/sha1.h>
#include <compat/rmd160.h>
#include <compat/dir.h>
#include <compat/file.h>

#include <core/config.h>
#include <core/view.h>
#include <core/typesw.h>

#include <gui/window.h>
#include <gui/box.h>
#include <gui/label.h>
#include <gui/tlist.h>
#include <gui/textbox.h>
#include <gui/notebook.h>
#include <gui/separator.h>
#include <gui/checkbox.h>

#ifdef NETWORK
#include <core/rcs.h>
#endif

#include "dev.h"

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
	AG_LockLinkage();
	TAILQ_FOREACH(dep, &ob->deps, deps) {
		char label[AG_TLIST_LABEL_MAX];
	
		if (dep->obj != NULL) {
			AG_ObjectCopyName(dep->obj, path, sizeof(path));
		} else {
			strlcpy(path, "(NULL)", sizeof(path));
		}
		if (dep->count == AG_OBJECT_DEP_MAX) {
			snprintf(label, sizeof(label), "%s (wired)", path);
		} else {
			snprintf(label, sizeof(label), "%s (%u)", path,
			    (Uint)dep->count);
		}
		AG_TlistAddPtr(tl, AG_ObjectIcon(dep->obj), label, dep);
	}
	AG_UnlockLinkage();
	AG_TlistRestore(tl);
}

static void
PollProps(AG_Event *event)
{
	char val[AG_TLIST_LABEL_MAX];
	AG_Tlist *tl = AG_SELF();
	AG_Object *ob = AG_PTR(1);
	AG_Prop *prop;
	
	AG_TlistClear(tl);
	TAILQ_FOREACH(prop, &ob->props, props) {
		AG_PropPrint(val, sizeof(val), ob, prop->key);
		AG_TlistAdd(tl, NULL, "%s = %s", prop->key, val);
	}
	AG_TlistRestore(tl);
}

static void
PollEvents(AG_Event *event)
{
	extern const char *evarg_type_names[];
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
			char *argn = ev->argn[i];

			if (argn[0] != '\0') {
				strlcat(args, argn, sizeof(args));
				strlcat(args, "=", sizeof(args));
			}
			switch (ev->argt[i]) {
			case AG_EVARG_POINTER:
				snprintf(arg, sizeof(arg), "%p", ev->argv[i].p);
				break;
			case AG_EVARG_STRING:
				snprintf(arg, sizeof(arg), "\"%s\"",
				    ev->argv[i].s);
				break;
			case AG_EVARG_UCHAR:
			case AG_EVARG_CHAR:
				snprintf(arg, sizeof(arg), "'%c'",
				    (Uchar)ev->argv[i].i);
				break;
			case AG_EVARG_INT:
				snprintf(arg, sizeof(arg), "%d", ev->argv[i].i);
				break;
			case AG_EVARG_UINT:
				snprintf(arg, sizeof(arg), "%u",
				    (Uint)ev->argv[i].i);
				break;
			case AG_EVARG_LONG:
				snprintf(arg, sizeof(arg), "%li",
				    ev->argv[i].li);
				break;
			case AG_EVARG_ULONG:
				snprintf(arg, sizeof(arg), "%li",
				    (Ulong)ev->argv[i].li);
				break;
			case AG_EVARG_FLOAT:
				snprintf(arg, sizeof(arg), "<%g>",
				    ev->argv[i].f);
				break;
			case AG_EVARG_DOUBLE:
				snprintf(arg, sizeof(arg), "<%g>",
				    ev->argv[i].f);
				break;
			}
			strlcat(args, arg, sizeof(args));
			if (i < ev->argc-1) {
				strlcat(args, ", ", sizeof(args));
			}
		}
		strlcat(args, ")", sizeof(args));

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
		strlcpy(ob->name, tb->string, sizeof(ob->name));
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

#ifdef NETWORK

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
	size_t len;
	Uint working_rev, repo_rev;

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

	AG_TlistClear(tl);
	AG_RcsLog(objdir, tl);
	AG_TlistRestore(tl);

	AG_RcsDisconnect();
}

#endif /* NETWORK */

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

		tbox = AG_TextboxNew(ntab, AG_TEXTBOX_HFILL|AG_TEXTBOX_FOCUS,
		    _("Name: "));
		AG_TextboxPrintf(tbox, ob->name);
		AG_SetEvent(tbox, "textbox-return", RenameObject, "%p", ob);
		
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);
	
		AG_LabelNewStatic(ntab, 0, _("Class: %s"), ob->ops->type);
		AG_CheckboxSetFromFlags(ntab, &ob->flags, devObjectFlags);

		AG_LabelNewPolledMT(ntab, AG_LABEL_HFILL, &agLinkageLock,
		    _("Parent: %[obj]"), &ob->parent);
		AG_LabelNewStatic(ntab, 0, _("Save prefix: %s"),
		    ob->save_pfx != NULL ? ob->save_pfx : AG_PATHSEP);

		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);

		tbMD5 = AG_TextboxNew(ntab,
		    AG_TEXTBOX_HFILL|AG_TEXTBOX_READONLY, "MD5: ");
		AG_TextboxSizeHint(tbMD5, "888888888888888888888888888888888");
		AG_WidgetDisable(tbMD5);
		
		tbSHA1 = AG_TextboxNew(ntab,
		    AG_TEXTBOX_HFILL|AG_TEXTBOX_READONLY, "SHA1: ");
		AG_WidgetDisable(tbSHA1);
		
		tbRMD160 = AG_TextboxNew(ntab,
		    AG_TEXTBOX_HFILL|AG_TEXTBOX_READONLY, "RMD160: ");
		AG_WidgetDisable(tbRMD160);

		box = AG_BoxNew(ntab, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|
					            AG_BOX_HFILL);
		{
			btn = AG_ButtonNewFn(box, 0, _("Refresh checksums"),
			    RefreshSums, "%p,%p,%p,%p", ob,
			    tbMD5, tbSHA1, tbRMD160);
			AG_PostEvent(NULL, btn, "button-pushed", NULL);
		}
	}

#ifdef NETWORK
	ntab = AG_NotebookAddTab(nb, _("RCS"), AG_BOX_VERT);
	{
		AG_Label *lblStatus;
		AG_Tlist *tl;

		lblStatus = AG_LabelNewStatic(ntab, AG_LABEL_HFILL, "...");
		AG_LabelSizeHint(lblStatus, 3, _("Repository revision: #0000"));

		AG_LabelNewStaticString(ntab, 0, _("Revision history:"));
		tl = AG_TlistNew(ntab, AG_TLIST_EXPAND);

		btn = AG_ButtonNewFn(ntab, AG_BUTTON_HFILL, _("Refresh status"),
		    RefreshRepoStatus, "%p,%p,%p", ob, lblStatus, tl);

		if (agRcsMode)
			AG_PostEvent(NULL, btn, "button-pushed", NULL);
	}
#endif /* NETWORK */

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
	return (win);
}
