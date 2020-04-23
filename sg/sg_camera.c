/*
 * Copyright (c) 2006-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Camera (viewpoint) node object.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

/* Create a new Camera node. */
SG_Camera *
SG_CameraNew(void *parent, const char *name)
{
	SG_Camera *cam;

	cam = Malloc(sizeof(SG_Camera));
	AG_ObjectInit(cam, &sgCameraClass);
	if (name) {
		AG_ObjectSetNameS(cam, name);
	} else {
		OBJECT(cam)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, cam);
	return (cam);
}

/* Create a new Camera node, copying parameters from an existing one. */
SG_Camera *
SG_CameraNewDuplicate(void *parent, const char *name, SG_Camera *cOrig)
{
	SG *sg = SGNODE(cOrig)->sg;
	SG_Camera *cNew;

	cNew = Malloc(sizeof(SG_Camera));
	AG_ObjectInit(cNew, &sgCameraClass);
	AG_ObjectSetNameS(cNew, name);
	
	if (sg) { AG_ObjectLock(sg); }
	AG_ObjectLock(cOrig);
	
	cNew->flags = cOrig->flags;
	cNew->pmode = cOrig->pmode;
	cNew->polyFace = cOrig->polyFace;
	cNew->polyBack = cOrig->polyBack;
	cNew->fov = cOrig->fov;
	cNew->aspect = cOrig->aspect;
	cNew->pNear = cOrig->pNear;
	cNew->pFar = cOrig->pFar;
	cNew->userProj[0] = cOrig->userProj[0];
	cNew->userProj[1] = cOrig->userProj[1];
	cNew->rotCtrl = cOrig->rotCtrl;
	cNew->focus[0] = cOrig->focus[0];
	cNew->focus[1] = cOrig->focus[1];
	SGNODE(cNew)->T = SGNODE(cOrig)->T;

	AG_ObjectAttach(parent, cNew);

	AG_ObjectUnlock(cOrig);
	if (sg) { AG_ObjectUnlock(sg); }
	return (cNew);
}

static void
Init(void *_Nonnull obj)
{
	SG_Camera *cam = obj;

	cam->flags = SG_CAMERA_DRAW;
	cam->pmode = SG_CAMERA_PERSPECTIVE;
	cam->aspect = 1.0;
	cam->pNear = 0.40;
	cam->pFar = 100.0;
	cam->rotSpeed = 0.1;
	cam->fov = Radians(60.0);
	cam->polyFace.mode = SG_CAMERA_SMOOTH_SHADED;
	cam->polyFace.cull = 0;
	cam->polyBack.mode = SG_CAMERA_WIREFRAME;
	cam->polyBack.cull = 0;
	cam->rotCtrl = SG_CAMERA_ROT_CIRCULAR;
	cam->focus[0] = NULL;
	cam->focus[1] = NULL;
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull buf, const AG_Version *_Nonnull ver)
{
	SG_Camera *cam = obj;
	
	cam->flags = (Uint)AG_ReadUint32(buf);
	cam->pmode = (enum sg_camera_pmode)AG_ReadUint8(buf);
	cam->fov = M_ReadReal(buf);
	cam->aspect = M_ReadReal(buf);
	cam->pNear = M_ReadReal(buf);
	cam->pFar = M_ReadReal(buf);
	cam->polyFace.mode = (int)AG_ReadUint8(buf);
	cam->polyFace.cull = (int)AG_ReadUint8(buf);
	cam->polyBack.mode = (int)AG_ReadUint8(buf);
	cam->polyBack.cull = (int)AG_ReadUint8(buf);
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull buf)
{
	SG_Camera *cam = obj;

	AG_WriteUint32(buf, (Uint32)cam->flags);
	AG_WriteUint8(buf, (Uint8)cam->pmode);
	M_WriteReal(buf, cam->fov);
	M_WriteReal(buf, cam->aspect);
	M_WriteReal(buf, cam->pNear);
	M_WriteReal(buf, cam->pFar);
	AG_WriteUint8(buf, (Uint8)cam->polyFace.mode);
	AG_WriteUint8(buf, (Uint8)cam->polyFace.cull);
	AG_WriteUint8(buf, (Uint8)cam->polyBack.mode);
	AG_WriteUint8(buf, (Uint8)cam->polyBack.cull);
	return (0);
}

