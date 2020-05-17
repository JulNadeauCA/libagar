/*
 * Copyright (c) 2011-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Agar-SG script interpreter and editor.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>
#include <agar/gui/opengl.h>
#include <agar/gui/packedpixel.h>
#include <agar/gui/begin.h>

#include <string.h>
#include <ctype.h>

const char *sgScriptInsnNames[] = {
	N_("Noop"),
	N_("Create"),
	N_("Delete"),
	N_("Action"),
	N_("CamAction")
};

/* Edition context for a SG_Script. */
typedef struct sg_script_edit_ctx {
	AG_Window *win;			/* Parent window */
	SG_Script *scr;			/* Script object */
	SG *sg;				/* Constructed scene */
	SG_View *sv;			/* Visualization widget */
	AG_Menu *menu;			/* Main menu */
	AG_Pane *paHoriz;		/* Main horizontal pane */
	AG_Pane *paLeft;		/* Left side vertical pane */
	AG_Box *boxBtns;		/* Save/Cancel buttons */
	AG_Widget *wEdit;		/* Edit area */
	SG_ScriptInsn *siNew;		/* New instruction */
	AG_Label *stat;			/* Status bar label */
	AG_Timer toCamMove;		/* Camera motion timer */
	Uint8 _pad1[8];
	M_Vector3   vCamMove;		/* Camera motion vector */
	M_Vector3   vCamMoveSum;	/* Camera motion vector sum */
	int          camMoving;
	Uint32 _pad2;
	AG_Slider *slTime;		/* Time slider */
	AG_Surface *suUnder;		/* Underlay (for animation) */
	Uint8 _pad3[8];
} SG_ScriptEditCtx;

/* Rendering context for a SG_Script. */
typedef struct sg_script_render_ctx {
	int nFirst, nLast;			/* Frames to render */
	double timeScale;			/* Time scale */
	double fps;				/* Frames per second */
	enum sg_script_interp_mode interp;	/* Interpolation method */
	int iInt;				/* Current interpolated frame# */
	int nInt;				/* Interpolated video frames per frame */
	int clearDir;				/* Clear target dir on render */
} SG_ScriptRenderCtx;

/* Resolve an instruction's target node. */
static __inline__ void *_Nullable
GetInsnTarget(SG *_Nonnull sg, SG_ScriptInsn *_Nonnull si)
{
	return AG_ObjectFindS(sg, si->tgtName);
}

/* Create a new script instance. */
SG_Script *
SG_ScriptNew(void *parent, const char *name)
{
	SG_Script *scr;

	scr = Malloc(sizeof(SG_Script));
	AG_ObjectInit(scr, &sgScriptClass);
	if (name) {
		AG_ObjectSetNameS(scr, name);
	} else {
		OBJECT(scr)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, scr);
	return (scr);
}

static void
Init(void *_Nonnull obj)
{
	SG_Script *scr = obj;

	scr->flags = 0;
	scr->fps = 0;
	scr->n = 0;
	scr->t = 0;
	scr->tPrev = -1;
	scr->tFirst = 0;
	scr->tLast = 0;
	scr->frames = NULL;

	if ((SG_ScriptAlloc(scr, 201)) == -1)
		AG_FatalError(NULL);
}

static void
Reset(void *_Nonnull obj)
{
	SG_Script *scr = obj;
	Uint i;

	for (i = 0; i < scr->n; i++) {
		SG_ScriptFrame *sf = &scr->frames[i];
		SG_ScriptInsn *si, *siNext;

		for (si = TAILQ_FIRST(&sf->insns);
		     si != TAILQ_END(&sf->insns);
		     si = siNext) {
			siNext = TAILQ_NEXT(si, insns);
			SG_ScriptInsnFree(si);
		}
	}
	Free(scr->frames);
	scr->frames = NULL;
	scr->n = 0;
	scr->tFirst = 0;
	scr->tLast = 0;
	scr->tPrev = 0;
}

static int
LoadInsn(SG_Script *_Nonnull scr, SG_ScriptInsn *_Nonnull si,
    AG_DataSource *_Nonnull ds)
{
	enum sg_script_insn_type siType;

	siType = (enum sg_script_insn_type)AG_ReadUint8(ds);
	if (siType >= SG_INSN_LAST) {
		AG_SetError("Bad insn type: %d", siType);
		return (-1);
	}
	si->type = siType;
	si->flags &= ~(SG_SCRIPT_INSN_SAVED);
	si->flags |= (AG_ReadUint8(ds) & SG_SCRIPT_INSN_SAVED);
	si->tgtName = AG_ReadStringLen(ds, AG_OBJECT_PATH_MAX);
	if (si->tgtName[0] == '\0') { Free(si->tgtName); si->tgtName = NULL; }

	switch (si->type) {
	case SG_INSN_CREATE:
		{
			char clsName[AG_OBJECT_HIER_MAX];
			AG_ObjectClass *cls;
			AG_Size dataSize;

			si->si_create.name = AG_ReadStringLen(ds, AG_OBJECT_NAME_MAX);
			if (si->si_create.name == NULL) {
				return (-1);
			}
			AG_CopyString(clsName, ds, sizeof(clsName));
			if ((cls = AG_LookupClass(clsName)) == NULL) {
				return (-1);
			}
			dataSize = (AG_Size)AG_ReadUint32(ds);
			if ((si->si_create.data = TryMalloc(dataSize)) == NULL) {
				return (-1);
			}
			si->si_create.cls = cls;
			si->si_create.size = dataSize;

			if (AG_Read(ds, si->si_create.data, si->si_create.size) != 0)
				return (-1);
		}
		break;
	case SG_INSN_ACTION:
	case SG_INSN_CAMACTION:
		return SG_ActionLoad(&si->si_action, ds);
	default:
		break;
	}
	return (0);
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds,
    const AG_Version *_Nonnull ver)
{
	SG_Script *scr = obj;
	Uint nFrames, fr, i;

	nFrames = (Uint)AG_ReadUint32(ds);
	if (SG_ScriptAlloc(scr, nFrames) == -1)
		return (-1);

	scr->flags &= ~(SG_SCRIPT_SAVED);
	scr->flags |= ((Uint)AG_ReadUint32(ds) & SG_SCRIPT_SAVED);
	scr->fps = AG_ReadDouble(ds);
	(void)AG_ReadSint32(ds);		/* Ignore last t */
	scr->t = 0;
	scr->tFirst = (int)AG_ReadSint32(ds);
	scr->tLast = (int)AG_ReadSint32(ds);
	scr->tPrev = -1;

	for (fr = 0; fr < scr->n; fr++) {
		SG_ScriptInsn *si;
		Uint nInsns;

		nInsns = AG_ReadUint32(ds);
		for (i = 0; i < nInsns; i++) {
			if ((si = SG_ScriptInsnNew(scr)) == NULL) {
				return (-1);
			}
			if (LoadInsn(scr, si, ds) == -1) {
				SG_ScriptInsnFree(si);
				return (-1);
			}
			SG_ScriptAddInsn(scr, fr, si);
		}
	}
	return (0);
}

