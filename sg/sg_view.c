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
 * Visualization widget for SG(3).
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

M_Real sgEyeSeparation = 0.01;

/* Create a new SG_View widget. */
SG_View	*
SG_ViewNew(void *parent, SG *sg, Uint flags)
{
	SG_View *sv;

	sv = Malloc(sizeof(SG_View));
	AG_ObjectInit(sv, &sgViewClass);

	if (flags & SG_VIEW_HFILL) { WIDGET(sv)->flags |= AG_WIDGET_HFILL; }
	if (flags & SG_VIEW_VFILL) { WIDGET(sv)->flags |= AG_WIDGET_VFILL; }
	sv->flags |= flags;

	sv->sg = sg;

	if (sg != NULL) {
		AG_OBJECT_ISA(sg, "SG:*");
		AG_ObjectLock(sg);

		if (sg->def.cam != NULL)
			SG_ViewSetCamera(sv, sg->def.cam);

		AG_ObjectUnlock(sg);
	}

	AG_ObjectAttach(parent, sv);
	return (sv);
}

static void
ChangeScene(SG_View *_Nonnull sv, SG *_Nullable sg, SG_Camera *_Nullable cam)
{
	SG_Node *node;

#ifdef AG_DEBUG
	Debug(sv, "Change to: %s(%s)\n", OBJECT(sg)->name,
	    (cam != NULL) ? OBJECT(cam)->name : "NULL");
#endif
	if (sv->sg == sg) {
		SG_ViewSetCamera(sv, cam);
		return;
	}
	if (sv->sg) {
		AG_ObjectLock(sv->sg);
		SG_FOREACH_NODE(node, sv->sg) {
			AG_PostEvent(node, "view-detach", "%p", sv);
		}
		AG_ObjectUnlock(sv->sg);
	}

	sv->sg = sg;
	SG_ViewSetCamera(sv, cam);

	if (sg) {
		AG_ObjectLock(sg);
		SG_FOREACH_NODE(node, sg) {
			AG_PostEvent(node, "view-attach", "%p", sv);
		}
		AG_ObjectUnlock(sg);
	}
}

static Uint32
TransitionFade(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	SG_View *sv = SG_VIEW_SELF();

	AG_ObjectLock(sv);

	/* (10ms timer and range -1.0 to 1.0) */
	sv->transProgress += (10.0*2.0/sv->transDuration);

	if (sv->camTrans != NULL && Fabs(sv->transProgress) < 0.01) { /* Mid */
		ChangeScene(sv, sv->sgTrans, sv->camTrans);
		sv->sgTrans = NULL;
		sv->camTrans = NULL;
	} else if (sv->transProgress >= 1.0) {		/* End */
		sv->transProgress = 1.0;
		goto stop;
	}

	AG_Redraw(sv);
	AG_ObjectUnlock(sv);
	return (to->ival);
stop:
	sv->transFlags &= ~(SG_VIEW_TRANSFADE);
	AG_Redraw(sv);
	AG_ObjectUnlock(sv);
	return (0);
}

/* Initiate transition to a different scene. */
int
SG_ViewTransition(SG_View *sv, SG *sg, SG_Camera *cam, Uint flags)
{
	AG_ObjectLock(sv);

#ifdef AG_DEBUG
	Debug(sv, "Transition to: %s(%s)\n", OBJECT(sg)->name,
	    (cam != NULL) ? OBJECT(cam)->name : "NULL");
#endif
	if (cam == NULL) {
		AG_ObjectLock(sg);
		if ((cam = sg->def.cam) == NULL) {
			AG_SetError("No target camera");
			AG_ObjectUnlock(sg);
			return (-1);
		}
		AG_ObjectUnlock(sg);
	}

	if (sv->sgTrans != NULL) {
		AG_SetError("Transition already in progress");
		goto fail;
	}

	sv->transFlags = flags;

	if (flags & SG_VIEW_TRANSFADE) {
		sv->sgTrans = sg;
		sv->camTrans = cam;
		sv->transProgress = -1.0;
		AG_AddTimer(sv, &sv->toTransFade, 10, TransitionFade, NULL);
	} else {
		ChangeScene(sv, sg, cam);
	} 

	AG_ObjectUnlock(sv);
	return (0);
fail:
	AG_ObjectUnlock(sv);
	return (-1);
}

