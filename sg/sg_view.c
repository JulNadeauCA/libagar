/*
 * Copyright (c) 2005-2007 Hypertriton, Inc.
 * <http://www.hypertriton.com/>
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

#include <agar/core/core.h>

#include "sg.h"

#include <GL/gl.h>
#include <GL/glu.h>

const AG_WidgetOps sgViewOps = {
	{
		"AG_Widget:AG_GLView:SG_View",
		sizeof(SG_View),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		SG_ViewDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_GLViewDraw,
	AG_GLViewScale
};

SG_View	*
SG_ViewNew(void *parent, SG *sg, Uint flags)
{
	SG_View *sv;

	sv = Malloc(sizeof(SG_View), M_OBJECT);
	SG_ViewInit(sv, sg, flags);
	AG_ObjectAttach(parent, sv);
	return (sv);
}

int
SG_UnProject(SG_Real wx, SG_Real wy, SG_Real wz, const SG_Matrix *M,
    const SG_Matrix *P, int *vp, SG_Vector *vOut)
{
	SG_Matrix A;
	SG_Vector4 in, out;

	SG_MatrixMultpv(&A, M, P);
	in.x = wx;
	in.y = wy;
	in.z = wz;
	in.w = 1.0;
	in.x = (in.x - vp[0]) / vp[2];
	in.y = (in.y - vp[1]) / vp[3];
	in.x = in.x*2.0 - 1.0;
	in.y = in.y*2.0 - 1.0;
	in.z = in.z*2.0 - 1.0;

	out = SG_MatrixMultVector4p(&A, &in);
	if (out.w == 0.0) { return (-1); }
	out.x /= out.w;
	out.y /= out.w;
	out.z /= out.w;
	vOut->x = out.x;
	vOut->y = out.y;
	vOut->z = out.z;
	return (0);
}

void
SG_ViewUpdateProjection(SG_View *sv)
{
	SG_Matrix P;
	int m, n;

	SG_CameraGetProjection(sv->cam, &P);
	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			AGGLVIEW(sv)->mProjection[(m<<2)+n] = P.m[n][m];
	}
}

int
SG_ViewUnProject(SG_View *sv, SG_Vector w, SG_Real zNear, SG_Real zFar,
    SG_Vector *vOut)
{
	SG_Matrix T, P;
	SG_Real y, zNearSv, zFarSv;
	int vp[4];

	vp[0] = AGWIDGET(sv)->cx;
	vp[1] = agView->h - AGWIDGET(sv)->cy2;
	vp[2] = AGWIDGET(sv)->w;
	vp[3] = AGWIDGET(sv)->h;
	y = (SG_Real)(agView->h - AGWIDGET(sv)->cy2 + AGWIDGET(sv)->h - w.y);

	zNearSv = sv->cam->zNear; sv->cam->zNear = zNear;
	zFarSv = sv->cam->zFar; sv->cam->zFar = zFar;
	SG_CameraGetProjection(sv->cam, &P);
	sv->cam->zNear = zNearSv;
	sv->cam->zFar = zFarSv;

	SG_GetNodeTransform(sv->cam, &T);
	SG_UnProject(AGWIDGET(sv)->cx+w.x, y, w.z, &T, &P, vp, vOut);
	return (0);
}


static void
DrawEv(AG_Event *event)
{
	SG_View *sv = AG_PTR(1);

	glPushAttrib(GL_ENABLE_BIT|GL_POLYGON_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	if ((sv->flags & SG_VIEW_NO_DEPTH_TEST) == 0)
		glEnable(GL_DEPTH_TEST);

	/* Set rendering parameters. */
	if (sv->cam->polyFace.cull && sv->cam->polyBack.cull) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT_AND_BACK);
	} else if (sv->cam->polyFace.cull) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	} else if (sv->cam->polyBack.cull) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	} else {
		glDisable(GL_CULL_FACE);
	}
	switch (sv->cam->polyFace.mode) {
	case SG_CAMERA_POINTS:
		glPolygonMode(GL_FRONT, GL_POINT);
		break;
	case SG_CAMERA_WIREFRAME:
		glPolygonMode(GL_FRONT, GL_LINE);
		break;
	case SG_CAMERA_FLAT_SHADED:
		glPolygonMode(GL_FRONT, GL_FILL);
		glShadeModel(GL_FLAT);
		break;
	case SG_CAMERA_SMOOTH_SHADED:
		glPolygonMode(GL_FRONT, GL_FILL);
		glShadeModel(GL_SMOOTH);
		break;
	}
	switch (sv->cam->polyBack.mode) {
	case SG_CAMERA_POINTS:
		glPolygonMode(GL_BACK, GL_POINT);
		break;
	case SG_CAMERA_WIREFRAME:
		glPolygonMode(GL_BACK, GL_LINE);
		break;
	case SG_CAMERA_FLAT_SHADED:
		glPolygonMode(GL_BACK, GL_FILL);
		glShadeModel(GL_FLAT);
		break;
	case SG_CAMERA_SMOOTH_SHADED:
		glPolygonMode(GL_BACK, GL_FILL);
		glShadeModel(GL_SMOOTH);
		break;
	}

	/* Set the modelview matrix for the current camera. */
	glLoadIdentity();
	SG_CameraSetup(sv->cam);

	/* Enable the light sources. */
	if ((sv->flags & SG_VIEW_NO_LIGHTING) == 0) {
		SG_Light *lt;
	
		glPushAttrib(GL_LIGHTING_BIT);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

#if 0
		SG_FOREACH_NODE_CLASS(lt, sv->sg, sg_light, "Light:*") {
			if (lt->light != GL_INVALID_ENUM)
				SG_LightSetup(lt);
		}
#endif
		glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0);
	}

	/* Render the scene. */
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	SG_RenderNode(sv->sg, SGNODE(sv->sg->root), sv);
	glPopMatrix();
	
	if ((sv->flags & SG_VIEW_NO_LIGHTING) == 0) {
		glPopAttrib();
	}
	glPopAttrib();
}

