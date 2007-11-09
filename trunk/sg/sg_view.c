/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Visualization widget for SG(3) objects.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sg.h"
#include "icons.h"

SG_View	*
SG_ViewNew(void *parent, SG *sg, Uint flags)
{
	SG_View *sv;

	sv = Malloc(sizeof(SG_View));
	AG_ObjectInit(sv, &sgViewOps);
	sv->flags |= flags;
	sv->sg = sg;

	if (flags & SG_VIEW_HFILL) WIDGET(sv)->flags |= AG_WIDGET_HFILL;
	if (flags & SG_VIEW_VFILL) WIDGET(sv)->flags |= AG_WIDGET_VFILL;

	AG_ObjectAttach(parent, sv);
	return (sv);
}

int
SG_UnProject(SG_Real wx, SG_Real wy, SG_Real wz, const SG_Matrix *M,
    const SG_Matrix *P, int *vp, SG_Vector *vOut)
{
	SG_Matrix A;
	SG_Vector4 in, out;

	MatMultpv(&A, M, P);
	in.x = wx;
	in.y = wy;
	in.z = wz;
	in.w = 1.0;
	in.x = (in.x - vp[0]) / vp[2];
	in.y = (in.y - vp[1]) / vp[3];
	in.x = in.x*2.0 - 1.0;
	in.y = in.y*2.0 - 1.0;
	in.z = in.z*2.0 - 1.0;

	out = MatMultVector4p(&A, &in);
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

	AG_LockGL();
	SG_CameraGetProjection(sv->cam, &P);
	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			AGGLVIEW(sv)->mProjection[(m<<2)+n] = P.m[n][m];
	}
	AG_UnlockGL();
}

int
SG_ViewUnProject(SG_View *sv, SG_Vector w, SG_Real zNear, SG_Real zFar,
    SG_Vector *vOut)
{
	SG_Matrix T, P;
	SG_Real y, zNearSv, zFarSv;
	int vp[4];

	vp[0] = WIDGET(sv)->cx;
	vp[1] = agView->h - WIDGET(sv)->cy2;
	vp[2] = WIDGET(sv)->w;
	vp[3] = WIDGET(sv)->h;
	y = (SG_Real)(agView->h - WIDGET(sv)->cy2 + WIDGET(sv)->h - w.y);

	zNearSv = sv->cam->zNear; sv->cam->zNear = zNear;
	zFarSv = sv->cam->zFar; sv->cam->zFar = zFar;
	SG_CameraGetProjection(sv->cam, &P);
	sv->cam->zNear = zNearSv;
	sv->cam->zFar = zFarSv;

	SG_GetNodeTransform(sv->cam, &T);
	SG_UnProject(WIDGET(sv)->cx+w.x, y, w.z, &T, &P, vp, vOut);
	return (0);
}

static void
ViewOverlay(AG_Event *event)
{
	char text[1024];
	SG_View *sv = AG_PTR(1);
	SDL_Surface *su;
	SG_Vector v;

	v = SG_NodePos(sv->cam);

	snprintf(text, sizeof(text), "%s (%f,%f,%f)",
	    SGNODE(sv->cam)->name, v.x, v.y, v.z);

	AG_TextColor(TEXT_COLOR);
	su = AG_TextRender(text);
	AG_WidgetBlit(sv, su, 0, WIDGET(sv)->h - su->h);
	SDL_FreeSurface(su);
}

static void
SetupLights(SG_View *sv, SG_Node *root)
{
	SG_Light *lt;

	/* XXX TODO map into array */
	SG_FOREACH_SUBNODE_CLASS(lt, root, sg_light, "Light:*") {
		if (lt->light != GL_INVALID_ENUM)
			SG_LightSetup(lt, sv);
	}
}

static void
ViewDraw(AG_Event *event)
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
		glPushAttrib(GL_LIGHTING_BIT);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		/* XXX TODO array */
		SetupLights(sv, sv->sg->root);
		glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0);
	}

	/* Render the scene. */
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	SG_RenderNode(sv->sg, sv->sg->root, sv);
	glPopMatrix();

	if ((sv->flags & SG_VIEW_NO_LIGHTING) == 0) {
		glPopAttrib();
	}
	glPopAttrib();
}