void
SG_ViewSetFadeColor(SG_View *sv, const AG_Color *c)
{
	AG_ObjectLock(sv);
	sv->transFadeColor = *c;
	AG_ObjectUnlock(sv);
}

/* Configure fade duration. */
void
SG_ViewSetFadeDuration(SG_View *sv, Uint ms)
{
	AG_ObjectLock(sv);
	sv->transDuration = ms;
	AG_ObjectUnlock(sv);
}

/*
 * Convert widget coordinates [x,y] to a vector vOut in world coordinates.
 * The output vector is on the active camera's near plane.
 */
void
SG_ViewUnProject(SG_View *sv, int x, int y, M_Vector4 *vNear, M_Vector4 *vFar)
{
	M_Rectangle3 rNear, rFar;
	M_Real w, h;

	/*
	 * Convert widget coordinates to normalized device coordinates,
	 * and project the point onto the camera's near plane.
	 */
	SG_CameraFrustum(sv->cam, &rNear, &rFar);
	w = M_VecDistance3(rNear.b, rNear.c);
	h = M_VecDistance3(rNear.a, rNear.b);
	vNear->x = (-0.5 + (M_Real)x/(M_Real)WIDTH(sv))*w;
	vNear->y = (+0.5 - (M_Real)y/(M_Real)HEIGHT(sv))*h;
	vNear->z = rNear.a.z;
	vNear->w = 1.0;
	
	w = M_VecDistance3(rFar.b, rFar.c);
	h = M_VecDistance3(rFar.a, rFar.b);
	vFar->x = (-0.5 + (M_Real)x/(M_Real)WIDTH(sv))*w;
	vFar->y = (+0.5 - (M_Real)y/(M_Real)HEIGHT(sv))*h;
	vFar->z = rFar.a.z;
	vFar->w = 1.0;
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
/*	SG_View *sv = obj; */

	r->w = 100;
	r->h = 100;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	if (a->w < 4 || a->h < 4) {
		return (-1);
	}
	return (0);
}

static void
OnOverlayCameraStatus(SG_View *_Nonnull sv)
{
	char text[128];
	AG_Surface *su;
	M_Vector3 v;

	if (sv->cam != NULL) {
		v = SG_NodePos(sv->cam);
		AG_Snprintf(text, sizeof(text), "%s (%f,%f,%f)",
		    OBJECT(sv->cam)->name, v.x, v.y, v.z);
	} else {
		Strlcpy(text, _("No camera"), sizeof(text));
	}
	AG_TextColor(&WCOLOR(sv, TEXT_COLOR));
	su = AG_TextRender(text);
	AG_WidgetBlit(sv, su, 0, HEIGHT(sv) - su->h);
	AG_SurfaceFree(su);
}

static void
OnOverlay(AG_Event *_Nonnull event)
{
	SG_View *sv = SG_VIEW_SELF();
	AG_Surface *su;
	
	if (sv->transProgress < 1.0) {		/* Fade in progress */
		AG_Rect r;
		AG_Color c;

		r.x = 0;
		r.y = 0;
		r.w = WIDTH(sv);
		r.h = HEIGHT(sv);

		c = sv->transFadeColor;
		c.a = AG_COLOR_LAST - (AG_Component)
		      (Fabs(sv->transProgress) * (AG_COLOR_LASTD-1.0));

		AG_DrawRectBlended(sv, &r, &c,
		    AG_ALPHA_SRC,
		    AG_ALPHA_ONE_MINUS_SRC);
	}
	if (sv->flags & SG_VIEW_CAMERA_STATUS) {
		OnOverlayCameraStatus(sv);
	}
	if (sv->flags & SG_VIEW_EDIT_STATUS &&
	    sv->editStatus[0] != '\0') {
		AG_TextColor(&WCOLOR(sv, TEXT_COLOR));
		su = AG_TextRender(sv->editStatus);
		AG_WidgetBlit(sv, su, 0, HEIGHT(sv) - su->h);
		AG_SurfaceFree(su);
	}
}