static void
SaveInsn(SG_Script *_Nonnull scr, SG_ScriptInsn *_Nonnull si,
    AG_DataSource *_Nonnull ds)
{
	AG_WriteUint8(ds, (Uint8)si->type);
	AG_WriteUint8(ds, (Uint8)si->flags);
	AG_WriteString(ds, (si->tgtName != NULL) ? si->tgtName : "");

	switch (si->type) {
	case SG_INSN_CREATE:
		AG_WriteString(ds, si->si_create.name);
		AG_WriteString(ds, si->si_create.cls->hier);
		AG_WriteUint32(ds, (Uint32)si->si_create.size);
		AG_Write(ds, si->si_create.data, si->si_create.size);
		break;
	case SG_INSN_ACTION:
	case SG_INSN_CAMACTION:
		SG_ActionSave(&si->si_action, ds);
	default:
		break;
	}
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	SG_Script *scr = obj;
	Uint fr;

	AG_WriteUint32(ds, (Uint32)scr->n);
	AG_WriteUint32(ds, (Uint32)(scr->flags & SG_SCRIPT_SAVED));
	AG_WriteDouble(ds, scr->fps);
	AG_WriteSint32(ds, scr->t);
	AG_WriteSint32(ds, scr->tFirst);
	AG_WriteSint32(ds, scr->tLast);

	for (fr = 0; fr < scr->n; fr++) {
		SG_ScriptInsn *si;
		Uint32 count = 0;
		
		TAILQ_FOREACH(si, &scr->frames[fr].insns, insns) {
			count++;
		}
		AG_WriteUint32(ds, count);

		TAILQ_FOREACH(si, &scr->frames[fr].insns, insns) {
			SaveInsn(scr, si, ds);
		}
	}
	return (0);
}

/* Allocate time frames. */
int
SG_ScriptAlloc(SG_Script *scr, Uint n)
{
	SG_ScriptFrame *framesNew;
	Uint i;

	if (n <= scr->n) {
		scr->n = n;
		scr->tLast = (n-1);
		return (0);
	}
	if ((framesNew = TryRealloc(scr->frames, n*sizeof(SG_ScriptFrame)))
	    == NULL) {
		return (-1);
	}
	scr->frames = framesNew;
	for (i = scr->n; i < n; i++) {
		TAILQ_INIT(&scr->frames[i].insns);
	}
	scr->n = n;
	scr->tLast = (n-1);
	return (0);
}

/* Allocate a new script instruction. */
SG_ScriptInsn *
SG_ScriptInsnNew(SG_Script *scr)
{
	SG_ScriptInsn *si;

	if ((si = TryMalloc(sizeof(SG_ScriptInsn))) == NULL) {
		return (NULL);
	}
	si->type = SG_INSN_NOOP;
	si->flags = 0;
	si->tgtName = NULL;
	return (si);
}

/* Insert instruction in specified time frame, at tail. */
int
SG_ScriptAddInsn(SG_Script *scr, Uint t, SG_ScriptInsn *si)
{
	SG_ScriptFrame *sf;

	if (t >= scr->n) {
		AG_SetError("Bad frame#: %u", t);
		return (-1);
	}
	sf = &scr->frames[t];
	TAILQ_INSERT_TAIL(&sf->insns, si, insns);
	return (0);
}

/* Insert instruction in specified time frame, before another instruction. */
int
SG_ScriptAddInsnBefore(SG_Script *scr, Uint t, SG_ScriptInsn *siOther,
    SG_ScriptInsn *si)
{
	if (t >= scr->n) {
		AG_SetError("Bad frame#: %u", t);
		return (-1);
	}
	TAILQ_INSERT_BEFORE(siOther, si, insns);
	return (0);
}

/* Destroy a script instruction structure. */
void
SG_ScriptInsnFree(SG_ScriptInsn *si)
{
	switch (si->type) {
	case SG_INSN_CREATE:
		Free(si->si_create.name);
		Free(si->si_create.data);
		break;
	default:
		break;
	}
	Free(si->tgtName);
	Free(si);
}

/* Remove specified instruction from script (without freeing it). */
int
SG_ScriptDelInsn(SG_Script *scr, Uint t, SG_ScriptInsn *si)
{
	if (t >= scr->n) {
		AG_SetError("Bad frame#: %u", t);
		return (-1);
	}
	TAILQ_REMOVE(&scr->frames[t].insns, si, insns);
	return (0);
}

/* Print the given instruction to a fixed-size buffer. */
void
SG_ScriptPrintInsn(const SG_ScriptInsn *si, char *buf, AG_Size len)
{
	if (si->type >= SG_INSN_LAST) {
		if (len > 0) { buf[0] = '\0'; }
		return;
	}
	switch (si->type) {
	case SG_INSN_CREATE:
		Snprintf(buf, len, "%s(<%s> %s%s)",
		    sgScriptInsnNames[si->type],
		    si->si_create.cls->name,
		    si->tgtName, si->si_create.name);
		break;
	case SG_INSN_ACTION:
	case SG_INSN_CAMACTION:
		{
			char abuf[128];

			SG_ActionPrint(&si->si_action, abuf, sizeof(abuf));
			Snprintf(buf, len, "%s(%s -> %s): %s",
			    sgScriptInsnNames[si->type],
			    sgActionNames[si->si_action.type],
			    si->tgtName, abuf);
		}
		break;
	default:
		Snprintf(buf, len, "%s(%s)",
		    sgScriptInsnNames[si->type],
		    si->tgtName);
		break;
	}
}

/*
 * Execute a Create instruction. A new node is created, initialized from
 * the instruction's argument and attached to the parent node (tgtName).
 */
static int
ExecCreateInsn(SG_ScriptEditCtx *_Nonnull e, SG_ScriptRenderCtx *_Nullable re,
    SG_ScriptInsn *_Nonnull si)
{
	SG_View *sv = e->sv;
	AG_DataSource *ds;
	SG_Node *node, *parent;

	if (re != NULL && re->iInt > 0) {
		/* TODO: fade in */
		return (0);
	}

	if ((parent = GetInsnTarget(e->sg, si)) == NULL)
		return (-1);

	if ((ds = AG_OpenCore(si->si_create.data, si->si_create.size)) == NULL)
		return (-1);

	if ((node = TryMalloc(si->si_create.cls->size)) == NULL) {
		goto fail;
	}
	AG_ObjectInit(node, si->si_create.cls);
	OBJECT(node)->flags |= AG_OBJECT_NAME_ONATTACH;
	if (AG_ObjectUnserialize(node, ds) == -1) {
		AG_ObjectDestroy(node);
		goto fail;
	}
	AG_ObjectAttach(parent, node);
	
	/* Ignore any saved transformation matrix. */
	SG_Identity(node);

	if (parent != e->sg->root) {
		M_Vector3 vOffsZ = SG_NodeDir(sv->cam);
		M_VecScale3v(&vOffsZ, 0.01);
		SG_Translatev(node, vOffsZ);
	}
	AG_CloseCore(ds);
	return (0);
fail:
	AG_CloseCore(ds);
	return (-1);
}

/* Execute a Delete instruction. The specified node is destroyed. */
static int
ExecDeleteInsn(SG_ScriptEditCtx *_Nonnull e, SG_ScriptRenderCtx *_Nonnull re,
    SG_ScriptInsn *_Nonnull si)
{
	SG_Node *tgt;
	
	if (re != NULL &&
	    re->iInt == (re->nInt-1)) {
		/* TODO: fade out */
		return (0);
	}

	if ((tgt = GetInsnTarget(e->sg, si)) == NULL)
		return (-1);

	if (!TAILQ_EMPTY(&OBJECT(tgt)->children)) {
		AG_SetError("Cannot delete, node has child objects");
		return (-1);
	}
	AG_ObjectDetach(tgt);
	AG_ObjectDestroy(tgt);
	return (0);
}

