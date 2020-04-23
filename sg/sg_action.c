/*
 * Copyright (c) 2012-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Node actions.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

#include <string.h>

const char *sgActionNames[] = {
	N_("None"),
	N_("Move"),	N_("MoveBegin"),	N_("MoveEnd"),
	N_("ZMove"),	N_("ZMoveBegin"),	N_("ZMoveEnd"),
	N_("Rotate"),	N_("RotateBegin"),	N_("RotateEnd"),
	N_("Scale"),	N_("ScaleBegin"),	N_("ScaleEnd"),
};

/* Initialize a node action structure. */
void
SG_ActionInit(SG_Action *act, enum sg_action_type type)
{
	act->type = type;
	act->flags = 0;
	act->keyMod = 0;
	TAILQ_INIT(&act->widgets);

	switch (type) {
	case SG_ACTION_MOVE:
		act->key = AG_KEY_M;
		act->args.move = M_VecZero3();
		break;
	case SG_ACTION_ZMOVE:
		act->key = AG_KEY_Z;
		act->args.move = M_VecZero3();
		break;
	case SG_ACTION_ROTATE:
		act->key = AG_KEY_R;
		act->args.rotate.axis = M_VecZero3();
		act->args.rotate.theta = 0.0;
		break;
	case SG_ACTION_SCALE:
		act->key = AG_KEY_S;
		act->args.scale = M_VecZero3();
		break;
	default:
		act->key = AG_KEY_NONE;
		break;
	}
}

/* Copy node action arguments. */
void
SG_ActionCopy(SG_Action *da, const SG_Action *sa)
{
	da->type = sa->type;
	da->flags = sa->flags;
	da->key = sa->key;

	switch (sa->type) {
	case SG_ACTION_MOVE:
	case SG_ACTION_ZMOVE:
		da->act_move = sa->act_move;
		break;
	case SG_ACTION_ROTATE:
		da->act_rotate = sa->act_rotate;
		break;
	case SG_ACTION_SCALE:
		da->act_scale = sa->act_scale;
		break;
	default:
		break;
	}
}

/* Enable the specified node action. */
int
SG_EnableAction(void *obj, enum sg_action_type type)
{
	SG_Node *node = obj;
	SG_Action *act;

	if ((act = TryMalloc(sizeof(SG_Action))) == NULL) {
		return (-1);
	}
	SG_ActionInit(act, type);
	TAILQ_INSERT_TAIL(&node->actions, act, actions);
	return (0);
}

/* Lookup the specified node action. */
SG_Action *
SG_FetchAction(void *obj, enum sg_action_type type)
{
	SG_Node *node = obj;
	SG_Action *act;

	AG_ObjectLock(node);
	TAILQ_FOREACH(act, &node->actions, actions) {
		if (act->type == type)
			break;
	}
	AG_ObjectUnlock(node);
	return (act);
}

/* Disable the specified node action. */
void
SG_DisableAction(void *obj, enum sg_action_type type)
{
	SG_Node *node = obj;
	SG_Action *act;

	AG_ObjectLock(node);
	TAILQ_FOREACH(act, &node->actions, actions) {
		if (act->type == type)
			break;
	}
	if (act != NULL) {
		TAILQ_REMOVE(&node->actions, act, actions);
		Free(act);
	}
	AG_ObjectUnlock(node);
}

void
SG_ActionPrint(const SG_Action *a, char *buf, AG_Size len)
{
	if (a->type >= SG_ACTION_LAST) {
		if (len > 0) { buf[0] = '\0'; }
		return;
	}
	switch (a->type) {
	case SG_ACTION_MOVE:
	case SG_ACTION_MOVE_BEGIN:
	case SG_ACTION_MOVE_END:
	case SG_ACTION_ZMOVE:
	case SG_ACTION_ZMOVE_BEGIN:
	case SG_ACTION_ZMOVE_END:
		Snprintf(buf, len, "%s(%.03f,%.03f,%.03f)",
		    sgActionNames[a->type],
		    a->act_move.x,
		    a->act_move.y,
		    a->act_move.z);
		break;
	case SG_ACTION_ROTATE:
	case SG_ACTION_ROTATE_BEGIN:
	case SG_ACTION_ROTATE_END:
		Snprintf(buf, len, "%s(%.01f\xc2\xb0 @ %.01f,%.01f,%.01f)",
		    sgActionNames[a->type],
		    M_Degrees(a->act_rotate.theta),
		    a->act_rotate.axis.x,
		    a->act_rotate.axis.y,
		    a->act_rotate.axis.z);
		break;
	case SG_ACTION_SCALE:
	case SG_ACTION_SCALE_BEGIN:
	case SG_ACTION_SCALE_END:
		Snprintf(buf, len, "%s(%.03f,%.03f,%.03f)",
		    sgActionNames[a->type],
		    a->act_scale.x,
		    a->act_scale.y,
		    a->act_scale.z);
		break;
	default:
		Snprintf(buf, len, "%s()", sgActionNames[a->type]);
		break;
	}
}

int
SG_ActionLoad(SG_Action *a, AG_DataSource *ds)
{
	a->type = (enum sg_action_type)AG_ReadUint8(ds);
	if (a->type >= SG_ACTION_LAST) {
		AG_SetError("Bad action: %u", a->type);
		return (-1);
	}
	a->flags &= ~(SG_ACTION_SAVED);
	a->flags |= (Uint)AG_ReadUint8(ds) & SG_ACTION_SAVED;
	
	switch (a->type) {
	case SG_ACTION_MOVE:
	case SG_ACTION_ZMOVE:
		a->act_move = M_ReadVector3(ds);
		break;
	case SG_ACTION_ROTATE:
		a->act_rotate.axis = M_ReadVector3(ds);
		a->act_rotate.theta = M_ReadReal(ds);
		break;
	case SG_ACTION_SCALE:
		a->act_scale = M_ReadVector3(ds);
		break;
	default:
		break;
	}
	return (0);
}

int
SG_ActionSave(SG_Action *a, AG_DataSource *ds)
{
	AG_WriteUint8(ds, (Uint8)a->type);
	AG_WriteUint8(ds, a->flags & SG_ACTION_SAVED);
	
	switch (a->type) {
	case SG_ACTION_MOVE:
	case SG_ACTION_ZMOVE:
		M_WriteVector3(ds, &a->act_move);
		break;
	case SG_ACTION_ROTATE:
		M_WriteVector3(ds, &a->act_rotate.axis);
		M_WriteReal(ds, a->act_rotate.theta);
		break;
	case SG_ACTION_SCALE:
		M_WriteVector3(ds, &a->act_scale);
		break;
	default:
		break;
	}
	return (0);
}