static void
SetupLights(SG_View *_Nonnull sv, SG_Node *_Nonnull root)
{
	SG_Light *lt;

	/* XXX TODO map into array */
	OBJECT_FOREACH_CLASS(lt, root, sg_light, "SG_Node:SG_Light:*") {
		if (lt->light != GL_INVALID_ENUM)
			SG_LightSetup(lt);
	}
}

static void
Draw(void *_Nonnull obj)
{
	SG_View *sv = obj;
	SG *sg = sv->sg;

	if (sg == NULL || sv->cam == NULL)
		return;

#ifdef AG_DEBUG
/*	Debug(sv, "Draw %s(%s)\n", sg ? OBJECT(sg)->name : "NULL", sv->cam ? OBJECT(sv->cam)->name : ""); */
#endif
	GL_PushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);

	AG_ObjectLock(sg);
	AG_ObjectLock(sv->cam);

	if (sv->flags & SG_VIEW_UPDATE_PROJ) {
		M_Matrix44 P;
		int m, n;

		SG_CameraGetProjection(sv->cam, &P);
#ifdef AG_DEBUG
		if (WIDGET(sv)->gl == NULL) { AG_FatalError("NULL gl"); }
#endif
		for (m = 0; m < 4; m++) {
			for (n = 0; n < 4; n++)
				WIDGET(sv)->gl->mProjection[(m<<2)+n] =
				    (float)P.m[n][m];
		}
		sv->flags &= ~(SG_VIEW_UPDATE_PROJ);
		AG_WidgetUpdate(sv);
	}
	if ((sv->flags & SG_VIEW_NO_DEPTH_TEST) == 0)
		GL_Enable(GL_DEPTH_TEST);

	/* Set the modelview matrix and rendering modes the current camera. */
	GL_LoadIdentity();
	SG_CameraSetup(sv->cam);

	/* Enable the light sources. */
	if ((sv->flags & SG_VIEW_NO_LIGHTING) == 0) {
		GL_PushAttrib(GL_LIGHTING_BIT);
		GL_Enable(GL_LIGHTING);
		GL_Enable(GL_LIGHT0);

		/* XXX TODO array */
		SetupLights(sv, sg->root);
		glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0);
	}
		
	/* Render the scene. */
	if (agStereo) {
		glDrawBuffer(GL_BACK_LEFT);
		glClear(GL_DEPTH_BUFFER_BIT);
		GL_MatrixMode(GL_PROJECTION);
		GL_PushMatrix();
		GL_LoadIdentity();
		SG_CameraProjectLeft(sv->cam);
		GL_MatrixMode(GL_MODELVIEW);
		GL_PushMatrix();
		SG_NodeDraw(sg, sg->root, sv);
		GL_PopMatrix();

		glDrawBuffer(GL_BACK_RIGHT);
		glClear(GL_DEPTH_BUFFER_BIT);
		GL_MatrixMode(GL_PROJECTION);
		GL_LoadIdentity();
		SG_CameraProjectRight(sv->cam);
		GL_MatrixMode(GL_MODELVIEW);
		GL_PushMatrix();
		SG_NodeDraw(sg, sg->root, sv);
		GL_PopMatrix();
		GL_MatrixMode(GL_PROJECTION);
		GL_PopMatrix();
		GL_MatrixMode(GL_MODELVIEW);
		
		glDrawBuffer(GL_BACK);
	} else {
		glClear(GL_DEPTH_BUFFER_BIT);
		GL_MatrixMode(GL_MODELVIEW);
		GL_PushMatrix();
		SG_NodeDraw(sg, sg->root, sv);
		GL_PopMatrix();
	}
	
	AG_ObjectUnlock(sv->cam);
	AG_ObjectUnlock(sg);
	
	if ((sv->flags & SG_VIEW_NO_LIGHTING) == 0) {
		GL_PopAttrib();
	}
	GL_PopAttrib();
}