/* Execute an Action instruction. */
static int
ExecActionInsn(SG_ScriptEditCtx *_Nonnull e, SG_ScriptRenderCtx *_Nonnull re,
    SG_ScriptInsn *_Nonnull si)
{
	SG_Node *tgt;

	if ((tgt = GetInsnTarget(e->sg, si)) == NULL) {
		return (-1);
	}
	if (re != NULL && re->nInt > 1) {		/* Interpolate */
		SG_Action *ao = &si->si_action;
		SG_Action a;

		SG_ActionInit(&a, ao->type);
		switch (a.type) {
		case SG_ACTION_MOVE:
		case SG_ACTION_ZMOVE:
			a.act_move = M_VecScale3(ao->act_move,
			    1.0/(M_Real)re->nInt);
			break;
		case SG_ACTION_ROTATE:
			a.act_rotate.theta = ao->act_rotate.theta /
			    (M_Real)re->nInt;
			a.act_rotate.axis = ao->act_rotate.axis;
			break;
		case SG_ACTION_SCALE:
			a.act_scale = M_VecScale3(ao->act_scale,
			    1.0/(M_Real)re->nInt);
			break;
		default:
			break;
		}
		return SGNODE_OPS(tgt)->script_action(tgt, &a, 0);
	} else {
		return SGNODE_OPS(tgt)->script_action(tgt, &si->si_action, 0);
	}
}

/*
 * Invert a Create instruction. Look up the created node under the
 * target parent and destroy it.
 */
static int
UndoCreateInsn(SG_ScriptEditCtx *_Nonnull e, SG_ScriptInsn *_Nonnull si)
{
	SG_Node *tgtParent, *tgtCreated;

	if ((tgtParent = GetInsnTarget(e->sg, si)) == NULL)
		return (-1);

	if ((tgtCreated = AG_ObjectFindChild(tgtParent, si->si_create.name))
	    == NULL) {
		return (-1);
	}
	AG_ObjectDetach(tgtCreated);
	AG_ObjectDestroy(tgtCreated);
	return (0);
}

/*
 * Invert a Delete instruction. We do this by executing the previous Create
 * instruction corresponding to the deleted node.
 */
static int
UndoDeleteInsn(SG_ScriptEditCtx *_Nonnull e, SG_ScriptInsn *_Nonnull si)
{
	SG_Script *scr = e->scr;
	char path[AG_OBJECT_PATH_MAX];
	SG_ScriptInsn *siCreate = NULL;
	int tCreate;

	for (tCreate = scr->t; tCreate >= 0; tCreate--) {
		SG_ScriptFrame *sf = &scr->frames[tCreate];

		TAILQ_FOREACH(siCreate, &sf->insns, insns) {
			if (siCreate->type != SG_INSN_CREATE) {
				continue;
			}
			Strlcpy(path, siCreate->tgtName, sizeof(path));
			Strlcat(path, "/", sizeof(path));
			Strlcat(path, siCreate->si_create.name, sizeof(path));
			if (strcmp(path, si->tgtName) == 0)
				break;
		}
		if (siCreate != NULL)
			break;
	}
	if (tCreate < 0) {
		AG_SetError("No matching Create insn for %s", si->tgtName);
		return (-1);
	}
	return ExecCreateInsn(e, NULL, siCreate);
}

/* Invert an Action instruction. We call script_action() with invert=1. */
static int
UndoActionInsn(SG_ScriptEditCtx *_Nonnull e, SG_ScriptInsn *_Nonnull si)
{
	SG_Node *tgt;

	if ((tgt = GetInsnTarget(e->sg, si)) == NULL) {
		return (-1);
	}
	return SGNODE_OPS(tgt)->script_action(tgt, &si->si_action, 1);
}

/* Execute a script instruction. */
static int
ExecInsn(SG_ScriptEditCtx *_Nonnull e, SG_ScriptRenderCtx *_Nullable re,
    SG_ScriptInsn *_Nonnull si)
{
#if 1
	char buf[128];
	SG_ScriptPrintInsn(si, buf, sizeof(buf));
	if (re != NULL && re->nInt > 1) {
		printf("Exec: %s (Interp: %d/%d)\n", buf, re->iInt, re->nInt);
	} else {
		printf("Exec: %s\n", buf);
	}
#endif
	switch (si->type) {
	case SG_INSN_CREATE:
		return ExecCreateInsn(e, re, si);
	case SG_INSN_DELETE:
		return ExecDeleteInsn(e, re, si);
	case SG_INSN_ACTION:
	case SG_INSN_CAMACTION:
		return ExecActionInsn(e, re, si);
	case SG_INSN_NOOP:
		break;
	default:
		AG_SetError("Illegal instruction: 0x%x", si->type);
		return (-1);
	}
	return (0);
}

/* Undo the action of a script instruction. */
static int
UndoInsn(SG_ScriptEditCtx *_Nonnull e, SG_ScriptInsn *_Nonnull si)
{
#ifdef AG_DEBUG
	char buf[128];
	SG_ScriptPrintInsn(si, buf, sizeof(buf));
	printf("Undo: %s\n", buf);
#endif
	switch (si->type) {
	case SG_INSN_CREATE:
		return UndoCreateInsn(e, si);
	case SG_INSN_DELETE:
		return UndoDeleteInsn(e, si);
	case SG_INSN_ACTION:
	case SG_INSN_CAMACTION:
		return UndoActionInsn(e, si);
	case SG_INSN_NOOP:
		break;
	default:
		AG_SetError("Illegal instruction: 0x%x", si->type);
		return (-1);
	}
	return (0);
}

static void
PollInsns(AG_Event *_Nonnull event)
{
	char text[1024];
	AG_Tlist *tl = AG_TLIST_SELF();
	SG_Script *scr = SG_SCRIPT_PTR(1);
	SG_ScriptFrame *sf = &scr->frames[scr->t];
	SG_ScriptInsn *si;

	AG_TlistClear(tl);
	AG_ObjectLock(scr);

	TAILQ_FOREACH(si, &sf->insns, insns) {
		AG_TlistItem *it;

		SG_ScriptPrintInsn(si, text, sizeof(text));
		it = AG_TlistAddS(tl, sgIconNode.s, text);
		it->cat = "insn";
		it->p1 = si;
	}

	AG_ObjectUnlock(scr);
	AG_TlistRestore(tl);
}

static void
SelectInsn(AG_Event *_Nonnull event)
{
#ifdef AG_THREADS
	SG_Script *scr = SG_SCRIPT_PTR(1);
#endif
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	const int state = AG_INT(3);

	if (strcmp(it->cat, "insn") == 0) {
		SG_ScriptInsn *si = it->p1;

		AG_ObjectLock(scr);
		AG_SETFLAGS(si->flags, SG_SCRIPT_INSN_SELECTED, state);
		AG_ObjectUnlock(scr);
	}
}

static void
AutosizeEditPane(SG_ScriptEditCtx *_Nonnull e)
{
	AG_SizeReq rDiv;

	AG_WidgetSizeReq(e->paLeft->div[1], &rDiv);
	AG_PaneMoveDivider(e->paLeft, WIDGET(e->paLeft)->h - rDiv.h);
}

static void
ClearEditPane(SG_ScriptEditCtx *_Nonnull e)
{
	if (e->wEdit != NULL) {
		AG_ObjectDetach(e->wEdit);
		e->wEdit = NULL;
	}
	if (e->boxBtns != NULL) {
		AG_ObjectDetach(e->boxBtns);
		e->boxBtns = NULL;
	}
}

/*
 * Insert a new "Create" instruction from an active node object. The
 * Create instruction will contains the node's serialized dataset.
 */