static void
ViewScale(AG_Event *event)
{
	SG_View *sv = AG_PTR(1);

	AG_LockGL();
	glMatrixMode(GL_PROJECTION);
	SG_CameraProject(sv->cam);
	AG_UnlockGL();
}

static void
RotateCameraByMouse(SG_View *sv, int x, int y)
{
	SG_CameraRotMouse(sv->cam, sv, x, y);
}

static void
MoveCameraByMouse(SG_View *sv, int xrel, int yrel, int zrel)
{
	SG_Vector m;
	
	m.x = sv->mouse.tsens.x*(-(SG_Real)xrel);
	m.y = sv->mouse.tsens.y*((SG_Real)yrel);
	m.z = sv->mouse.tsens.z*(-(SG_Real)zrel);

	/* Translate along the global axis, not the camera axis. */
//	T->m[0][3] += m.x;
//	T->m[1][3] += m.y;
//	T->m[2][3] += m.z;
#if 1
	/* Translate along the local camera axis. */
	MatTranslate(&SGNODE(sv->cam)->T, m);
#endif

	if (sv->cam->pmode == SG_CAMERA_ORTHOGRAPHIC) {
		SG_ViewUpdateProjection(sv);
	}
}

static void
ViewMotion(AG_Event *event)
{
	SG_View *sv = AG_PTR(1);
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
ViewSwitchCamera(AG_Event *event)
{
	SG_View *sv = AG_PTR(1);
	SG_Camera *cam = AG_PTR(2);

	sv->cam = cam;
}

static void
PopupMenuOpen(SG_View *sv, int x, int y)
{
	SG *sg = sv->sg;
	AG_MenuItem *mRoot, *mOvl, *mCam;

	if (sv->popup != NULL) {
		AG_PopupDestroy(sv, sv->popup);
	}
	sv->popup = AG_PopupNew(sv);
	mRoot = sv->popup->item;

	AG_MenuUintFlags(mRoot, _("Lighting"),
	    sgIconLighting.s, &sv->flags, SG_VIEW_NO_LIGHTING, 1);
	AG_MenuUintFlags(mRoot, _("Z-Buffer"),
	    sgIconZBuffer.s, &sv->flags, SG_VIEW_NO_DEPTH_TEST, 1);

	mOvl = AG_MenuNode(mRoot, _("Overlay"), NULL);
	{
		AG_MenuUintFlags(mOvl, _("Wireframe"),
		    sgIconWireframe.s,
		    &sg->flags, SG_OVERLAY_WIREFRAME, 0);
		AG_MenuUintFlags(mOvl, _("Vertices"),
		    sgIconVertices.s,
		    &sg->flags, SG_OVERLAY_VERTICES, 0);
		AG_MenuUintFlags(mOvl, _("Facet normals"),
		    sgIconFacetNormals.s,
		    &sg->flags, SG_OVERLAY_FNORMALS, 0);
		AG_MenuUintFlags(mOvl, _("Vertex normals"),
		    sgIconVertexNormals.s,
		    &sg->flags, SG_OVERLAY_VNORMALS, 0);
	}
	mCam = AG_MenuNode(mRoot, _("Switch to camera"), NULL);
	{
		AG_MenuItem *mi;
		SG_Camera *cam;

		SG_FOREACH_NODE_CLASS(cam, sg, sg_camera, "Camera:*") {
			mi = AG_MenuAction(mCam, SGNODE(cam)->name,
			    sgIconCamera.s,
			    ViewSwitchCamera, "%p,%p", sv, cam);
			mi->state = (cam != sv->cam);
		}
	}
	AG_PopupShow(sv->popup);
}

static void
SelectByMouse(SG_View *sv, int x, int y)
{
	SG_Vector vOut;

	if (SG_ViewUnProject(sv, VecGet((SG_Real)x, (SG_Real)y, 0.0),
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
ViewKeydown(AG_Event *event)
{
	SG_View *sv = AG_PTR(1);
	int keysym = AG_INT(2);
	int kmod = AG_INT(3);

	switch (keysym) {
	case SDLK_LEFT:
		if (kmod & KMOD_CTRL) {
			sv->rot_yaw = -sv->rot_yaw_incr;
			AG_ReplaceTimeout(sv, &sv->to_rot_yaw, sv->rot_vel_min);
		} else {
			sv->trans_x = -sv->trans_x_incr;
			AG_ReplaceTimeout(sv, &sv->to_trans_x,
			                  sv->trans_vel_min);
		}
		break;
	case SDLK_RIGHT:
		if (kmod & KMOD_CTRL) {
			sv->rot_yaw = +sv->rot_yaw_incr;
			AG_ReplaceTimeout(sv, &sv->to_rot_yaw, sv->rot_vel_min);
		} else {
			sv->trans_x = +sv->trans_x_incr;
			AG_ReplaceTimeout(sv, &sv->to_trans_x,
			                  sv->trans_vel_min);
		}
		break;
	case SDLK_UP:
		if (kmod & KMOD_CTRL) {
			sv->rot_pitch = -sv->rot_pitch_incr;
			AG_ReplaceTimeout(sv, &sv->to_rot_pitch,
			                  sv->rot_vel_min);
		} else {
			sv->trans_y = +sv->trans_y_incr;
			AG_ReplaceTimeout(sv, &sv->to_trans_y,
			                  sv->trans_vel_min);
		}
		break;
	case SDLK_DOWN:
		if (kmod & KMOD_CTRL) {
			sv->rot_pitch = +sv->rot_pitch_incr;
			AG_ReplaceTimeout(sv, &sv->to_rot_pitch,
			                  sv->rot_vel_min);
		} else {
			sv->trans_y = -sv->trans_y_incr;
			AG_ReplaceTimeout(sv, &sv->to_trans_y,
			                  sv->trans_vel_min);
		}
		break;
	case SDLK_PAGEUP:
		sv->trans_z = -sv->trans_z_incr;
		AG_ReplaceTimeout(sv, &sv->to_trans_z, sv->trans_vel_min);
		break;
	case SDLK_PAGEDOWN:
		sv->trans_z = +sv->trans_z_incr;
		AG_ReplaceTimeout(sv, &sv->to_trans_z, sv->trans_vel_min);
		break;
	case SDLK_DELETE:
		sv->rot_roll = -sv->rot_roll_incr;
		AG_ReplaceTimeout(sv, &sv->to_rot_roll, sv->rot_vel_min);
		break;
	case SDLK_END:
		sv->rot_roll = +sv->rot_roll_incr;
		AG_ReplaceTimeout(sv, &sv->to_rot_roll, sv->rot_vel_min);
		break;
	}
}

static void
ViewKeyup(AG_Event *event)
{
	SG_View *sv = AG_PTR(1);
	int keysym = AG_INT(2);

	switch (keysym) {
	case SDLK_LEFT:
	case SDLK_RIGHT:
		AG_DelTimeout(sv, &sv->to_rot_yaw);
		AG_DelTimeout(sv, &sv->to_trans_x);
		break;
	case SDLK_UP:
	case SDLK_DOWN:
		AG_DelTimeout(sv, &sv->to_rot_pitch);
		AG_DelTimeout(sv, &sv->to_trans_y);
		break;
	case SDLK_PAGEUP:
	case SDLK_PAGEDOWN:
		AG_DelTimeout(sv, &sv->to_trans_z);
		break;
	case SDLK_DELETE:
	case SDLK_END:
		AG_DelTimeout(sv, &sv->to_rot_roll);
		break;
	}
}

static void
ViewButtondown(AG_Event *event)
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
Attached(AG_Event *event)
{
	SG_Camera *cam;
	SG_View *sv = AG_SELF();

	/* Attach to the default camera. */
	if ((cam = SG_FindNode(sv->sg, "Camera0")) == NULL) {
		SG_AttachDefaultNodes(sv->sg);
		cam = SG_FindNode(sv->sg, "Camera0");
	}
	SG_ViewSetCamera(sv, cam);
}

static Uint32
RotateYawTimeout(void *obj, Uint32 ival, void *arg)
{
	SG_View *sv = obj;

	SG_Rotatev(sv->cam, sv->rot_yaw, VecJ());
	return (ival > sv->rot_vel_max ? ival-sv->rot_vel_accel : ival);
}

static Uint32
RotatePitchTimeout(void *obj, Uint32 ival, void *arg)
{
	SG_View *sv = obj;

	SG_Rotatev(sv->cam, sv->rot_pitch, VecI());
	return (ival > sv->rot_vel_max ? ival-sv->rot_vel_accel : ival);
}

static Uint32
RotateRollTimeout(void *obj, Uint32 ival, void *arg)
{
	SG_View *sv = obj;

	SG_Rotatev(sv->cam, sv->rot_roll, VecK());
	return (ival > sv->rot_vel_max ? ival-sv->rot_vel_accel : ival);
}

static Uint32
TranslateXTimeout(void *obj, Uint32 ival, void *arg)
{
	SG_View *sv = obj;
	SG_TranslateX(sv->cam, sv->trans_x);
	return (ival > sv->trans_vel_max ? ival-sv->trans_vel_accel : ival);
}

static Uint32
TranslateYTimeout(void *obj, Uint32 ival, void *arg)
{
	SG_View *sv = obj;
	SG_TranslateY(sv->cam, sv->trans_y);
	return (ival > sv->trans_vel_max ? ival-sv->trans_vel_accel : ival);
}

static Uint32
TranslateZTimeout(void *obj, Uint32 ival, void *arg)
{
	SG_View *sv = obj;
	SG_TranslateZ(sv->cam, sv->trans_z);
	return (ival > sv->trans_vel_max ? ival-sv->trans_vel_accel : ival);
}

static void
Init(void *obj)
{
	SG_View *sv = obj;
	AG_GLView *glv = obj;

	AG_GLViewDrawFn(glv, ViewDraw, "%p", sv);
	AG_GLViewOverlayFn(glv, ViewOverlay, "%p", sv);
	AG_GLViewScaleFn(glv, ViewScale, "%p", sv);
	AG_GLViewMotionFn(glv, ViewMotion, "%p", sv);
	AG_GLViewButtondownFn(glv, ViewButtondown, "%p", sv);
	AG_GLViewKeydownFn(glv, ViewKeydown, "%p", sv);
	AG_GLViewKeyupFn(glv, ViewKeyup, "%p", sv);

	sv->flags = 0;
	sv->sg = NULL;
	sv->cam = NULL;
	sv->editPane = NULL;
	sv->popup = NULL;

	sv->mouse.rsens = VecGet(0.002, 0.002, 0.002);
	sv->mouse.tsens = VecGet(0.01, 0.01, 0.5);

	sv->rot_vel_min = 30;
	sv->rot_vel_accel = 1;
	sv->rot_vel_max = 10;
	sv->trans_vel_min = 20;
	sv->trans_vel_accel = 1;
	sv->trans_vel_max = 5;

	sv->rot_yaw_incr = 0.05;
	sv->rot_pitch_incr = 0.05;
	sv->rot_roll_incr = 0.05;
	sv->trans_x_incr = 0.1;
	sv->trans_y_incr = 0.1;
	sv->trans_z_incr = 0.1;

	AG_SetTimeout(&sv->to_rot_yaw, RotateYawTimeout, NULL, 0);
	AG_SetTimeout(&sv->to_rot_pitch, RotatePitchTimeout, NULL, 0);
	AG_SetTimeout(&sv->to_rot_roll, RotateRollTimeout, NULL, 0);
	AG_SetTimeout(&sv->to_trans_x, TranslateXTimeout, NULL, 0);
	AG_SetTimeout(&sv->to_trans_y, TranslateYTimeout, NULL, 0);
	AG_SetTimeout(&sv->to_trans_z, TranslateZTimeout, NULL, 0);
	
	AG_SetEvent(sv, "attached", Attached, NULL);
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

const AG_WidgetOps sgViewOps = {
	{
		"AG_Widget:AG_GLView:SG_View",
		sizeof(SG_View),
		{ 0,0 },
		NULL,		/* init */
		Init,
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_GLViewDraw,
	AG_GLViewSizeRequest,
	AG_GLViewSizeAllocate,
};

#endif /* HAVE_OPENGL */