static void
OnReshape(AG_Event *_Nonnull event)
{
	SG_View *sv = SG_VIEW_SELF();

	if (sv->cam == NULL)
		return;

	Debug(sv, "OnReshape (%d x %d)\n", WIDTH(sv), HEIGHT(sv));

	GL_MatrixMode(GL_PROJECTION);
	SG_CameraProject(sv->cam);
	GL_MatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	SG_View *sv = SG_VIEW_SELF();
/*	const int x = AG_INT(1); */
/*	const int y = AG_INT(2); */
	const int xrel = AG_INT(3);
	const int yrel = AG_INT(4);
	const int bs = AG_INT(5);

	if (bs & AG_MOUSE_LEFT) {
		if (AG_GetModState(sv) & AG_KEYMOD_CTRL) {
			SG_CameraRotMouse(sv->cam, sv, xrel, yrel);
		} else {
			SG_CameraMoveMouse(sv->cam, sv, xrel, yrel, 0);
		}
	}
}

static void
ViewSwitchCamera(AG_Event *_Nonnull event)
{
	SG_View *sv = SG_VIEW_PTR(1);
	SG_Camera *cam = SG_CAMERA_PTR(2);

	AG_ObjectLock(sv);
	sv->cam = cam;
	sv->flags |= SG_VIEW_UPDATE_PROJ;
	AG_ObjectUnlock(sv);
}

static void
PopupMenu(SG_View *_Nonnull sv, int x, int y)
{
	SG *sg = sv->sg;
	AG_PopupMenu *pm;
	AG_MenuItem *mOvl, *mCam;

	if (sv->pmView != NULL) {
		AG_PopupShowAt(sv->pmView, x, y);
		return;
	}
	if ((pm = sv->pmView = AG_PopupNew(sv)) == NULL)
		return;

	AG_MenuUintFlags(pm->root, _("Lighting"), sgIconLighting.s,
	    &sv->flags, SG_VIEW_NO_LIGHTING, 1);
	AG_MenuUintFlags(pm->root, _("Z-Buffer"), sgIconZBuffer.s,
	    &sv->flags, SG_VIEW_NO_DEPTH_TEST, 1);

	mOvl = AG_MenuNode(pm->root, _("Overlay"), NULL);
	{
		AG_MenuUintFlags(mOvl, _("Wireframe"),      sgIconWireframe.s,     &sg->flags, SG_OVERLAY_WIREFRAME, 0);
		AG_MenuUintFlags(mOvl, _("Vertices"),       sgIconVertices.s,      &sg->flags, SG_OVERLAY_VERTICES, 0);
		AG_MenuUintFlags(mOvl, _("Facet normals"),  sgIconFacetNormals.s,  &sg->flags, SG_OVERLAY_FNORMALS, 0);
		AG_MenuUintFlags(mOvl, _("Vertex normals"), sgIconVertexNormals.s, &sg->flags, SG_OVERLAY_VNORMALS, 0);
	}

	mCam = AG_MenuNode(pm->root, _("Switch to camera"), NULL);
	{
		AG_MenuItem *mi;
		SG_Camera *cam;

		OBJECT_FOREACH_CLASS(cam, sg->root, sg_camera,
		    "SG_Node:SG_Camera:*") {
			mi = AG_MenuAction(mCam, OBJECT(cam)->name, sgIconCamera.s,
			    ViewSwitchCamera, "%p,%p", sv, cam);
			mi->state = (cam != sv->cam);
		}
	}
	AG_PopupShowAt(pm, x,y);
}