static void
ScaleEv(AG_Event *event)
{
	SG_View *sv = AG_PTR(1);

	glMatrixMode(GL_PROJECTION);
	SG_CameraProject(sv->cam);
}

static void
RotateCameraByMouse(SG_View *sv, int x, int y)
{
	SG_Real dPitch = 0.01*(SG_Real)y;
	SG_Real dYaw = 0.01*(SG_Real)x;
	SG_Matrix *T = &SGNODE(sv->cam)->T;
	SG_Vector pos = SG_NodePos(sv->cam);

	SG_MatrixTranslatev(T, SG_VectorMirror(pos,1,1,1));
	SG_MatrixRotatev(T, dPitch, SG_I);
	SG_MatrixRotatev(T, dYaw, SG_J);
	SG_MatrixTranslatev(T, pos);
}

static void
MoveCameraByMouse(SG_View *sv, int xrel, int yrel, int zrel)
{
	SG_Vector m;
	SG_Matrix *T = &SGNODE(sv->cam)->T;
	
	m.x = sv->mouse.tsens.x*((SG_Real)xrel);
	m.y = sv->mouse.tsens.y*(-(SG_Real)yrel);
	m.z = sv->mouse.tsens.z*((SG_Real)zrel);

	/* Translate along the global axis, not the camera axis. */
	T->m[0][3] += m.x;
	T->m[1][3] += m.y;
	T->m[2][3] += m.z;
#if 0
	/* Translate along the local camera axis. */
	SG_MatrixTranslatev(&SGNODE(sv->cam)->T, m);
#endif

	if (sv->cam->pmode == SG_CAMERA_ORTHOGRAPHIC) {
		SG_ViewUpdateProjection(sv);
	}
}