/* Obtain view frustum coordinates (relative to camera node). */
void
SG_CameraFrustum(SG_Camera *cam, M_Rectangle3 *rNear, M_Rectangle3 *rFar)
{
	M_Vector3 cFar, cNear, r[4];
	M_Vector3 x, y;
	M_Real w, h;
	
	AG_ObjectLock(cam);

	if (rFar != NULL) {
		h = 2.0*Tan(cam->fov/2.0)*cam->pFar;
		w = h*cam->aspect;

		cFar = M_VecScale3(M_VecK3(), -cam->pFar);
		x = M_VecScale3(M_VecI3(), w/2);
		y = M_VecScale3(M_VecJ3(), h/2);
		r[0] = M_VecSub3(M_VecAdd3(cFar,x), y);
		r[1] = M_VecAdd3(M_VecAdd3(cFar,x), y);
		r[2] = M_VecAdd3(M_VecSub3(cFar,x), y);
		r[3] = M_VecSub3(M_VecSub3(cFar,x), y);
		*rFar = M_RectangleFromPts3(r[0], r[1], r[2], r[3]);
	}
	if (rNear != NULL) {
		h = 2.0*Tan(cam->fov/2.0)*cam->pNear;
		w = h*cam->aspect;

		cNear = M_VecScale3(M_VecK3(), -cam->pNear);
		x = M_VecScale3(M_VecI3(), w/2);
		y = M_VecScale3(M_VecJ3(), h/2);
		r[0] = M_VecSub3(M_VecAdd3(cNear,x), y);
		r[1] = M_VecAdd3(M_VecAdd3(cNear,x), y);
		r[2] = M_VecAdd3(M_VecSub3(cNear,x), y);
		r[3] = M_VecSub3(M_VecSub3(cNear,x), y);
		*rNear = M_RectangleFromPts3(r[0], r[1], r[2], r[3]);
	}
	
	AG_ObjectUnlock(cam);
}

/*
 * Multiply the current matrix by the camera's projection matrix
 * (mono rendering).
 */
void
SG_CameraProject(SG_Camera *cam)
{
	GLdouble yMin, yMax;

	switch (cam->pmode) {
	case SG_CAMERA_PERSPECTIVE:
		yMax = cam->pNear*Tan(cam->fov/2.0);
		yMin = -yMax;
		glFrustum(
		    yMin*cam->aspect, yMax*cam->aspect,
		    yMin, yMax, cam->pNear, cam->pFar);
		break;
	case SG_CAMERA_ORTHOGRAPHIC:
		{
			M_Matrix44 *Tcam;
			M_Real zoom;
			M_Vector3 t;
	
			AG_ObjectLock(SGNODE(cam)->sg);
			Tcam = &SGNODE(cam)->T;
			t.x = Tcam->m[0][3];
			t.y = Tcam->m[1][3];
			t.z = Tcam->m[2][3];
			AG_ObjectUnlock(SGNODE(cam)->sg);

			zoom = -Fabs(t.z);
			glOrtho(-1.0+zoom, 1.0-zoom, -1.0+zoom, 1.0-zoom,
			    -1.0/cam->pNear, cam->pFar);
		}
		break;
	case SG_CAMERA_USER_PROJ:
		GL_MultMatrixv(&cam->userProj[0]);
		break;
	}
}

/*
 * Multiply the current matrix by the camera's projection matrix
 * (stereo rendering, left eye).
 */