/* Select node by mouse and initiate move instruction. */
static void
SelectByMouse(SG_View *_Nonnull sv, int x, int y)
{
	M_Vector4 pNear, pFar;
	M_Line3 ray;
	SG_Node *node, *nodeNearest = NULL;
	M_Real dNearest = M_INFINITY;
	M_Matrix44 T;

	/* Find point corresponding to x,y on the Near and Far planes */
	SG_ViewUnProject(sv, x, y, &pNear, &pFar);
	SG_GetNodeTransform(sv->cam, &T);
	M_MatMultVector44v(&pNear, &T);
	M_MatMultVector44v(&pFar, &T);

	/* Intersect ray with nodes in the scene. */
	SG_FOREACH_NODE(node, sv->sg) {
		M_Vector3 pRayNear, pRayFar;
		M_GeomSet3 S = M_GEOM_SET_EMPTY;
		M_Matrix44 T;
		M_Geom3 gt;
		M_Real d;
		M_Line3 ln;
		
		SG_GetNodeTransformInverse(node, &T);
		pRayNear = M_VecFromProj3(M_MatMultVector44(T,pNear));
		pRayFar = M_VecFromProj3(M_MatMultVector44(T,pFar));
		ray = M_LineFromPts3(pRayNear, pRayFar);
		gt.type = M_LINE;
		gt.g.line = ray;
		
		node->flags &= ~(SG_NODE_SELECTED);

		if (SG_Intersect(node, gt, &S) != 1) {
			continue;
		}
		switch (S.g[0].type) {
		case M_POINT:
			d = M_VecDistance3(S.g[0].g.point, pRayNear);
			if (d < dNearest) {
				dNearest = d;
				nodeNearest = node;
			}
			break;
		case M_LINE:
			ln = S.g[0].g.line;
			d = M_VecDistance3(M_LineInitPt3(ln), pRayNear);
			if (d < dNearest) {
				dNearest = d;
				nodeNearest = node;
			}
			d = M_VecDistance3(M_LineTermPt3(ln), pRayNear);
			if (d < dNearest) {
				dNearest = d;
				nodeNearest = node;
			}
			break;
		default:
			break;
		}
	}
	if (nodeNearest != NULL)
		nodeNearest->flags |= SG_NODE_SELECTED;
}

/* Timer for keyboard controlled camera rotation. */
static Uint32
CamRotateTimeout(AG_Timer *_Nonnull to, AG_Event *event)
{
	SG_View *sv = SG_VIEW_SELF();
	const SG_ViewCamAction *act = AG_PTR(1);
	const M_Real dir = (M_Real)AG_FLOAT(2);
	SG *sg = sv->sg;
	Uint32 rv = 0;

	AG_ObjectLock(sv);
	if (sv->cam == NULL) {
		goto out;
	}
	AG_ObjectLock(sg);
	SG_Rotatev(sv->cam, dir*act->incr, act->v);
	AG_ObjectUnlock(sg);
	rv = (to->ival > act->vMax) ?
	     (to->ival - act->vAccel) : to->ival;
	AG_Redraw(sv);
out:
	AG_ObjectUnlock(sv);
	return (rv);
}