static void
InsertCreateInsn(AG_Event *_Nonnull event)
{
	char path[AG_OBJECT_PATH_MAX];
	SG_ScriptEditCtx *e = AG_PTR(1);
	SG_Node *parent = SG_NODE_PTR(2);
	SG_Node *node = SG_NODE_PTR(3);
	SG_Script *scr = e->scr;
	SG_ScriptInsn *si = NULL, *siRef;
	AG_DataSource *ds;
	AG_CoreSource *cs;
	int selOrig;

	if ((ds = AG_OpenAutoCore()) == NULL) {
		return;
	}
	cs = AG_CORE_SOURCE(ds);

	selOrig = (node->flags & SG_NODE_SELECTED);
	node->flags &= ~(SG_NODE_SELECTED);
	if (AG_ObjectSerialize(node, ds) == -1) {
		goto fail;
	}
	node->flags |= selOrig;

	if ((si = SG_ScriptInsnNew(scr)) == NULL)
		goto fail;

	si->type = SG_INSN_CREATE;
	if ((si->tgtName = AG_ObjectGetName(parent)) == NULL)
		goto fail;

	if ((si->si_create.name = TryStrdup(OBJECT(node)->name)) == NULL)
		goto fail;
	if ((si->si_create.data = TryMalloc(cs->size)) == NULL) {
		goto fail;
	}
	memcpy(si->si_create.data, cs->data, cs->size);
	si->si_create.size = cs->size;
	si->si_create.cls = OBJECT_CLASS(node);

	/*
	 * Insert Create insn before any other newly created instruction
	 * referencing created object.
	 */
	AG_ObjectCopyName(node, path, sizeof(path));
	TAILQ_FOREACH(siRef, &scr->frames[scr->t].insns, insns) {
		if (siRef->tgtName != NULL &&
		    strcmp(siRef->tgtName, path) == 0)
			break;
	}
	if (siRef != NULL) {
		SG_ScriptAddInsnBefore(scr, scr->t, siRef, si);
	} else {
		SG_ScriptAddInsn(scr, scr->t, si);
	}

	AG_LabelText(e->stat, _("Created %s object (%u bytes)"),
	    OBJECT(node)->name, (Uint)cs->size);

	ClearEditPane(e);
	AG_CloseAutoCore(ds);
	return;
fail:
	if (si != NULL) { SG_ScriptInsnFree(si); }
	AG_CloseAutoCore(ds);
	AG_TextMsgFromError();
	return;
}

static void
CancelCreateInsn(AG_Event *_Nonnull event)
{
	SG_ScriptEditCtx *e = AG_PTR(1);
	SG_Node *node = SG_NODE_PTR(2);
	SG_Script *scr = e->scr;
	SG_ScriptInsn *siRef, *siRefNext;
	SG_ScriptFrame *sf;
	char *nodeName;

	ClearEditPane(e);
	
	AG_LabelText(e->stat, _("Aborted creation of %s"), OBJECT(node)->name);
	
	/*
	 * Remove other newly created insns referencing the new object.
	 */
	nodeName = AG_ObjectGetName(node);
	sf = &scr->frames[scr->t];
	for (siRef = TAILQ_FIRST(&sf->insns);
	     siRef != TAILQ_END(&sf->insns);
	     siRef = siRefNext) {
		siRefNext = TAILQ_NEXT(siRef, insns);

		if (siRef->tgtName != NULL &&
		    strcmp(siRef->tgtName, nodeName) == 0) {
			SG_ScriptDelInsn(scr, scr->t, siRef);
			SG_ScriptInsnFree(siRef);
		}
	}

	e->siNew = NULL;

	AG_ObjectDetach(node);
	AG_ObjectDestroy(node);
}

/* Insert a new "Delete" instruction to delete the selected node(s). */
static void
InsertDeleteInsn(AG_Event *_Nonnull event)
{
	SG_ScriptEditCtx *e = AG_PTR(1);
	SG_Script *scr = e->scr;
	SG *sg = e->sg;
	SG_Node *node;
	int nDeleted = 0;

	SG_FOREACH_NODE(node, sg) {
		SG_ScriptInsn *si;

		if ((node->flags & SG_NODE_SELECTED) == 0) {
			continue;
		}
		if (!TAILQ_EMPTY(&OBJECT(node)->children)) {
			AG_LabelText(e->stat,
			    _("Cannot Delete: %s has child objects"),
			    OBJECT(node)->name);
			return;
		}
		if ((si = SG_ScriptInsnNew(scr)) == NULL) {
			continue;
		}
		si->type = SG_INSN_DELETE;
		if ((si->tgtName = AG_ObjectGetName(node)) == NULL) {
			SG_ScriptInsnFree(si);
			continue;
		}
		SG_ScriptAddInsn(scr, scr->t, si);

		AG_ObjectDetach(node);
		AG_ObjectDestroy(node);

		nDeleted++;
	}
	AG_LabelText(e->stat, _("Deleted %d node(s)"), nDeleted);
}

