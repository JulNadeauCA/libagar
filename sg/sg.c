/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Agar-SG library initialization and implementation of the base
 * Scene Graph (SG) class.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

#include <math.h>
#include <string.h>

/* Standard Agar-SG classes */
void *sgStdClasses[] = {
	&sgClass,
	&sgNodeClass,
	&sgProgramClass,
/*	&sgCgProgramClass, <- Legacy */
	&sgTextureClass,
	&sgPaletteClass,
	&sgScriptClass,
	/* Non-geometrical */
	&sgDummyClass,
	&sgCameraClass,
	&sgLightClass,
	/* Reference geometry, editor controls */
	&sgGeomClass,
	&sgWidgetClass,
	&sgPointClass,
	&sgLineClass,
	&sgCircleClass,
	&sgSphereClass,
	&sgPlaneClass,
	&sgPolygonClass,
	&sgTriangleClass,
	&sgRectangleClass,
	/* BREP objects */
	&sgObjectClass,
	&sgPolyballClass,
	&sgPolyboxClass,
	/* Volumetric objects */
	&sgVoxelClass,
	/* Thin objects */
	&sgImageClass,
	NULL
};

/* File extension mappings */
const AG_FileExtMapping sgFileExtMap[] = {
	{ ".sg",	N_("Scene"),		&sgClass,		1 },
	{ ".sgs",	N_("Script"),		&sgScriptClass,		1 },
	{ ".sgt",	N_("Texture"),		&sgTextureClass,	1 },
	{ ".sgp",	N_("Palette"),		&sgPaletteClass,	1 },
	{ ".sgo",	N_("Object"),		&sgObjectClass,		0 },
};
const Uint sgFileExtCount = sizeof(sgFileExtMap) / sizeof(sgFileExtMap[0]);
int sgInitedSubsystem = 0;

/* Initialize the SG library. */
void
SG_InitSubsystem(void)
{
	void **cls;

	if (sgInitedSubsystem++ > 0)
		return;

	M_InitSubsystem();
	
	AG_RegisterNamespace("Agar-SG", "SG_", "http://libagar.org/");
	AG_RegisterFileExtMappings(sgFileExtMap, sgFileExtCount);

	for (cls = &sgStdClasses[0]; *cls != NULL; cls++)
		AG_RegisterClass(*cls);

	/* Initialize GUI facilities for SG_Edit() */
	SG_InitGUI();
}

/* Cleanup the SG library. */
void
SG_DestroySubsystem(void)
{
	void **cls;

	if (--sgInitedSubsystem > 0) {
		return;
	}
	SG_DestroyGUI();
	for (cls = &sgStdClasses[0]; *cls != NULL; cls++) {
		AG_UnregisterClass(*cls);
	}
	AG_UnregisterNamespace("Agar-SG");
}

/* Create a new SG object. */
SG *
SG_New(void *parent, const char *name, Uint flags)
{
	SG *sg;

	sg = Malloc(sizeof(SG));
	AG_ObjectInit(sg, &sgClass);
	if (name) {
		AG_ObjectSetNameS(sg, name);
	} else {
		OBJECT(sg)->flags |= AG_OBJECT_NAME_ONATTACH;
	}

	sg->root = (SG_Node *)SG_PointNew(sg, "_root", NULL);
	SG_PointSize(SGPOINT(sg->root), 0);
	sg->root->flags |= SG_NODE_HIDE;
	OBJECT(sg->root)->flags |= AG_OBJECT_INDESTRUCTIBLE;

	if (!(flags & SG_NO_DEFAULT_NODES)) {
		sg->def.cam = SG_CameraNew(sg->root, "Camera0");
		SG_Translate(sg->def.cam, 0.0, 0.0, 10.0);
		SG_CameraSetRotCtrlCircular(sg->def.cam, sg->root);
		OBJECT(sg->def.cam)->flags |= AG_OBJECT_INDESTRUCTIBLE;

		sg->def.lt[0] = SG_LightNew(sg->root, "Light0");
		SG_Translate(sg->def.lt[0], 30.0, 30.0, 30.0);
		OBJECT(sg->def.lt[0])->flags |= AG_OBJECT_INDESTRUCTIBLE;

		sg->def.lt[1] = SG_LightNew(sg->root, "Light1");
		SG_Translate(sg->def.lt[1], -30.0, -30.0, -30.0);
		OBJECT(sg->def.lt[1])->flags |= AG_OBJECT_INDESTRUCTIBLE;
	}
	
	AG_ObjectAttach(parent, sg);
	return (sg);
}

