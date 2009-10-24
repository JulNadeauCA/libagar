/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

#include "objsel.h"

#include "window.h"
#include "primitive.h"
#include "label.h"
#include "iconmgr.h"

#include <stdarg.h>
#include <string.h>

AG_ObjectSelector *
AG_ObjectSelectorNew(void *parent, int flags, void *pobj, void *root,
    const char *fmt, ...)
{
	AG_ObjectSelector *os;
	va_list ap;

	os = Malloc(sizeof(AG_ObjectSelector));
	AG_ObjectInit(os, &agObjectSelectorClass);
	os->flags |= flags;
	os->pobj = pobj;
	os->root = root;
	
	if (fmt != NULL) {
		char label[AG_LABEL_MAX];

		va_start(ap, fmt);
		Vsnprintf(label, sizeof(label), fmt, ap);
		va_end(ap);
		AG_TextboxSetLabelS(os->com.tbox, label);
	}
	AG_ObjectAttach(parent, os);
	return (os);
}

/* Select an object in the list. */
AG_TlistItem *
AG_ObjectSelectorSelect(AG_ObjectSelector *os, void *p)
{
	return (AG_ComboSelectPointer(&os->com, p));
}

static void
FindObjects(AG_ObjectSelector *os, AG_Tlist *tl, AG_Object *pob, int depth)
{
	AG_Object *cob;
	AG_TlistItem *it;
	int nosel = 0;
	
	if (!AG_OfClass(pob, os->type_mask)) {
		if (!TAILQ_EMPTY(&pob->children)) {
			nosel++;
		} else {
			return;
		}
	}

	it = AG_TlistAddS(tl, AG_ObjectIcon(pob), pob->name);
	it->depth = depth;
	it->cat = "object";
	it->p1 = pob;
	if (nosel) {
		it->flags |= AG_TLIST_NO_SELECT;
	}
	if (!TAILQ_EMPTY(&pob->children)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
		if (pob->parent == NULL)
			it->flags |= AG_TLIST_VISIBLE_CHILDREN;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN)) {
		TAILQ_FOREACH(cob, &pob->children, cobjs)
			FindObjects(os, tl, cob, depth+1);
	}
}

static void
PollObjects(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_ObjectSelector *os = AG_PTR(1);

	AG_TlistClear(tl);
	AG_ObjectLock(os);
	AG_LockVFS(os->root);
	FindObjects(os, tl, os->root, 0);
	AG_UnlockVFS(os->root);
	AG_ObjectUnlock(os);
	AG_TlistRestore(tl);
}

static void
SelectObject(AG_Event *event)
{
	AG_ObjectSelector *os = AG_PTR(1);
	AG_TlistItem *it = AG_PTR(2);
	AG_Variable *objectb;
	void **object;
	
	objectb = AG_GetVariable(os, "object", &object);

	if (*object != NULL) {
		if (os->flags & AG_OBJSEL_PAGE_DATA) {
			AG_ObjectPageOut(*object);
		}
		AG_ObjectDelDep(os->pobj, *object);
	}

	*object = it->p1;

	AG_ObjectAddDep(os->pobj, *object, 1);

	if (os->flags & AG_OBJSEL_PAGE_DATA) {
		AG_ObjectPageIn(*object);
	}
	AG_UnlockVariable(objectb);
}

static void
Bound(AG_Event *event)
{
	AG_ObjectSelector *os = AG_SELF();
	AG_Variable *b = AG_PTR(1);

	if (strcmp(b->name, "object") == 0) {
		void **object = b->data.p;
	
		if (*object != NULL)
			AG_ComboSelectPointer(&os->com, *object);
	}
}

static void
Init(void *obj)
{
	AG_Combo *com = obj;
	AG_ObjectSelector *os = obj;

	AG_ExpandHoriz(os);

	com->flags |= AG_COMBO_POLL;
	com->list->flags |= AG_TLIST_POLL|AG_TLIST_TREE;
	
	os->flags = 0;
	os->pobj = NULL;
	os->root = NULL;
	os->type_mask[0] = '*';
	os->type_mask[1] = '\0';

	AG_BindPointer(os, "object", &os->object);

	AG_SetEvent(os->com.list, "tlist-poll", PollObjects, "%p", os);
	AG_SetEvent(&os->com, "combo-selected", SelectObject, "%p", os);
	AG_SetEvent(os, "bound", Bound, NULL);
}

void
AG_ObjectSelectorMaskType(AG_ObjectSelector *os, const char *type)
{
	AG_ObjectLock(os);
	Strlcpy(os->type_mask, type, sizeof(os->type_mask));
	AG_ObjectUnlock(os);
}

AG_WidgetClass agObjectSelectorClass = {
	{
		"Agar(Widget:Combo:ObjectSelector)",
		sizeof(AG_ObjectSelector),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_WidgetInheritDraw,
	AG_WidgetInheritSizeRequest,
	AG_WidgetInheritSizeAllocate
};