static void
InsertCreateInsnDlg(AG_Event *_Nonnull event)
{
	SG_ScriptEditCtx *e = AG_PTR(1);
	AG_ObjectClass *cls = AG_PTR(2);
	SG *sg = e->sg;
	AG_Widget *w;
	SG_Node *node, *parent = NULL;

	/* Use any currently selected node as parent. */
	SG_FOREACH_NODE(node, sg) {
		if (node->flags & SG_NODE_SELECTED) {
			parent = node;
			break;
		}
	}
	if (parent == NULL)
		parent = sg->root;

	/* Create a new node instance. */
	if ((node = Malloc(cls->size)) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	AG_ObjectInit(node, cls);
	OBJECT(node)->flags |= AG_OBJECT_NAME_ONATTACH;
	AG_ObjectAttach(parent, node);
	
	if (parent != sg->root) {
		M_Vector3 vOffsZ = SG_NodeDir(e->sv->cam);
		M_VecScale3v(&vOffsZ, 0.01);
		SG_Translatev(node, vOffsZ);
	}

	/* Edit node parameters. */
	ClearEditPane(e);
	e->wEdit = (AG_Widget *)AG_BoxNewVert(e->paLeft->div[1], AG_BOX_EXPAND);
	if (NODE_OPS(node)->edit != NULL) {
		w = NODE_OPS(node)->edit(node, e->sv);
		if (w != NULL && AG_OfClass(w, "AG_Widget:*"))
			AG_ObjectAttach(e->wEdit, w);
	}
	e->boxBtns = AG_BoxNewHoriz(e->paLeft->div[1], AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	{
		AG_ButtonNewFn(e->boxBtns, 0, _("Save"),
		    InsertCreateInsn, "%p,%p,%p", e, parent, node);
		AG_ButtonNewFn(e->boxBtns, 0, _("Cancel"),
		    CancelCreateInsn, "%p,%p", e, node);
	}

	AutosizeEditPane(e);
}

static void
EditInsn(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	SG_ScriptEditCtx *e = AG_PTR(1);
	AG_TlistItem *it = AG_TlistSelectedItem(tl);

	if (strcmp(it->cat, "insn") != 0)
		return;
#if 0
	SG_GUI_EditNode(node, paEdit->div[1], sv);
#endif
	AutosizeEditPane(e);
}

static void
AllocFrames(AG_Event *_Nonnull event)
{
	SG_Script *scr = SG_SCRIPT_PTR(1);
	AG_Numerical *numAlloc = AG_NUMERICAL_PTR(2);

	if (SG_ScriptAlloc(scr, scr->n+AG_GetInt(numAlloc,"value")) == -1)
		AG_TextMsgFromError();
}

/* Pick nearest point on nearest node. */
static SG_Node *_Nullable
PickNearestNode(SG_ScriptEditCtx *_Nonnull e, int xWid, int yWid,
    M_Vector3 *_Nonnull Xnear)
{
	M_Real dNearest = M_INFINITY;
	SG_Node *node, *nodeNearest = NULL;
	M_Vector4 pNear, pFar;
	M_Line3 ray;
	M_Matrix44 T;

	/* Find corresponding points on Near and Far plane. */
	SG_ViewUnProject(e->sv, xWid, yWid, &pNear, &pFar);
	SG_GetNodeTransform(e->sv->cam, &T);
	M_MatMultVector44v(&pNear, &T);
	M_MatMultVector44v(&pFar, &T);
	
	/* Intersect ray with nodes in the scene. */
	SG_FOREACH_NODE(node, e->sg) {
		M_Vector3 pRayNear, pRayFar;
		M_GeomSet3 S = M_GEOM_SET_EMPTY;
		M_Matrix44 T;
		M_Geom3 gt;
		M_Real d;
		M_Vector3 lp1, lp2;
		
		SG_GetNodeTransformInverse(node, &T);
		pRayNear = M_VecFromProj3(M_MatMultVector44(T,pNear));
		pRayFar = M_VecFromProj3(M_MatMultVector44(T,pFar));
		ray = M_LineFromPts3(pRayNear, pRayFar);
		gt.type = M_LINE;
		gt.g.line = ray;
		
		if (SG_Intersect(node, gt, &S) != 1) {
			continue;
		}
		switch (S.g[0].type) {
		case M_POINT:
			d = M_VecDistance3(S.g[0].g.point, pRayNear);
			if (d < dNearest) {
				dNearest = d;
				nodeNearest = node;
				*Xnear = S.g[0].g.point;
			}
			break;
		case M_LINE:
			M_LineToPts3(S.g[0].g.line, &lp1, &lp2);
			d = M_VecDistance3(lp1, pRayNear);
			if (d < dNearest) {
				dNearest = d;
				nodeNearest = node;
				*Xnear = lp1;
			}
			d = M_VecDistance3(lp2, pRayNear);
			if (d < dNearest) {
				dNearest = d;
				nodeNearest = node;
				*Xnear = lp2;
			}
			break;
		default:
			break;
		}
		M_GeomSetFree3(&S);
	}
	return (nodeNearest);
}

/* Pick a node, initiate an Action and create a corresponding Insn. */
static int
BeginAction(SG_ScriptEditCtx *_Nonnull e, int xWid, int yWid)
{
	SG_View *sv = e->sv;
	const int *kbdState = AG_GetKeyState(sv);
	const Uint keyModState = AG_GetModState(sv);
	SG_Script *scr = e->scr;
	SG_Node *node;
	M_Vector3 Xnear = M_VecZero3();
	SG_ScriptInsn *si = NULL;
	SG_Action *actReg, *act;
	M_Vector4 pNear, pFar;
	M_Matrix44 T;

	/* Select the nearest intersecting node. */
	SG_FOREACH_NODE(node, e->sg) {
		node->flags &= ~(SG_NODE_SELECTED);
	}
	if ((node = PickNearestNode(e, xWid, yWid, &Xnear)) == NULL) {
		return (0);
	}
	node->flags |= SG_NODE_SELECTED;

	/* Look for a matching action key shortcut (default to Move). */
	TAILQ_FOREACH(actReg, &node->actions, actions) {
		if (kbdState[actReg->key] &&
		    (actReg->keyMod == 0 || (keyModState & actReg->keyMod)))
			break;
	}
	if (actReg == NULL) {
		TAILQ_FOREACH(actReg, &node->actions, actions) {
			if (actReg->type == SG_ACTION_MOVE)
				break;
		}
		if (actReg == NULL)
			return (0);
	}

	/* Compute the mouse control ray. */
	SG_ViewUnProject(sv, xWid, yWid, &pNear, &pFar);
	SG_GetNodeTransform(sv->cam, &T);
	M_MatMultVector44v(&pNear, &T);
	M_MatMultVector44v(&pFar, &T);

	/* Bring the ray into the node's coordinate system. */
	SG_GetNodeTransformInverse(node, &T);

	/* Create a new Action instruction. */
	if ((si = SG_ScriptInsnNew(scr)) == NULL) {
		goto fail;
	}
	si->type = SG_INSN_ACTION;
	if ((si->tgtName = AG_ObjectGetName(node)) == NULL) {
		goto fail;
	}
	act = &si->si_action;
	SG_ActionInit(act, actReg->type);
	act->Rorig = M_LineFromPts3(M_VecFromProj3(M_MatMultVector44(T,pNear)),
	                            M_VecFromProj3(M_MatMultVector44(T,pFar)));
	act->Rcur = act->Rorig;
	act->vOrig = Xnear;
	act->Torig = node->T;
	SG_ScriptAddInsn(scr, scr->t, si);
	
	/* Invoke the SG_ACTION_*_BEGIN routine. */
	SGNODE_OPS(node)->editor_action(node, act->type+1, act);

	AG_LabelText(e->stat, _("Selected %s for %s"),
	    OBJECT(node)->name, sgActionNames[act->type]);

	e->siNew = si;
	return (1);
fail:
	if (si != NULL) { SG_ScriptInsnFree(si); }
	AG_LabelText(e->stat, _("Failed: %s"), AG_GetError());
	return (0);
}

/* Update the current action by mouse gesture. */
static void
UpdateAction(SG_ScriptEditCtx *_Nonnull e, int xWid, int yWid)
{
	M_Vector4 pNear, pFar;
	M_Matrix44 T, Tsave;
	SG_Node *node;
	SG_Action *act;

	if (e->siNew == NULL) {
		return;
	}
	act = &e->siNew->si_action;

	if ((node = GetInsnTarget(e->sg, e->siNew)) == NULL) {
		AG_LabelText(e->stat, _("Failed: %s"), AG_GetError());
		return;
	}

	/* Compute the mouse control ray. */
	SG_ViewUnProject(e->sv, xWid, yWid, &pNear, &pFar);
	SG_GetNodeTransform(e->sv->cam, &T);
	M_MatMultVector44v(&pNear, &T);
	M_MatMultVector44v(&pFar, &T);

	/*
	 * Bring the control ray into the node's coordinate system,
	 * as it was when the action was first initiated (using Torig).
	 * This enables transformations such as rotations to be done
	 * in a numerically stable manner.
	 */
	Tsave = node->T;
	node->T = act->Torig;
	SG_GetNodeTransformInverse(node, &T);
	act->Rcur = M_LineFromPts3(M_VecFromProj3(M_MatMultVector44(T,pNear)),
	                           M_VecFromProj3(M_MatMultVector44(T,pFar)));
	node->T = Tsave;

	/* Invoke the node's editor_action routine. */
	SGNODE_OPS(node)->editor_action(node, act->type, act);
}

/* Terminate any current action. */
static void
EndAction(SG_ScriptEditCtx *_Nonnull e)
{
	SG_ScriptInsn *si = e->siNew;
	SG_Action *act = &si->si_action;
	SG_Node *tgt;

	if ((tgt = GetInsnTarget(e->sg, si)) == NULL)
		return;

	/* Invoke SG_ACTION_*_END routine. */
	SGNODE_OPS(tgt)->editor_action(tgt, act->type+2, act);

	/* Cancel any no-op instructions. */
	switch (act->type) {
	case SG_ACTION_MOVE:
	case SG_ACTION_ZMOVE:
		if (act->act_move.x == 0 &&
		    act->act_move.y == 0 &&
		    act->act_move.z == 0) {
			SG_ScriptDelInsn(e->scr, e->scr->t, si);
			SG_ScriptInsnFree(si);
		}
		break;
	case SG_ACTION_ROTATE:
		if (act->act_rotate.theta == 0) {
			SG_ScriptDelInsn(e->scr, e->scr->t, si);
			SG_ScriptInsnFree(si);
		}
		break;
	default:
		break;
	}

	e->siNew = NULL;
}

static Uint32
CamMoveTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	SG_View *sv = SG_VIEW_SELF();
	SG_ScriptEditCtx *e = AG_PTR(1);
	
	SG_Translatev(sv->cam, e->vCamMove);
	e->vCamMoveSum = M_VecAdd3(e->vCamMoveSum, e->vCamMove);

	AG_LabelText(e->stat, _("%s: +[%f,%f,%f]"), OBJECT(sv->cam)->name,
	    e->vCamMoveSum.x, e->vCamMoveSum.y, e->vCamMoveSum.z);
	return (to->ival);
}

static void
BeginCameraMove(SG_ScriptEditCtx *_Nonnull e)
{
	e->camMoving = 1;
	e->vCamMove = M_VecZero3();
	e->vCamMoveSum = M_VecZero3();
	AG_AddTimer(e->sv, &e->toCamMove, 1, CamMoveTimeout, "%p", e);
}

static void
EndCameraMove(SG_ScriptEditCtx *_Nonnull e)
{
	SG_View *sv = e->sv;

	AG_DelTimer(sv, &e->toCamMove);
	e->camMoving = 0;
	
	/*
	 * If requested, generate a new Camera Action instruction.
	 */
	if (AG_GetModState(sv) & AG_KEYMOD_CTRL) {
		SG_Script *scr = e->scr;
		SG_ScriptInsn *si;

		if ((si = SG_ScriptInsnNew(scr)) == NULL) {
			goto fail;
		}
		si->type = SG_INSN_CAMACTION;
		if ((si->tgtName = AG_ObjectGetName(sv->cam)) == NULL) {
			goto fail;
		}
		SG_ActionInit(&si->si_action, SG_ACTION_MOVE);
		SG_ScriptAddInsn(scr, scr->t, si);
		si->si_action.act_move = e->vCamMoveSum;
	}
	return;
fail:
	AG_LabelText(e->stat, _("Failed: %s"), AG_GetError());
}

static void
SeekToFrame(SG_ScriptEditCtx *_Nonnull e, int t)
{
	SG_Script *scr = e->scr;
	int i;

	scr->t = t;

	if (scr->t > scr->tPrev) {
		for (i = scr->tPrev+1; i <= scr->t; i++) {
			SG_ScriptFrame *sf = &scr->frames[i];
			SG_ScriptInsn *si;
		
			printf("Forward seek: #%d\n", i);
			TAILQ_FOREACH(si, &sf->insns, insns) {
				if (ExecInsn(e, NULL, si) == -1)
					goto fail;
			}
		}
	} else if (scr->t < scr->tPrev) {
		for (i = scr->tPrev; i > scr->t; i--) {
			SG_ScriptFrame *sf = &scr->frames[i];
			SG_ScriptInsn *si;
		
			printf("Backward seek: #%d\n", scr->t);
			TAILQ_FOREACH_REVERSE(si, &sf->insns, sg_script_insnsq,
			    insns) {
				if (UndoInsn(e, si) == -1)
					goto fail;
			}
		}
	}

	scr->tPrev = scr->t;
	return;
fail:
	AG_LabelText(e->stat, _("Seek failed: %s"), AG_GetError());
	scr->t = scr->tPrev;
}

static AG_Surface *_Nullable
GenerateUnderlay(SG_ScriptEditCtx *_Nonnull e)
{
	AG_Rect2 r = WIDGET(e->sv)->rView;
	Uint8 *buf = NULL;
	AG_Surface *S;
	
	if ((buf = TryMalloc((r.w*r.h) << 2)) == NULL)
		return (NULL);

	glReadPixels(r.x1, WIDGET(e->win)->h - r.y2,
		     r.w, r.h,
		     GL_RGBA, GL_UNSIGNED_BYTE, buf);
	AG_PackedPixelFlip(buf, r.h, r.w << 2);

	S = AG_SurfaceFromPixelsRGBA(buf, r.w, r.h, 32,
	    0x000000ff,
	    0x0000ff00,
	    0x00ff0000, 0);

	free(buf);
	return (S);
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	SG_ScriptEditCtx *e = AG_PTR(1);
	const int key = AG_INT(2);
	AG_Event ev;

	switch (key) {
	case AG_KEY_DELETE:
		AG_EventInit(&ev);
		AG_EventPushPointer(&ev, "", e);
		InsertDeleteInsn(&ev);
		break;
	case AG_KEY_SPACE:
		if ((e->scr->t+1) < e->scr->n) {
			if (e->suUnder != NULL) {
				AG_SurfaceFree(e->suUnder);
			}
			e->suUnder = GenerateUnderlay(e);
			SeekToFrame(e, e->scr->t+1);
		}
		break;
	}
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	SG_ScriptEditCtx *e = AG_PTR(1);
	SG_View *sv = e->sv;
	const int button = AG_INT(2);
	const int x = AG_INT(3);
	const int y = AG_INT(4);
	
	if (sv->cam == NULL)
		return;

	AG_WidgetFocus(sv);
	switch (button) {
	case AG_MOUSE_LEFT:
		BeginAction(e, x, y);
		break;
	case AG_MOUSE_MIDDLE:
		BeginCameraMove(e);
		break;
	case AG_MOUSE_WHEELUP:
		if (e->camMoving) {
			e->vCamMove.z -= 0.01;
		} else {
			SG_Translate(sv->cam, 0.0, 0.0, -0.01);
		}
		break;
	case AG_MOUSE_WHEELDOWN:
		if (e->camMoving) {
			e->vCamMove.z += 0.01;
		} else {
			SG_Translate(sv->cam, 0.0, 0.0, +0.01);
		}
		break;
	case AG_MOUSE_RIGHT:
		if (AG_GetModState(sv) & AG_KEYMOD_CTRL) {
			sv->flags |= SG_VIEW_ROTATING;
		} else {
			sv->flags |= SG_VIEW_PANNING;
		}
		break;
	}
	AG_Redraw(sv);
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	SG_ScriptEditCtx *e = AG_PTR(1);
	const int button = AG_INT(2);

	switch (button) {
	case AG_MOUSE_LEFT:
		if (e->siNew != NULL && e->siNew->type == SG_INSN_ACTION) {
			EndAction(e);
			AG_Redraw(e->sv);
		}
		break;
	case AG_MOUSE_RIGHT:
		e->sv->flags &= ~(SG_VIEW_PANNING|SG_VIEW_ROTATING);
		break;
	case AG_MOUSE_MIDDLE:
		EndCameraMove(e);
		break;
	}
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	SG_ScriptEditCtx *e = AG_PTR(1);
	const int xWid = AG_INT(2);
	const int yWid = AG_INT(3);
	const int xRel = AG_INT(4);
	const int yRel = AG_INT(5);
	SG_View *sv = e->sv;

	if (e->siNew != NULL && e->siNew->type == SG_INSN_ACTION) {
		UpdateAction(e, xWid, yWid);
		AG_Redraw(e->sv);
	} else {
		if (sv->flags & SG_VIEW_PANNING) {
			SG_CameraMoveMouse(sv->cam, sv, xRel, yRel, 0);
		} else if (sv->flags & SG_VIEW_ROTATING) {
			SG_CameraRotMouse(sv->cam, sv, xRel, yRel);
		}
	}
}

static void
OnUnderlay(AG_Event *_Nonnull event)
{
	SG_ScriptEditCtx *e = AG_PTR(1);
	SG_View *sv = e->sv;
	AG_Rect r;
	AG_Color c;

	if (e->suUnder == NULL)
		return;
	
	AG_WidgetBlit(sv, e->suUnder, 0, 0);
	r.x = 0;
	r.y = 0;
	r.w = WIDTH(sv);
	r.h = HEIGHT(sv);
	AG_ColorRGBA_8(&c, 0,0,0, 128);
	AG_DrawRectBlended(sv, &r, &c, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
}

/* Reset generated scene and seek to first frame. */
static void
ResetScene(SG_ScriptEditCtx *_Nonnull e, SG_ScriptRenderCtx *_Nullable re)
{
	SG_Script *scr = e->scr;
	SG_ScriptInsn *si;

	SG_Clear(e->sg);
	
	scr->t = 0;
	scr->tPrev = 0;

	if (scr->n > 0) {
		TAILQ_FOREACH(si, &scr->frames[0].insns, insns) {
			if (ExecInsn(e, re, si) == -1)
				break;
		}
	}
}

/* Render the scripted scene to disk. */
static void
Render(AG_Event *_Nonnull event)
{
	char path[AG_PATHNAME_MAX];
	SG_ScriptEditCtx *e = AG_PTR(1);
	SG_ScriptRenderCtx *re = AG_PTR(2);
	AG_Window *winDlg = AG_WINDOW_PTR(3);
	AG_DirDlg *dirDlg = AG_DIRDLG_PTR(4);
	char *outDir = Strdup(dirDlg->cwd);
	SG_Script *scr = e->scr;
	AG_Driver *drv = WIDGET(e->sv)->drv;
	AG_Rect2 r = WIDGET(e->sv)->rView;
	Uint8 *buf = NULL;
	Uint32 ticks;
	Uint nOutFrames = 0;
	AG_Dir *dir;
	int i;

	if ((dir = AG_OpenDir(outDir)) == NULL) {
		if (AG_MkPath(outDir) == -1) {
			AG_TextMsgFromError();
			return;
		}
	} else {
		if (re->clearDir) {		/* Delete existing frames */
			for (i = 0; i < dir->nents; i++) {
				char *dent = dir->ents[i], *c;

				if (dent[0] == '.') {
					continue;
				}
				for (c = &dent[0]; *c != '.' && *c != '\0'; c++) {
					if (!isdigit(*c))
						break;
				}
				if (*c == '.' || *c == '\0') {
					if (*c == '.') {
						if (Strcasecmp(c, ".jpg") != 0 &&
						    Strcasecmp(c, ".jpeg") != 0 &&
						    Strcasecmp(c, ".png") != 0) {
							continue;
						}
					}
					Strlcpy(path, outDir, sizeof(path));
					Strlcat(path, AG_PATHSEP, sizeof(path));
					Strlcat(path, dent, sizeof(path));
					if (AG_FileDelete(path) == -1) {
						fprintf(stderr, "%s: %s\n", path,
						    AG_GetError());
					}
				}
			}
		}
		AG_CloseDir(dir);
	}

	if (re->nFirst >= re->nLast) {
		AG_SetError("Invalid range");
		goto fail;
	}

	switch (re->interp) {
	case SG_SCRIPT_INTERP_LINEAR:
		re->nInt = re->timeScale * re->fps / (re->nLast - re->nFirst);
		if (re->nInt < 1) { re->nInt = 1; }
		break;
	default:
		re->nInt = 1;
		break;
	}

	ClearEditPane(e);
	AG_ObjectDetach(winDlg);
	
	SG_Clear(e->sg);

	if ((buf = TryMalloc((r.w*r.h) << 2)) == NULL)
		goto fail;
	
	printf("Rendering %s to %s (@ %f fps)\n",
	    OBJECT(scr)->name, outDir, re->fps);

	scr->tPrev = re->nFirst - 1;
	ticks = AG_GetTicks();
	for (scr->t = re->nFirst;
	     scr->t < re->nLast;
	     scr->t++) {
		SG_ScriptFrame *sf = &scr->frames[scr->t];
		SG_ScriptInsn *si;
		AG_Surface *S;
		
		fprintf(stderr, "Rendering #%d (", scr->t);
		
		for (re->iInt = 0;
		     re->iInt < re->nInt;
		     re->iInt++) {
			fprintf(stderr, "%u ", nOutFrames);
			TAILQ_FOREACH(si, &sf->insns, insns) {
				if (ExecInsn(e, re, si) == -1) {
					fprintf(stderr, "Insn[%d/%d]: %s; ignoring\n",
					    re->iInt, re->nInt, AG_GetError());
/*					goto fail; */
				}
			}

			Snprintf(path, sizeof(path), "%s%s%08u.jpg",
			    outDir, AG_PATHSEP, nOutFrames);

			/* TODO: render offscreen */
			AG_BeginRendering(drv);
			AG_ObjectLock(e->win);
			AG_WindowDraw(e->win);
			AG_ObjectUnlock(e->win);
			AG_EndRendering(drv);
	
			glReadPixels(r.x1, WIDGET(e->win)->h - r.y2,
				     r.w, r.h,
				     GL_RGBA, GL_UNSIGNED_BYTE, buf);
			AG_PackedPixelFlip(buf, r.h, r.w << 2);

			S = AG_SurfaceFromPixelsRGBA(buf, r.w, r.h, 32,
			    0x000000ff,
			    0x0000ff00,
			    0x00ff0000, 0);
			if (S == NULL)
				goto fail;

			if (AG_SurfaceExportJPEG(S, path, 100,
			    AG_EXPORT_JPEG_JDCT_ISLOW) == -1) {
				AG_SurfaceFree(S);
				goto fail;
			}
			AG_SurfaceFree(S);

			nOutFrames++;
		}
		fprintf(stderr, ")\n");
	}
	AG_LabelText(e->stat, _("Rendering Complete: %u frames in %s (%.01fs elapsed)"),
	    nOutFrames, outDir,
	    (float)(AG_GetTicks() - ticks)/1000);

	ResetScene(e, NULL);

	Free(buf);
	Free(outDir);
	return;
fail:
	AG_LabelText(e->stat, _("Render Failed: %s"), AG_GetError());
	AG_TextMsgFromError();
	Free(buf);
	Free(outDir);
}

static void
RenderDlg(AG_Event *_Nonnull event)
{
	SG_ScriptEditCtx *e = AG_PTR(1);
	SG_ScriptRenderCtx *re;
	SG_Script *scr = e->scr;
	AG_Window *win;
	AG_DirDlg *dd;
	AG_Box *hBox;
	AG_Numerical *num;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Script Rendering: %s"), OBJECT(e->scr)->name);
	
	re = Malloc(sizeof(SG_ScriptRenderCtx));
	re->interp = SG_SCRIPT_INTERP_NONE;
	re->nInt = 0;
	re->iInt = 0;
	re->timeScale = (scr->tLast - scr->tFirst)/5;
	re->fps = 30.0;
	re->clearDir = 1;

	re->nFirst = scr->tFirst;
	for (re->nLast = scr->tLast; re->nLast >= 0; re->nLast--) {
		if (!TAILQ_EMPTY(&scr->frames[re->nLast].insns))
			break;
	}
	if (re->nLast < (scr->tLast-1)) { re->nLast++; }

	/* Range */
	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		AG_LabelNewS(hBox, 0, _("Render #"));
		num = AG_NumericalNewInt(hBox, 0, NULL, NULL, &re->nFirst);
		AG_NumericalSizeHint(num, "<0>");
		
		AG_LabelNewS(hBox, 0, _("to #"));
		num = AG_NumericalNewInt(hBox, 0, NULL, NULL, &re->nLast);
		AG_NumericalSizeHint(num, "<0>");
	}

	/* Time scale */
	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		AG_LabelNewS(hBox, 0, _("Scaled to: "));

		num = AG_NumericalNewS(hBox, 0, "sec", NULL);
		AG_BindDouble(num, "value", &re->timeScale);
		AG_SetDouble(num, "min", 1.0);
		AG_NumericalSetPrecision(num, "f", 1);
		AG_NumericalSizeHint(num, "<10.00>");

		AG_LabelNewS(hBox, 0, _("at "));

		num = AG_NumericalNewS(hBox, 0, NULL, NULL);
		AG_BindDouble(num, "value", &re->fps);
		AG_SetDouble(num, "min", 1.0);
		AG_NumericalSetPrecision(num, "f", 1);
		AG_NumericalSizeHint(num, "<00>");
		
		AG_LabelNewS(hBox, 0, _("fps"));
	}
	/* Interpolation mode */
	{
		const char *items[] = {
			N_("No Interpolation"),
			N_("Linear Interpolation"),
			NULL
		};
		AG_RadioNewUint(win, 0, items, (Uint *)&re->interp);
	}

	AG_SeparatorNewHoriz(win);

	AG_LabelNewS(win, 0, _("Output Directory:"));
	dd = AG_DirDlgNewMRU(win, "sg-renderout",
	    AG_DIRDLG_EXPAND | AG_DIRDLG_SAVE | AG_DIRDLG_NOBUTTONS);
/*	AG_DirDlgOkAction(dd, Render, "%p,%p", e, win); */

	AG_CheckboxNewInt(win, 0, _("Delete Existing Frames"), &re->clearDir);

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Render"),
	    Render, "%p,%p,%p,%p", e, re, win, dd);

	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 22, 30);
	AG_WindowShow(win);
}