void
SG_CameraProjectLeft(SG_Camera *cam)
{
	GLdouble yMin, yMax;

	switch (cam->pmode) {
	case SG_CAMERA_PERSPECTIVE:
		yMax = cam->pNear*Tan(cam->fov/2.0);
		yMin = -yMax;
		glFrustum(
		    yMin*cam->aspect, yMax*cam->aspect,
		    yMin, yMax,
		    cam->pNear, cam->pFar);
		GL_Translate(M_VECTOR3(-sgEyeSeparation, 0.0, 0.0));
		break;
	case SG_CAMERA_ORTHOGRAPHIC:
		SG_CameraProject(cam);
		GL_Translate(M_VECTOR3(-sgEyeSeparation, 0.0, 0.0));
		break;
	case SG_CAMERA_USER_PROJ:
		GL_MultMatrixv(&cam->userProj[0]);
		break;
	}
}

/*
 * Multiply the current matrix by the camera's projection matrix
 * (stereo rendering, right eye).
 */
void
SG_CameraProjectRight(SG_Camera *cam)
{
	GLdouble yMin, yMax;

	switch (cam->pmode) {
	case SG_CAMERA_PERSPECTIVE:
		yMax = cam->pNear*Tan(cam->fov/2.0);
		yMin = -yMax;
		glFrustum(
		    yMin*cam->aspect, yMax*cam->aspect,
		    yMin, yMax,
		    cam->pNear, cam->pFar);
		GL_Translate(M_VECTOR3(+sgEyeSeparation, 0.0, 0.0));
		break;
	case SG_CAMERA_ORTHOGRAPHIC:
		SG_CameraProject(cam);
		GL_Translate(M_VECTOR3(+sgEyeSeparation, 0.0, 0.0));
		break;
	case SG_CAMERA_USER_PROJ:
		GL_MultMatrixv(&cam->userProj[1]);
		break;
	}
}

/*
 * Apply the viewing transformation for a camera and set up for
 * rendering.
 *
 * Must be called from widget draw context; SG and SG_Camera object
 * must be locked.
 */
