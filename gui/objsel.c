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
#include <core/view.h>

#include "objsel.h"

#include "window.h"
#include "primitive.h"
#include "label.h"

#include <stdarg.h>
#include <string.h>

static AG_WidgetOps agObjectSelectorOps = {
	{
		"AG_Widget:AG_Combo:AG_ObjectSelector",
		sizeof(AG_ObjectSelector),
		{ 0,0 },
		NULL,					/* init */
		NULL,					/* reinit */
		AG_ComboDestroy,
		NULL,					/* load */
		NULL,					/* save */
		NULL					/* edit */
	},
	NULL,						/* draw */
	AG_ComboScale
};

AG_ObjectSelector *
AG_ObjectSelectorNew(void *parent, int flags, void *pobj, void *root,
    const char *fmt, ...)
{
	char label[AG_LABEL_MAX];
	AG_ObjectSelector *os;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	os = Malloc(sizeof(AG_ObjectSelector), M_OBJECT);
	AG_ObjectSelectorInit(os, label, flags, pobj, root);
	AG_ObjectAttach(parent, os);
	return (os);
}

/* Select an object in the list. */
AG_TlistItem *
AG_ObjectSelectorSelect(AG_ObjectSelector *os, void *p)
{
	AG_TlistItem *it;

	return (AG_ComboSelectPointer(&os->com, p));
}

static void
FindObjects(AG_ObjectSelector *os, AG_Tlist *tl, AG_Object *pob, int depth)
{
	AG_Object *cob;
	AG_TlistItem *it;
	int nosel = 0;
	
	if (!AG_ObjectIsClass(pob, os->type_mask)) {
		if (!TAILQ_EMPTY(&pob->children)) {
			nosel++;
		} else {
			return;
		}
	}

	it = AG_TlistAdd(tl, AG_ObjectIcon(pob), "%s", pob->name);
	it->depth = depth;
	it->cat = "object";
	it->p1 = pob;
	if (nosel) {
		it->flags |= AG_TLIST_NO_SELECT;
	}
	if (!TAILQ_EMPTY(&pob->children)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
		if (AG_ObjectRoot(pob) == pob)
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
	AG_LockLinkage();
	FindObjects(os, tl, os->root, 0);
	AG_UnlockLinkage();
	AG_TlistRestore(tl);
}

static void
SelectObject(AG_Event *event)
{
	AG_Combo *com = AG_SELF();
	AG_ObjectSelector *os = AG_PTR(1);
	AG_TlistItem *it = AG_PTR(2);
	AG_WidgetBinding *objectb;
	void **object;
	
	objectb = AG_WidgetGetBinding(os, "object", &object);

	if (*object != NULL) {
		if (os->flags & AG_OBJSEL_PAGE_DATA) {
			AG_ObjectPageOut(*object);
		}
		AG_ObjectDelDep(os->pobj, *object);
	}

	*object = it->p1;

	AG_ObjectAddDep(os->pobj, *object);

	if (os->flags & AG_OBJSEL_PAGE_DATA) {
		AG_ObjectPageIn(*object);
	}
	AG_WidgetUnlockBinding(objectb);
}

static void
bound(AG_Event *event)
{
	AG_ObjectSelector *os = AG_SELF();
	AG_WidgetBinding *b = AG_PTR(1);

	if (strcmp(b->name, "object") == 0) {
		void **object = b->p1;
	
		if (*object != NULL)
			AG_ComboSelectPointer(&os->com, *object);
	}
}

void
AG_ObjectSelectorInit(AG_ObjectSelector *os, const char *label, int flags,
    void *pobj, void *root)
{
	AG_ComboInit(&os->com, AG_COMBO_POLL|AG_COMBO_TREE|AG_COMBO_HFILL,
	    label);
	AG_ObjectSetOps(os, &agObjectSelectorOps);

	os->flags = flags;
	os->pobj = pobj;
	os->root = root;
	os->type_mask[0] = '*';
	os->type_mask[1] = '\0';

	AG_WidgetBind(os, "object", AG_WIDGET_POINTER, &os->object);

	AG_SetEvent(os->com.list, "tlist-poll", PollObjects, "%p", os);
	AG_SetEvent(&os->com, "combo-selected", SelectObject, "%p", os);
	AG_SetEvent(os, "widget-bound", bound, NULL);
}

void
AG_ObjectSelectorMaskType(AG_ObjectSelector *os, const char *type)
{
	strlcpy(os->type_mask, type, sizeof(os->type_mask));
}