static void
SliderChanged(AG_Event *_Nonnull event)
{
	SG_ScriptEditCtx *e = AG_PTR(1);

	if (e->suUnder != NULL) {
		AG_SurfaceFree(e->suUnder);
		e->suUnder = NULL;
	}
	SeekToFrame(e, e->scr->t);
}

static void *_Nullable
Edit(void *_Nonnull obj)
{
	SG_ScriptEditCtx *e;
	SG_Script *scr = obj;
	AG_Mutex *lock = &OBJECT(scr)->lock;
	AG_Window *win;
	AG_MenuItem *m;
	AG_Label *lbl;
	AG_Box *hBox, *statBox;
	SG_View *sv;

	if ((e = AG_TryMalloc(sizeof(SG_ScriptEditCtx))) == NULL) {
		return (NULL);
	}
	e->boxBtns = NULL;
	e->wEdit = NULL;
	e->siNew = NULL;
	e->suUnder = NULL;
	AG_InitTimer(&e->toCamMove, "cam-move", 0);

	if ((win = e->win = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, OBJECT(scr)->name);
	AG_SetStyle(win, "spacing", "0");
	AG_SetStyleF(win, "padding", "0 %d %d %d",
	    WIDGET(win)->paddingRight,
	    WIDGET(win)->paddingBottom,
	    WIDGET(win)->paddingLeft);

	e->scr = scr;
	e->sg = SG_New(&sgVfsRoot, NULL, 0);
	e->sv = sv = SG_ViewNew(NULL, e->sg, SG_VIEW_EXPAND);
	AG_SetEvent(sv, "key-down", KeyDown, "%p", e);
/*	AG_SetEvent(sv, "key-up", KeyUp, "%p", e); */
	AG_SetEvent(sv, "mouse-button-down", MouseButtonDown, "%p", e);
	AG_SetEvent(sv, "mouse-button-up", MouseButtonUp, "%p", e);
	AG_SetEvent(sv, "mouse-motion", MouseMotion, "%p", e);
	AG_SetEvent(sv, "widget-underlay", OnUnderlay, "%p", e);

	e->menu = AG_MenuNew(win, AG_MENU_HFILL);
	m = AG_MenuNode(e->menu->root, _("File"), NULL);
	{
		AG_MenuAction(m, _("Render to disk..."), agIconSave.s,
		    RenderDlg, "%p", e);
		AG_MenuSeparator(m);
		SG_FileMenu(m, scr, win);
	}
	m = AG_MenuNode(e->menu->root, _("Edit"), NULL);
	{
		SG_EditMenu(m, scr, win);
	}
	m = AG_MenuNode(e->menu->root, _("View"), NULL);
	{
		SG_ViewMenu(m, e->sg, win, sv);
	}
	
	e->paHoriz = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND);
	{
		AG_Tlist *tl;
		AG_Toolbar *tbInsns;

		e->paLeft = AG_PaneNew(e->paHoriz->div[0], AG_PANE_VERT,
		    AG_PANE_EXPAND | AG_PANE_DIV1FILL);
	
		lbl = AG_LabelNewPolled(e->paLeft->div[0], 0,
		    _("Instructions at f%u: "), &scr->t);
		AG_LabelSizeHint(lbl, 1,
		    _("Instructions at f0000: "));

		tl = AG_TlistNew(e->paLeft->div[0], AG_TLIST_POLL |
		                                    AG_TLIST_EXPAND |
		                                    AG_TLIST_MULTI);
		WIDGET(tl)->flags &= ~(AG_WIDGET_FOCUSABLE);
		AG_TlistSizeHint(tl, "<Isocahedron>", 2);

		AG_SetEvent(tl, "tlist-poll", PollInsns, "%p", scr);
		AG_SetEvent(tl, "tlist-changed", SelectInsn, "%p", scr);
		AG_SetEvent(tl, "tlist-dblclick", EditInsn, "%p", e);
	
		/* Action buttons */
		AG_LabelNew(e->paLeft->div[0], 0, _("Insert Instruction: "));
		tbInsns = AG_ToolbarNew(e->paLeft->div[0],
		    AG_TOOLBAR_VERT, 2, AG_TOOLBAR_HFILL);
		{
			AG_ToolbarButton(tbInsns, _("Create(Image)"), 0,
			    InsertCreateInsnDlg, "%p,%p", e, &sgImageClass);
			AG_ToolbarButton(tbInsns, _("Create(Object)"), 0,
			    InsertCreateInsnDlg, "%p,%p", e, &sgObjectClass);
			AG_ToolbarButton(tbInsns, _("Create(Polyball)"), 0,
			    InsertCreateInsnDlg, "%p,%p", e, &sgPolyballClass);
			AG_ToolbarButton(tbInsns, _("Delete"), 0,
			    InsertDeleteInsn, "%p", e);
		}
	
		AG_PaneMoveDividerPct(e->paLeft, 50);
	}

	AG_ObjectAttach(e->paHoriz->div[1], sv);

	lbl = AG_LabelNewPolled(win, 0, _("Position: %u/%u"), &scr->t,
	                                                      &scr->tLast);
	AG_LabelSizeHint(lbl, 1, _("Position: 0000/0000"));

	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		AG_Numerical *numAlloc;

		e->slTime = AG_SliderNew(hBox, AG_SLIDER_HORIZ, AG_SLIDER_HFILL);
		AG_SliderSetControlSize(e->slTime, 32);
		AG_BindIntMp(e->slTime, "value", &scr->t, lock);
		AG_BindIntMp(e->slTime, "min", &scr->tFirst, lock);
		AG_BindIntMp(e->slTime, "max", &scr->tLast, lock);
		AG_SetEvent(e->slTime, "slider-changed", SliderChanged, "%p", e);

		AG_SeparatorNewVert(hBox);

		numAlloc = AG_NumericalNewS(hBox, 0, NULL, NULL);
		AG_SetInt(numAlloc, "value", 100);
		AG_SetInt(numAlloc, "min", 0);
		AG_ButtonNewFn(hBox, 0, "+",
		    AllocFrames, "%p,%p", scr, numAlloc);
	}

	statBox = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	e->stat = AG_LabelNew(statBox, AG_LABEL_STATIC | AG_LABEL_HFILL,
	    _("Idle"));

	AG_PaneMoveDividerPct(e->paHoriz, 20);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 60, 60);
	AG_WidgetFocus(sv);

	if (scr->n > 0) {
		SG_ScriptFrame *sf = &scr->frames[0];
		SG_ScriptInsn *si;

		TAILQ_FOREACH(si, &sf->insns, insns) {
			if (ExecInsn(e, NULL, si) == -1) {
				AG_LabelText(e->stat, _("Seek failed: %s"), AG_GetError());
				break;
			}
		}
		scr->tPrev = 0;
	}
	return (win);
}

AG_ObjectClass sgScriptClass = {
	"SG_Script",
	sizeof(SG_Script),
	{ 0,0 },
	Init,
	Reset,
	NULL,			/* destroy */
	Load,
	Save,
	Edit
};