static void
MotionEv(AG_Event *event)
{
	SG_View *sv = AG_PTR(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	int xrel = AG_INT(4);
	int yrel = AG_INT(5);
	int state = AG_INT(6);

	if (state & SDL_BUTTON_LEFT) {
		if (SDL_GetModState() & KMOD_CTRL) {
			RotateCameraByMouse(sv, xrel, yrel);
		} else {
			MoveCameraByMouse(sv, xrel, yrel, 0);
		}
	}
}

static void
PopupMenuClose(SG_View *sv)
{
	AG_MenuCollapse(sv->popup.menu, sv->popup.item);
	AG_ObjectDestroy(sv->popup.menu);
	Free(sv->popup.menu, M_OBJECT);

	sv->popup.menu = NULL;
	sv->popup.item = NULL;
	sv->popup.win = NULL;
}

static void
PopupMenuOpen(SG_View *sv, int x, int y)
{
	SG *sg = sv->sg;

	if (sv->popup.menu != NULL)
		PopupMenuClose(sv);

	sv->popup.menu = Malloc(sizeof(AG_Menu), M_OBJECT);
	AG_MenuInit(sv->popup.menu, 0);

	sv->popup.item = AG_MenuAddItem(sv->popup.menu, NULL);
	{
		AG_MenuItem *mOvl;

		AG_MenuIntFlags(sv->popup.item, _("Lighting"),
		    RG_CONTROLS_ICON, &sv->flags, SG_VIEW_NO_LIGHTING, 1);
		AG_MenuIntFlags(sv->popup.item, _("Z-Buffer"),
		    RG_CONTROLS_ICON, &sv->flags, SG_VIEW_NO_DEPTH_TEST, 1);

		mOvl = AG_MenuNode(sv->popup.item, _("Overlay"), -1);
		{
			AG_MenuIntFlags(mOvl, _("Wireframe"),
			    GRID_ICON,
			    &sg->flags, SG_OVERLAY_WIREFRAME, 0);
			AG_MenuIntFlags(mOvl, _("Vertices"),
			    VGPOINTS_ICON,
			    &sg->flags, SG_OVERLAY_VERTICES, 0);
			AG_MenuIntFlags(mOvl, _("Facet normals"),
			    UP_ARROW_ICON,
			    &sg->flags, SG_OVERLAY_FNORMALS, 0);
			AG_MenuIntFlags(mOvl, _("Vertex normals"),
			    UP_ARROW_ICON,
			    &sg->flags, SG_OVERLAY_VNORMALS, 0);
		}
	}
	sv->popup.menu->sel_item = sv->popup.item;
	sv->popup.win = AG_MenuExpand(sv->popup.menu, sv->popup.item,
	    AGWIDGET(sv)->cx+x, AGWIDGET(sv)->cy+y);
}

static void
SelectByMouse(SG_View *sv, int x, int y)
{
	int viewport[4];
	SG_Vector vOut;

	if (SG_ViewUnProject(sv, SG_VECTOR((SG_Real)x, (SG_Real)y, 0.0),
	    20.0, sv->cam->zFar, &vOut) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
	}
#if 0
	{
		SG_Node *node;

		dprintf("at %f,%f,%f\n", vOut.x, vOut.y, vOut.z);
		node = SG_NodeAdd(sv->sg->root, &sgSphereOps, 0);
		vOut.z -= 5.0;
		SG_NodeTranslateVec(node, vOut);
	}
#endif
}

static void
ButtondownEv(AG_Event *event)
{
	SG_View *sv = AG_PTR(1);
	int button = AG_INT(2);
	int x = AG_INT(3);
	int y = AG_INT(4);

	AG_WidgetFocus(sv);

	switch (button) {
	case SDL_BUTTON_WHEELUP:
		MoveCameraByMouse(sv, 0, 0, 1);
		break;
	case SDL_BUTTON_WHEELDOWN:
		MoveCameraByMouse(sv, 0, 0, -1);
		break;
	case SDL_BUTTON_RIGHT:
		PopupMenuOpen(sv, x, y);
		break;
	case SDL_BUTTON_LEFT:
		SelectByMouse(sv, x, y);
		break;
	}
}

void
SG_ViewSetCamera(SG_View *sv, SG_Camera *cam)
{
	sv->cam = cam;
	SG_ViewUpdateProjection(sv);
}

static void
SG_ViewAttached(AG_Event *event)
{
	SG_Camera *cam;
	SG_View *sv = AG_SELF();

	/* Attach to the default camera. */
	if ((cam = SG_FindNode(sv->sg, "Camera0")) == NULL) {
		fatal("no Camera0");
	}
	SG_ViewSetCamera(sv, cam);
}

void
SG_ViewInit(SG_View *sv, SG *sg, Uint flags)
{
	Uint glvflags = AG_GLVIEW_FOCUS;

	if (flags & SG_VIEW_HFILL) { glvflags |= AG_GLVIEW_HFILL; }
	if (flags & SG_VIEW_VFILL) { glvflags |= AG_GLVIEW_VFILL; }

	AG_GLViewInit(AGGLVIEW(sv), glvflags);
	AG_GLViewDrawFn(AGGLVIEW(sv), DrawEv, "%p", sv);
	AG_GLViewScaleFn(AGGLVIEW(sv), ScaleEv, "%p", sv);
	AG_GLViewMotionFn(AGGLVIEW(sv), MotionEv, "%p", sv);
	AG_GLViewButtondownFn(AGGLVIEW(sv), ButtondownEv, "%p", sv);
	AG_ObjectSetOps(sv, &sgViewOps);

	sv->flags = flags;
	sv->sg = sg;
	sv->cam = NULL;
	sv->popup.menu = NULL;
	sv->popup.item = NULL;
	sv->popup.win = NULL;
	sv->editPane = NULL;

	AG_SetEvent(sv, "attached", SG_ViewAttached, NULL);

	sv->mouse.rsens = SG_VECTOR(0.23, 0.13, 0.0);
	sv->mouse.tsens = SG_VECTOR(0.01, 0.01, 0.5);
}

void
SG_ViewDestroy(void *p)
{
	AG_GLViewDestroy(p);
}

void
SG_ViewKeydownFn(SG_View *sv, AG_EventFn fn, const char *fmt, ...)
{
}

void
SG_ViewKeyupFn(SG_View *sv, AG_EventFn fn, const char *fmt, ...)
{
}

void
SG_ViewButtondownFn(SG_View *sv, AG_EventFn fn, const char *fmt, ...)
{
}

void
SG_ViewButtonupFn(SG_View *sv, AG_EventFn fn, const char *fmt, ...)
{
}

void
SG_ViewMotionFn(SG_View *sv, AG_EventFn fn, const char *fmt, ...)
{
}