static void
Init(void *_Nonnull obj)
{
	SG *sg = obj;
	
	OBJECT(sg)->flags |= AG_OBJECT_REOPEN_ONLOAD;
	sg->flags = 0;
	sg->root = NULL;
	sg->def.cam = NULL;
	sg->def.lt[0] = NULL;
	sg->def.lt[1] = NULL;
	TAILQ_INIT(&sg->nodes);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	SG *sg = obj;

	AG_WriteUint32(ds, sg->flags);
	return SG_NodeSave(sg, ds, sg->root);
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	SG *sg = obj;

	sg->flags = (Uint)AG_ReadUint32(ds);
	return SG_NodeLoad(sg, ds, NULL);
}

/* Search child nodes by name. */
SG_Node *
SG_SearchNodes(SG_Node *node, const char *name)
{
#ifdef AG_THREADS
	SG *sg = node->sg;
#endif
	SG_Node *chld, *rnode;

	AG_ObjectLock(sg);
	OBJECT_FOREACH_CLASS(chld, node, sg_node, "SG_Node:*") {
		if (strcmp(OBJECT(chld)->name, name) == 0) {
			AG_ObjectUnlock(sg);
			return (chld);
		} else {
			if ((rnode = SG_SearchNodes(chld, name)) != NULL) {
				AG_ObjectUnlock(sg);
				return (rnode);
			}
		}
	}
	AG_ObjectUnlock(sg);
	return (NULL);
}

/* Search entire graph for a node by name. */
void *
SG_FindNode(SG *sg, const char *name)
{
	void *rv;

	AG_ObjectLock(sg);
	if (strcmp(name, OBJECT(sg->root)->name) == 0) {
		rv = sg->root;
	} else {
		rv = (void *)SG_SearchNodes(sg->root, name);
	}
	AG_ObjectUnlock(sg);
	return (rv);
}

static void
ClearNodes(AG_Object *_Nonnull obj)
{
	AG_Object *cob, *ncob;
	
	AG_ObjectLock(obj);
	for (cob = TAILQ_FIRST(&obj->children);
	     cob != TAILQ_END(&obj->children);
	     cob = ncob) {
		ncob = TAILQ_NEXT(cob, cobjs);
		ClearNodes(cob);
	}
	AG_ObjectUnlock(obj);

	AG_ObjectDetach(obj);
	AG_ObjectDestroy(obj);
}

/*
 * Clear the scene, destroying all nodes (except the default nodes which
 * are reinitialized).
 */
void
SG_Clear(SG *sg)
{
	AG_Object *cob, *ncob;

	AG_ObjectLock(sg);
	for (cob = TAILQ_FIRST(&OBJECT(sg->root)->children);
	     cob != TAILQ_END(&OBJECT(sg->root)->children);
	     cob = ncob) {
		ncob = TAILQ_NEXT(cob, cobjs);
		if (cob == OBJECT(sg->def.cam) ||
		    cob == OBJECT(sg->def.lt[0]) ||
		    cob == OBJECT(sg->def.lt[1])) {
			continue;
		}
		ClearNodes(cob);
	}

	SG_Identity(sg->def.cam);
	SG_Translate(sg->def.cam, 0.0, 0.0, 10.0);
	SG_CameraSetRotCtrlCircular(sg->def.cam, sg->root);
	SG_Identity(sg->def.lt[0]);
	SG_Translate(sg->def.lt[0], 30.0, 30.0, 30.0);
	SG_Identity(sg->def.lt[1]);
	SG_Translate(sg->def.lt[1], -30.0, -30.0, -30.0);

	AG_ObjectUnlock(sg);
}

AG_ObjectClass sgClass = {
	"SG",
	sizeof(SG),
	{ 1,0 },
	Init,
	NULL,		/* reset */
	NULL,		/* destroy */
	Load,
	Save,
	SG_Edit
};