void
SG_CameraSetup(SG_Camera *cam)
{
	M_Matrix44 T;

	SG_GetNodeTransformInverse(cam, &T);
	M_MatTranspose44v(&T);			/* OpenGL is column-major */
	GL_MultMatrixv(&T);
#if 1
	{
		static M_Real iRot = 0.0, jRot = 0.0, kRot = 0.0;

		if (cam->flags & SG_CAMERA_ROT_I) {
			glRotatef(iRot, 1.0, 0.0, 0.0);
			iRot += cam->rotSpeed;
		}
		if (cam->flags & SG_CAMERA_ROT_J) {
			glRotatef(jRot, 0.0, 1.0, 0.0);
			jRot += cam->rotSpeed;
		}
		if (cam->flags & SG_CAMERA_ROT_K) {
			glRotatef(kRot, 0.0, 0.0, 1.0);
			kRot += cam->rotSpeed;
		}
	}
#endif
	if (cam->polyFace.cull && cam->polyBack.cull) {
		GL_Enable(GL_CULL_FACE);
		glCullFace(GL_FRONT_AND_BACK);
	} else if (cam->polyFace.cull) {
		GL_Enable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	} else if (cam->polyBack.cull) {
		GL_Enable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	} else {
		GL_Disable(GL_CULL_FACE);
	}

	switch (cam->polyFace.mode) {
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

	switch (cam->polyBack.mode) {
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
}

/* Return the projection matrix for a camera. XXX */
void
SG_CameraGetProjection(SG_Camera *cam, M_Matrix44 *M)
{
	AG_ObjectLock(cam);

	GL_PushAttrib(GL_TRANSFORM_BIT);
	GL_MatrixMode(GL_PROJECTION);
	GL_PushMatrix();

	GL_LoadIdentity();
	SG_CameraProject(cam);
	GL_FetchMatrixv(GL_PROJECTION_MATRIX, M);
	M_MatTranspose44v(M);

	GL_PopMatrix();
	GL_PopAttrib();
	
	AG_ObjectUnlock(cam);
}

/*
 * Configure a perspective matrix from a given field of view and aspect
 * ratio.
 */
void
SG_CameraSetPerspective(SG_Camera *cam, M_Real fov, M_Real aspect)
{
	AG_ObjectLock(cam);
	cam->pmode = SG_CAMERA_PERSPECTIVE;
	cam->fov = fov;
	cam->aspect = aspect;
	AG_ObjectUnlock(cam);
}

/* Set an orthographic projection matrix. */
void
SG_CameraSetOrthographic(SG_Camera *cam)
{
	AG_ObjectLock(cam);
	cam->pmode = SG_CAMERA_ORTHOGRAPHIC;
	AG_ObjectUnlock(cam);
}

/* Set the z position of the near and far clipping planes. */
void
SG_CameraSetClipPlanes(SG_Camera *cam, M_Real pNear, M_Real pFar)
{
	AG_ObjectLock(cam);
	cam->pNear = pNear;
	cam->pFar = pFar;
	AG_ObjectUnlock(cam);
}

/* Set a user-specified projection matrix. */
void
SG_CameraSetUser(SG_Camera *cam, const M_Matrix44 *Pleft,
    const M_Matrix44 *Pright)
{
	AG_ObjectLock(cam);
	cam->userProj[0] = M_MatTranspose44p(Pleft);
	cam->userProj[1] = M_MatTranspose44p(Pright);
	AG_ObjectUnlock(cam);
}

/* Set the rendering mode for back-facing polygons. */
void
SG_CameraSetBackPolyMode(SG_Camera *cam, const SG_CameraPolyMode *pm)
{
	AG_ObjectLock(cam);
	cam->polyBack = *pm;
	AG_ObjectUnlock(cam);
}

/* Set the rendering mode for front-facing polygons. */
void
SG_CameraSetFacePolyMode(SG_Camera *cam, const SG_CameraPolyMode *pm)
{
	AG_ObjectLock(cam);
	cam->polyFace = *pm;
	AG_ObjectUnlock(cam);
}

static void
UpdateProjection(AG_Event *_Nonnull event)
{
	SG_View *sv = AG_PTR(1);

	AG_ObjectLock(sv);
	sv->flags |= SG_VIEW_UPDATE_PROJ;
	AG_WidgetUpdate(sv);
	AG_Redraw(sv);
	AG_ObjectUnlock(sv);
}

static void *_Nullable
Edit(void *_Nonnull obj, SG_View *_Nullable sgv)
{
	SG_Camera *cam = obj;
	AG_Mutex *lock = &OBJECT(cam)->lock;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_Radio *rad;
	const char *rasModes[] = {
		N_("Points"),
		N_("Wireframe"),
		N_("Flat shaded"),
		N_("Smooth shaded"),
		NULL
	};
	AG_Checkbox *cb;

	nb = AG_NotebookNew(NULL, AG_NOTEBOOK_EXPAND);
	ntab = AG_NotebookAdd(nb, _("Proj"), AG_BOX_VERT);
	{
		const char *projModes[] = {
			N_("Perspective"),
			N_("Orthographic"),
			NULL
		};
		AG_Box *vbox, *sbox;

		sbox = AG_BoxNewVert(ntab, AG_BOX_EXPAND);
		
		cb = AG_CheckboxNew(sbox, 0, _("Draw Camera Object"));
		AG_BindFlagMp(cb, "state", &cam->flags, SG_CAMERA_DRAW, lock);

		rad = AG_RadioNew(sbox, 0, projModes);
		if (sgv != NULL) {
			AG_BindUintMp(rad, "value", (Uint *)&cam->pmode, lock);
			AG_SetEvent(rad, "radio-changed",
			    UpdateProjection, "%p", sgv);
		} else {
			AG_WidgetDisable(rad);
		}
	
		AG_SeparatorNewHoriz(sbox);
		
		vbox = AG_BoxNewVert(sbox, AG_BOX_HFILL);
		{
			AG_Numerical *n[4];
			int i;

			n[0] = AG_NumericalNew(vbox, 0, "deg", _("Field of View: "));
			M_BindRealMp(n[0], "value", &cam->fov, lock);

			n[1] = AG_NumericalNew(vbox, 0, NULL, _("Aspect Ratio: "));
			M_BindRealMp(n[1], "value", &cam->aspect, lock);
			M_SetReal(n[1], "inc", 0.1);

			n[2] = AG_NumericalNew(vbox, 0, NULL, _("Near Plane: "));
			M_BindRealMp(n[2], "value", &cam->pNear, lock);
			M_SetReal(n[2], "inc", 0.05);

			n[3] = AG_NumericalNew(vbox, 0, NULL, _("Far Plane: "));
			M_BindRealMp(n[3], "value", &cam->pFar, lock);
			M_SetReal(n[3], "inc", 0.05);

			for (i = 0; i < 4; i++) {
				if (sgv != NULL) {
					AG_SetEvent(n[i], "numerical-changed",
					    UpdateProjection, "%p", sgv);
				} else {
					AG_WidgetDisable(n[i]);
				}
			}
		}
	}

	ntab = AG_NotebookAdd(nb, _("Pos"), AG_BOX_VERT);
	{
		M_EditTranslate3Mp(ntab, _("Eye coordinates: "),
		    &SGNODE(cam)->T, lock);
	}

	ntab = AG_NotebookAdd(nb, _("Polygons"), AG_BOX_VERT);
	{
		AG_LabelNewS(ntab, 0, _("Front-facing:"));

		rad = AG_RadioNew(ntab, 0, rasModes);
		AG_BindUintMp(rad, "value", (Uint *)&cam->polyFace.mode, lock);

		cb = AG_CheckboxNew(ntab, 0, _("Cull all"));
		AG_BindIntMp(cb, "value", &cam->polyFace.cull, lock);
		
		AG_SeparatorNewHoriz(ntab);
		
		AG_LabelNewS(ntab, 0, _("Back-facing:"));
		rad = AG_RadioNew(ntab, 0, rasModes);
		AG_BindUintMp(rad, "value", (Uint *)&cam->polyBack.mode, lock);

		cb = AG_CheckboxNew(ntab, 0, _("Cull all"));
		AG_BindIntMp(cb, "value", &cam->polyBack.cull, lock);
	}

	ntab = AG_NotebookAdd(nb, _("Rotation"), AG_BOX_VERT);
	{
		const char *rotModes[] = {
			N_("Ignore"),
			N_("Circular path"),
			N_("Elliptic path"),
			NULL
		};

		rad = AG_RadioNew(ntab, 0, rotModes);
		AG_BindUintMp(rad, "value", (Uint *)&cam->rotCtrl, lock);
#if 0
		AG_LabelNewPolled(ntab, AG_LABEL_HFILL, "Center: %s",
		    AGOBJECT(&cam->focus[0])->name);
#endif
	}
	return (nb);
}

#ifdef AG_DEBUG
static void
SetRotationSpeed(AG_Event *_Nonnull event)
{
	SG_Camera *cam = AG_PTR(1);
	M_Real speed = AG_FLOAT(2);

	AG_ObjectLock(cam);
	cam->rotSpeed = speed;
	AG_ObjectUnlock(cam);
}
#endif /* AG_DEBUG */

static void
MenuInstance(void *_Nonnull obj, AG_MenuItem *_Nonnull m, SG_View *_Nonnull sgv)
{
//	SG_Camera *cam = obj;

//	AG_MenuAction(m, _("Camera parameters..."), sgIconCamera.s,
//	    EditCameraParams, "%p,%p", cam, sgv);
#ifdef AG_DEBUG
	{
		AG_MenuItem *mRot;
		SG_Camera *cam = obj;
		AG_Mutex *lock = &OBJECT(cam)->lock;

		mRot = AG_MenuNode(m, _("Artificial rotation"), NULL);

		AG_MenuUintFlagsMp(mRot, _("Rotate around i"), sgIconI.s,
		    &cam->flags, SG_CAMERA_ROT_I, 0, lock);
		AG_MenuUintFlagsMp(mRot, _("Rotate around j"), sgIconJ.s,
		    &cam->flags, SG_CAMERA_ROT_J, 0, lock);
		AG_MenuUintFlagsMp(mRot, _("Rotate around k"), sgIconK.s,
		    &cam->flags, SG_CAMERA_ROT_K, 0, lock);

		AG_MenuSeparator(mRot);

		AG_MenuAction(mRot, _("Invert direction"), NULL,
		    SetRotationSpeed, "%p,%f", cam, -cam->rotSpeed);

		AG_MenuSeparator(mRot);

		AG_MenuAction(mRot, _("Very slow"), NULL,
		    SetRotationSpeed, "%p,%f", cam, 0.1);
		AG_MenuAction(mRot, _("Slow"), NULL,
		    SetRotationSpeed, "%p,%f", cam, 0.2);
		AG_MenuAction(mRot, _("Medium"), NULL,
		    SetRotationSpeed, "%p,%f", cam, 1.0);
		AG_MenuAction(mRot, _("Fast"), NULL,
		    SetRotationSpeed, "%p,%f", cam, 3.0);
		AG_MenuAction(mRot, _("Very fast"), NULL,
		    SetRotationSpeed, "%p,%f", cam, 6.0);
	}
#endif /* AG_DEBUG */
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull sgv)
{
	M_Vector3 v[4], vFoc;
	SG_Camera *cam = obj;
	M_Rectangle3 rNear, rFar;

	if (cam == sgv->cam || !(cam->flags & SG_CAMERA_DRAW))
		return;

	v[0] = M_VECTOR3(-0.5, -0.5, 0.0);
	v[1] = M_VECTOR3(+0.5, -0.5, 0.0);
	v[2] = M_VECTOR3(+0.5, +0.5, 0.0);
	v[3] = M_VECTOR3(-0.5, +0.5, 0.0);
	vFoc = M_VECTOR3(0.0, 0.0, -1.0);

	GL_PushAttrib(GL_LINE_BIT);

	GL_Begin(GL_QUADS);
/*	GL_Color3ub(150, 0, 0); */
	GL_Vertex3v(&v[3]);
	GL_Vertex3v(&v[2]);
	GL_Vertex3v(&v[1]);
	GL_Vertex3v(&v[0]);
	GL_End();

	GL_Begin(GL_QUADS);
/*	GL_Color3ub(0, 0, 150); */
	GL_Vertex3v(&v[0]);
	GL_Vertex3v(&v[1]);
	GL_Vertex3v(&v[2]);
	GL_Vertex3v(&v[3]);
	GL_End();

	GL_Begin(GL_LINES);
/*	GL_Color3ub(0, 0, 0); */
	GL_Vertex3v(&v[0]);
	GL_Vertex3v(&vFoc);
	GL_Vertex3v(&v[1]);
	GL_Vertex3v(&vFoc);
	GL_Vertex3v(&v[2]);
	GL_Vertex3v(&vFoc);
	GL_Vertex3v(&v[3]);
	GL_Vertex3v(&vFoc);
	GL_End();
	
	GL_LineStipple(1, 0x00ff);
	GL_Enable(GL_LINE_STIPPLE);
	
	SG_CameraFrustum(cam, &rNear, &rFar);
	GL_Begin(GL_LINE_LOOP);
/*	GL_Color3ub(0, 0, 80); */
	GL_Vertex3v(&rNear.a);
	GL_Vertex3v(&rNear.b);
	GL_Vertex3v(&rNear.c);
	GL_Vertex3v(&rNear.d);
	GL_End();
	GL_Begin(GL_LINE_LOOP);
/*	GL_Color3ub(80, 0, 80); */
	GL_Vertex3v(&rFar.a);
	GL_Vertex3v(&rFar.b);
	GL_Vertex3v(&rFar.c);
	GL_Vertex3v(&rFar.d);
	GL_End();

	GL_PopAttrib();
}

/* Set circular rotation control. */
void
SG_CameraSetRotCtrlCircular(SG_Camera *cam, SG_Node *focus)
{
	AG_ObjectLock(cam);
	cam->rotCtrl = SG_CAMERA_ROT_CIRCULAR;
	cam->focus[0] = focus;
	AG_ObjectUnlock(cam);
}

/* Set elliptic rotation control. */
void
SG_CameraSetRotCtrlElliptic(SG_Camera *cam, SG_Node *foc1, SG_Node *foc2)
{
	AG_ObjectLock(cam);
	cam->rotCtrl = SG_CAMERA_ROT_ELLIPTIC;
	cam->focus[0] = foc1;
	cam->focus[1] = foc2;
	AG_ObjectUnlock(cam);
}

/* Rotate the camera by mouse motion. */
void
SG_CameraRotMouse(SG_Camera *cam, SG_View *sv, int x, int y)
{
	M_Real iRot, jRot;

	AG_ObjectLock(cam);
	AG_ObjectLock(sv->sg);

	switch (cam->rotCtrl) {
	case SG_CAMERA_ROT_CIRCULAR:
		iRot = sv->rSens.y*(M_Real)y;
		jRot = sv->rSens.x*(M_Real)x;
		if (cam->focus[0] == NULL || cam->focus[0] == SGNODE(cam)) {
			SG_Rotatev(cam, iRot, M_VecI3());
			SG_Rotatev(cam, jRot, M_VecJ3());
		} else {
			M_Vector3 vFocus = SG_NodePos(cam->focus[0]);

			SG_Orbitv(cam, vFocus, M_VecI3(), iRot);
			SG_Orbitv(cam, vFocus, M_VecJ3(), jRot);
		}
		break;
	default:
		break;
	}
	
	AG_Redraw(sv);
	AG_ObjectUnlock(sv->sg);
	AG_ObjectUnlock(cam);
}

/* Move the camera by mouse motion. */
void
SG_CameraMoveMouse(SG_Camera *cam, SG_View *sv, int xrel, int yrel, int zrel)
{
	M_Vector3 m;

	m.x = sv->tSens.x*(-(M_Real)xrel);
	m.y = sv->tSens.y*((M_Real)yrel);
	m.z = sv->tSens.z*(-(M_Real)zrel);

	AG_ObjectLock(sv->sg);
	AG_ObjectLock(cam);
#if 0
	/* Translate along the global axis, not the camera axis. */
	T->m[0][3] += m.x;
	T->m[1][3] += m.y;
	T->m[2][3] += m.z;
#else
	/* Translate along the local camera axis. */
	M_MatTranslate44v(&SGNODE(cam)->T, m);
#endif
	AG_ObjectUnlock(cam);
	if (cam->pmode == SG_CAMERA_ORTHOGRAPHIC) {
		sv->flags |= SG_VIEW_UPDATE_PROJ;
	}
	AG_Redraw(sv);
	AG_ObjectUnlock(sv->sg);
}

static int
ScriptAction(void *_Nonnull obj, SG_Action *_Nonnull act, int invert)
{
	SG_Camera *cam = obj;

	switch (act->type) {
	case SG_ACTION_MOVE:
	case SG_ACTION_ZMOVE:
		SG_Translatev(cam,
		    invert ? M_VecFlip3(act->act_move) : act->act_move);
		break;
	case SG_ACTION_ROTATE:
		SG_Rotatev(cam,
		    invert ? -act->act_rotate.theta : +act->act_rotate.theta,
		    act->act_rotate.axis);
		break;
	case SG_ACTION_SCALE:
		break;
	default:
		return (0);
	}
	return (1);
}

SG_NodeClass sgCameraClass = {
	{
		"SG_Node:SG_Camera",
		sizeof(SG_Camera),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		Load,
		Save,
		SG_NodeEdit
	},
	MenuInstance,
	NULL,			/* menuClass */
	Draw,
	NULL,			/* intersect */
	Edit,
	NULL,
	ScriptAction
};