/* Timer for keyboard controlled camera translation. */
static Uint32
CamMoveTimeout(AG_Timer *_Nonnull to, AG_Event *event)
{
	SG_View *sv = SG_VIEW_SELF();
	const SG_ViewCamAction *act = AG_PTR(1);
	const M_Real dir = (M_Real)AG_FLOAT(2);
	SG *sg = sv->sg;
	Uint32 rv = 0;
	
	AG_ObjectLock(sv);
	if (sv->cam == NULL) {
		goto out;
	}
	AG_ObjectLock(sg);
	SG_Translatev(sv->cam, M_VecScale3p(&act->v, dir*act->incr));
	AG_ObjectUnlock(sg);
	rv = (to->ival > act->vMax) ?
	     (to->ival - act->vAccel) : to->ival;
	AG_Redraw(sv);
out:
	AG_ObjectUnlock(sv);
	return (rv);
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	SG_View *sv = SG_VIEW_SELF();
	const int keysym = AG_INT(1);
	const int kmod = AG_INT(2);
	
	switch (keysym) {
	case AG_KEY_LEFT:
		if (kmod & AG_KEYMOD_CTRL) {
			AG_AddTimer(sv, &sv->toRot, sv->rot[0].vMin,
			    CamRotateTimeout, "%p,%f", &sv->rot[0], -1.0f);
		} else {
			AG_AddTimer(sv, &sv->toMove, sv->move[0].vMin,
			    CamMoveTimeout, "%p,%f", &sv->move[0], -1.0f);
		}
		break;
	case AG_KEY_RIGHT:
		if (kmod & AG_KEYMOD_CTRL) {
			AG_AddTimer(sv, &sv->toRot, sv->rot[0].vMin,
			    CamRotateTimeout, "%p,%f", &sv->rot[0], +1.0f);
		} else {
			AG_AddTimer(sv, &sv->toMove, sv->move[0].vMin,
			    CamMoveTimeout, "%p,%f", &sv->move[0], +1.0f);
		}
		break;
	case AG_KEY_UP:
		if (kmod & AG_KEYMOD_CTRL) {
			AG_AddTimer(sv, &sv->toRot, sv->rot[1].vMin,
			    CamRotateTimeout, "%p,%f", &sv->rot[1], -1.0f);
		} else {
			AG_AddTimer(sv, &sv->toMove, sv->move[1].vMin,
			    CamMoveTimeout, "%p,%f", &sv->move[1], +1.0f);
		}
		break;
	case AG_KEY_DOWN:
		if (kmod & AG_KEYMOD_CTRL) {
			AG_AddTimer(sv, &sv->toRot, sv->rot[1].vMin,
			    CamRotateTimeout, "%p,%f", &sv->rot[1], +1.0f);
		} else {
			AG_AddTimer(sv, &sv->toMove, sv->move[1].vMin,
			    CamMoveTimeout, "%p,%f", &sv->move[1], -1.0f);
		}
		break;
	case AG_KEY_PAGEUP:
		AG_AddTimer(sv, &sv->toMove, sv->move[2].vMin,
		    CamMoveTimeout, "%p,%f", &sv->move[2], -1.0f);
		break;
	case AG_KEY_PAGEDOWN:
		AG_AddTimer(sv, &sv->toMove, sv->move[2].vMin,
		    CamMoveTimeout, "%p,%f", &sv->move[2], +1.0f);
		break;
	case AG_KEY_DELETE:
		AG_AddTimer(sv, &sv->toRot, sv->rot[2].vMin,
		    CamRotateTimeout, "%p,%f", &sv->rot[2], -1.0f);
		break;
	case AG_KEY_END:
		AG_AddTimer(sv, &sv->toRot, sv->rot[2].vMin,
		    CamRotateTimeout, "%p,%f", &sv->rot[2], +1.0f);
		break;
	}
	sv->lastKeyDown = keysym;
}

static void
KeyUp(AG_Event *_Nonnull event)
{
	SG_View *sv = SG_VIEW_SELF();
	const int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_LEFT:
	case AG_KEY_RIGHT:
	case AG_KEY_UP:
	case AG_KEY_DOWN:
	case AG_KEY_PAGEUP:
	case AG_KEY_PAGEDOWN:
	case AG_KEY_DELETE:
	case AG_KEY_END:
		if (keysym == sv->lastKeyDown) {
			AG_DelTimer(sv, &sv->toRot);
			AG_DelTimer(sv, &sv->toMove);
		}
		break;
	}
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	SG_View *sv = SG_VIEW_SELF();
	const int button = AG_INT(1);
	const int x = AG_INT(2);
	const int y = AG_INT(3);

	if (!AG_WidgetIsFocused(sv)) {
		AG_WidgetFocus(sv);
	}
	switch (button) {
	case AG_MOUSE_WHEELUP:
		SG_CameraMoveMouse(sv->cam, sv, 0, 0, 1);
		break;
	case AG_MOUSE_WHEELDOWN:
		SG_CameraMoveMouse(sv->cam, sv, 0, 0, -1);
		break;
	case AG_MOUSE_RIGHT:
		PopupMenu(sv, x, y);
		break;
	case AG_MOUSE_LEFT:
		if ((AG_GetModState(sv) & AG_KEYMOD_CTRL) == 0) {
			SelectByMouse(sv, x,y);
		}
		break;
	}
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
/*	SG_View *sv = SG_VIEW_SELF(); */
	const int button = AG_INT(1);

	switch (button) {
	case AG_MOUSE_LEFT:
		break;
	default:
		break;
	}
}

