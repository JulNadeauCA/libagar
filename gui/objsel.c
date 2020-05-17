/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * A subclass of AG_Combo(3) for selecting an element in a VFS of AG_Objects.
 */

#include <agar/core/core.h>
#if defined(AG_WIDGETS)

#include <agar/gui/objsel.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/label.h>
#include <agar/gui/iconmgr.h>

#include <stdarg.h>
#include <string.h>

AG_ObjectSelector *
AG_ObjectSelectorNew(void *parent, int flags, void *pobj, void *root,
    const char *fmt, ...)
{
	char *label;
	AG_ObjectSelector *os;
	va_list ap;

	os = Malloc(sizeof(AG_ObjectSelector));
	AG_ObjectInit(os, &agObjectSelectorClass);
	os->flags |= flags;
	os->pobj = pobj;
	os->root = root;

	va_start(ap, fmt);
	Vasprintf(&label, fmt, ap);
	va_end(ap);
	AG_TextboxSetLabelS(os->com.tbox, label);
	free(label);

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

	it = AG_TlistAddS(tl, NULL, pob->name);
	it->depth = depth;
	it->cat = "object";
	it->p1 = pob;
	if (nosel) {
		it->flags |= AG_TLIST_NO_SELECT;
	}
	if (!TAILQ_EMPTY(&pob->children)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
		if (pob->parent == NULL)
			it->flags |= AG_TLIST_ITEM_EXPANDED;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN)) {
		TAILQ_FOREACH(cob, &pob->children, cobjs)
			FindObjects(os, tl, cob, depth+1);
	}
}

static void
PollObjects(AG_Event *event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_ObjectSelector *os = AG_OBJECTSELECTOR_PTR(1);

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
	AG_ObjectSelector *os = AG_OBJECTSELECTOR_PTR(1);
	const AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	AG_Variable *V;
	void **object;
	
	V = AG_GetVariable(os, "object", (void *)&object);

	if (*object != NULL) {
		if (os->flags & AG_OBJSEL_PAGE_DATA)
			AG_ObjectPageOut(*object);
	}
	*object = it->p1;

	if (os->flags & AG_OBJSEL_PAGE_DATA) {
		AG_ObjectPageIn(*object);
	}
	AG_UnlockVariable(V);
}

static void
Bound(AG_Event *event)
{
	AG_ObjectSelector *os = AG_OBJECTSELECTOR_SELF();
	const AG_Variable *V = AG_PTR(1);

	if (strcmp(V->name, "object") == 0) {
		void **object = V->data.p;
	
		if (*object != NULL)
			AG_ComboSelectPointer(&os->com, *object);
	}
}

static void
Init(void *obj)
{
	AG_Combo *com = obj;
	AG_ObjectSelector *os = obj;

	WIDGET(os)->flags |= AG_WIDGET_HFILL;

	com->flags |= AG_COMBO_POLL;
	com->list->flags |= AG_TLIST_POLL;
	
	os->object = NULL;
	os->flags = 0;
	os->pobj = NULL;
	os->root = NULL;
	os->type_mask[0] = '*';
	os->type_mask[1] = '\0';

	AG_BindPointer(os, "object", &os->object);

	AG_SetEvent(os->com.list, "tlist-poll", PollObjects, "%p", os);
	AG_SetEvent(&os->com, "combo-selected", SelectObject, "%p", os);
	
	AG_SetEvent(os, "bound", Bound, NULL);
	OBJECT(os)->flags |= AG_OBJECT_BOUND_EVENTS;
}

void
AG_ObjectSelectorMaskType(AG_ObjectSelector *os, const char *type)
{
	AG_OBJECT_ISA(os, "AG_Widget:AG_Combo:AG_ObjectSelector:*");
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
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* draw */
	NULL,			/* size_request */
	NULL			/* size_allocate */
};

#endif /* AG_WIDGETS */
