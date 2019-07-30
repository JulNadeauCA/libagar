/*
 * Copyright (c) 2009-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Tool for selecting and moving entities in a component schematic.
 */

#include <agar/core/core.h>
#include <agar/core/limits.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

typedef struct vg_select_tool {
	VG_Tool _inherit;
	Uint flags;
#define MOVING_ENTITIES	0x01	/* Translation is in progress */
	VG_Vector vLast;	/* For grid snapping */
	Uint32 _pad;
	VG_Node *vnMouseOver;	/* Element under cursor */
} VG_SelectTool;

/* Return the entity nearest to vPos. */
static void *_Nullable
ProximityQuery(VG_View *_Nonnull vv, VG_Vector vPos)
{
	VG *vg = vv->vg;
	float prox, proxNearest;
	VG_Node *vn, *vnNearest;
	VG_Vector v;

#if 0
	/* First check if we intersect a block. */
	TAILQ_FOREACH(vn, &vg->nodes, list) {
		if (!VG_NodeIsClass(vn, "Block")) {
			continue;
		}
		v = vPos;
		prox = vn->ops->pointProximity(vn, vv, &v);
		if (prox == 0.0f)
			return (vn);
	}
#endif

	/* Then prioritize points at a fixed distance. */
	proxNearest = AG_FLT_MAX;
	vnNearest = NULL;
	TAILQ_FOREACH(vn, &vg->nodes, list) {
		if (!VG_NodeIsClass(vn, "Point")) {
			continue;
		}
		v = vPos;
		prox = vn->ops->pointProximity(vn, vv, &v);
		if (prox <= vv->pointSelRadius) {
			if (prox < proxNearest) {
				proxNearest = prox;
				vnNearest = vn;
			}
		}
	}
	if (vnNearest != NULL)
		return (vnNearest);

	/* Finally, fallback to a general query. */
	proxNearest = AG_FLT_MAX;
	vnNearest = NULL;
	TAILQ_FOREACH(vn, &vg->nodes, list) {
		if (vn->ops->pointProximity == NULL) {
			continue;
		}
		v = vPos;
		prox = vn->ops->pointProximity(vn, vv, &v);
		if (prox < proxNearest) {
			proxNearest = prox;
			vnNearest = vn;
		}
	}
	return (vnNearest);
}

static int
MouseButtonDown(void *_Nonnull obj, VG_Vector v, int b)
{
	VG_SelectTool *t = obj;
	VG_View *vv = VGTOOL(t)->vgv;
	VG_Node *vn;

	if ((vn = ProximityQuery(vv, v)) == NULL)
		return (0);

	VG_ClearEditAreas(vv);

	switch (b) {
	case AG_MOUSE_LEFT:
		t->vLast = v;
		if (VG_SELECT_MULTI(vv)) {
			if (vn->flags & VG_NODE_SELECTED) {
				vn->flags &= ~(VG_NODE_SELECTED);
			} else {
				vn->flags |= VG_NODE_SELECTED;
				VG_EditNode(vv, 0, vn);
			}
		} else {
			VG_UnselectAll(vv->vg);
			vn->flags |= VG_NODE_SELECTED;
			VG_EditNode(vv, 0, vn);
		}
		t->flags |= MOVING_ENTITIES;
		return (1);
	default:
		return (0);
	}
}

static int
MouseButtonUp(void *_Nonnull obj, VG_Vector v, int b)
{
	VG_SelectTool *t = obj;

	if (b != AG_MOUSE_LEFT) {
		return (0);
	}
	t->flags &= ~(MOVING_ENTITIES);
	return (1);
}

static int
MouseMotion(void *_Nonnull obj, VG_Vector vPos, VG_Vector vRel, int buttons)
{
	VG_SelectTool *t = obj;
	VG_View *vv = VGTOOL(t)->vgv;
	VG *vg = vv->vg;
	VG_Node *vn;
	VG_Vector v;

	/* Provide visual feedback of current selection. */
	if ((t->flags & MOVING_ENTITIES) == 0) {
		if ((vn = ProximityQuery(vv, vPos)) != NULL &&
		    t->vnMouseOver != vn) {
			t->vnMouseOver = vn;
			VG_Status(vv, _("Select schematic entity: %s%u"),
			    vn->ops->name, (Uint)vn->handle);
		}
		return (0);
	}

	/* Translate any selected entities. */
	v = vPos;
	if (!VG_SKIP_CONSTRAINTS(vv)) {
		VG_Vector vLast, vSnapRel;

		vLast = t->vLast;
		VG_ApplyConstraints(vv, &v);
		VG_ApplyConstraints(vv, &vLast);
		vSnapRel.x = v.x - vLast.x;
		vSnapRel.y = v.y - vLast.y;

		if (vSnapRel.x != 0.0 || vSnapRel.y != 0.0) {
			TAILQ_FOREACH(vn, &vg->nodes, list) {
				if (!(vn->flags & VG_NODE_SELECTED) ||
				    vn->ops->moveNode == NULL) {
					continue;
				}
				vn->ops->moveNode(vn, v, vSnapRel);
				VG_Status(vv, _("Moving entity: %s%u (grid)"),
				    vn->ops->name, (Uint)vn->handle);
			}
			t->vLast = v;
		}
	} else {
		TAILQ_FOREACH(vn, &vg->nodes, list) {
			if (!(vn->flags & VG_NODE_SELECTED) ||
			    vn->ops->moveNode == NULL) {
				continue;
			}
			vn->ops->moveNode(vn, v, vRel);
			VG_Status(vv, _("Moving entity: %s%u (free)"),
			    vn->ops->name, (Uint)vn->handle);
		}
	}
	return (0);
}

static int
KeyDown(void *_Nonnull obj, int ksym, int kmod, Uint32 unicode)
{
	VG_SelectTool *t = obj;
	VG_View *vv = VGTOOL(t)->vgv;
	VG_Node *vn;
	Uint nDel = 0;

	if (ksym == AG_KEY_DELETE || ksym == AG_KEY_BACKSPACE) {
del:
		TAILQ_FOREACH(vn, &vv->vg->nodes, list) {
			if (vn->flags & VG_NODE_SELECTED) {
				VG_ClearEditAreas(vv);
				if (VG_Delete(vn) == -1) {
					vn->flags &= ~(VG_NODE_SELECTED);
					VG_Status(vv, "%s%u: %s",
					    vn->ops->name, (Uint)vn->handle,
					    AG_GetError());
					return (0);
				} else {
					nDel++;
				}
				goto del;
			}
		}
		VG_Status(vv, _("Deleted %u entities"), nDel);
		return (1);
	}
	return (0);
}

static void
Init(void *_Nonnull obj)
{
	VG_SelectTool *t = obj;

	t->flags = 0;
}

VG_ToolOps vgSelectTool = {
	N_("Select entities"),
	N_("Select / move entities in the drawing."),
	&vgIconSelectArrow,
	sizeof(VG_SelectTool),
	VG_NOSNAP|VG_NOEDITCLEAR,
	Init,
	NULL,			/* destroy */
	NULL,			/* edit */
	NULL,			/* predraw */
	NULL,			/* postdraw */
	NULL,			/* selected */
	NULL,			/* deselected */
	MouseMotion,
	MouseButtonDown,
	MouseButtonUp,
	KeyDown,
	NULL			/* keyup */
};