/* Configure the active camera. */
void
SG_ViewSetCamera(SG_View *sv, SG_Camera *cam)
{
	AG_ObjectLock(sv);
	sv->cam = cam;
	sv->flags |= SG_VIEW_UPDATE_PROJ;
	AG_Redraw(sv);
	AG_ObjectUnlock(sv);
}

static void
OnShow(AG_Event *_Nonnull event)
{
	SG_View *sv = SG_VIEW_SELF();
	SG *sg = sv->sg;
	SG_Node *node;

	if (sg == NULL)
		return;

#ifdef AG_DEBUG
	Debug(sv, "Show: %s (%d x %d)\n", OBJECT(sg)->name,
	    WIDTH(sv), HEIGHT(sv));
#endif
	AG_ObjectLock(sg);
	SG_FOREACH_NODE(node, sg) {
		AG_PostEvent(node, "view-shown", "%p", sv);
	}
	AG_ObjectUnlock(sg);
}

static void
OnHide(AG_Event *_Nonnull event)
{
	SG_View *sv = SG_VIEW_SELF();
	SG *sg = sv->sg;
	SG_Node *node;

	if (sg == NULL)
		return;
	
#ifdef AG_DEBUG
	Debug(sv, "Hide: %s\n", OBJECT(sg)->name);
#endif
	AG_ObjectLock(sg);
	SG_FOREACH_NODE(node, sg) {
		AG_PostEvent(node, "view-hidden", "%p", sv);
	}
	AG_ObjectUnlock(sg);
}

static void
Init(void *_Nonnull obj)
{
	SG_View *sv = obj;
	int i;

	WIDGET(sv)->flags |= (AG_WIDGET_FOCUSABLE | AG_WIDGET_USE_OPENGL);

	sv->flags = 0;
	sv->transFlags = 0;
	sv->sg = NULL;
	sv->sgTrans = NULL;
	sv->transProgress = 1.0;
	sv->cam = NULL;
	sv->camTrans = NULL;
	sv->editArea = NULL;
	sv->editNode = NULL;
	AG_ColorBlack(&sv->transFadeColor);
	sv->transDuration = 500;
	sv->pmView = NULL;
	sv->pmNode = NULL;
	AG_InitTimer(&sv->toTransFade, "transFade", 0);
	AG_InitTimer(&sv->toRefresh, "refresh", 0);

	sv->rSens = M_VECTOR3(0.01, 0.01, 0.01);
	sv->tSens = M_VECTOR3(0.01, 0.01, 0.1);

	sv->rot[0].v = M_VecJ3();
	sv->rot[1].v = M_VecI3();
	sv->rot[2].v = M_VecK3();
	sv->move[0].v = M_VecI3();
	sv->move[1].v = M_VecJ3();
	sv->move[2].v = M_VecK3();
	for (i = 0; i < 3; i++) {
		sv->rot[i].vMin = 30;
		sv->rot[i].vAccel = 1;
		sv->rot[i].vMax = 10;
		sv->rot[i].incr = 0.02;
	}
	for (i = 0; i < 3; i++) {
		sv->move[i].vMin = 20;
		sv->move[i].vAccel = 1;
		sv->move[i].vMax = 5;
		sv->move[i].incr = 0.02;
	}
	sv->editStatus[0] = '\0';

	AG_AddEvent(sv, "widget-shown", OnShow, NULL);
	AG_AddEvent(sv, "widget-hidden", OnHide, NULL);
	AG_SetEvent(sv, "widget-reshape", OnReshape, NULL);
	AG_SetEvent(sv, "widget-overlay", OnOverlay, NULL);
	AG_SetEvent(sv, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(sv, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(sv, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(sv, "key-down", KeyDown, NULL);
	AG_SetEvent(sv, "key-up", KeyUp, NULL);

/*	AG_RedrawOnTick(sv, 1000); */
}

static void
Destroy(void *_Nonnull obj)
{
	SG_View *sv = obj;
	
	if (sv->pmView != NULL)
		AG_PopupDestroy(sv->pmView);
	if (sv->pmNode != NULL)
		AG_PopupDestroy(sv->pmNode);
}

AG_WidgetClass sgViewClass = {
	{
		"AG_Widget:SG_View",
		sizeof(SG_View),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
