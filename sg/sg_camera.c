/*
 * Copyright (c) 2006-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include <gui/notebook.h>
#include <gui/numerical.h>
#include <gui/radio.h>
#include <gui/checkbox.h>
#include <gui/box.h>
#include <gui/separator.h>
#include <gui/label.h>
#include <gui/menu.h>

#include "sg.h"
#include "sg_gui.h"

const AG_Version sgCameraVer = { 0, 0 };

SG_Camera *
SG_CameraNew(void *pnode, const char *name)
{
	SG_Camera *cam;

	cam = Malloc(sizeof(struct sg_camera), M_SG);
	SG_CameraInit(cam, name);
	SG_NodeAttach(pnode, cam);
	return (cam);
}

void
SG_CameraInit(void *p, const char *name)
{
	SG_Camera *cam = p;

	SG_NodeInit(cam, name, &sgCameraOps, 0);
	cam->flags = 0;
	cam->pmode = SG_CAMERA_PERSPECTIVE;
	cam->aspect = 1.0;
	cam->zNear = 0.001;
	cam->zFar = 300.0;
	cam->fovY = SG_Radians(60.0);
	cam->polyFace.mode = SG_CAMERA_SMOOTH_SHADED;
	cam->polyFace.cull = 0;
	cam->polyBack.mode = SG_CAMERA_WIREFRAME;
	cam->polyBack.cull = 0;
#ifdef DEBUG
	cam->rotSpeed = 0.1;
#endif
	cam->rotCtrl = SG_CAMERA_ROT_CIRCULAR;
	cam->focus[0] = NULL;
	cam->focus[1] = NULL;
}

int
SG_CameraLoad(void *p, AG_Netbuf *buf)
{
	SG_Camera *cam = p;
	
	if (AG_ReadVersion(buf, "SG_Camera", &sgCameraVer, NULL) != 0)
		return (-1);

	cam->flags = (Uint)AG_ReadUint32(buf);
	cam->pmode = (enum sg_camera_pmode)AG_ReadUint8(buf);
	cam->fovY = SG_ReadReal(buf);
	cam->aspect = SG_ReadReal(buf);
	cam->zNear = SG_ReadReal(buf);
	cam->zFar = SG_ReadReal(buf);
	cam->polyFace.mode = (int)AG_ReadUint8(buf);
	cam->polyFace.cull = (int)AG_ReadUint8(buf);
	cam->polyBack.mode = (int)AG_ReadUint8(buf);
	cam->polyBack.cull = (int)AG_ReadUint8(buf);
	return (0);
}

int
SG_CameraSave(void *p, AG_Netbuf *buf)
{
	SG_Camera *cam = p;

	AG_WriteVersion(buf, "SG_Camera", &sgCameraVer);
	AG_WriteUint32(buf, (Uint32)cam->flags);
	AG_WriteUint8(buf, (Uint8)cam->pmode);
	SG_WriteReal(buf, cam->fovY);
	SG_WriteReal(buf, cam->aspect);
	SG_WriteReal(buf, cam->zNear);
	SG_WriteReal(buf, cam->zFar);

	AG_WriteUint8(buf, (Uint8)cam->polyFace.mode);
	AG_WriteUint8(buf, (Uint8)cam->polyFace.cull);
	AG_WriteUint8(buf, (Uint8)cam->polyBack.mode);
	AG_WriteUint8(buf, (Uint8)cam->polyBack.cull);
	return (0);
}

/* Multiply the current matrix by the camera's projection matrix. */
void
SG_CameraProject(SG_Camera *cam)
{
	GLdouble yMin, yMax;

	AG_LockGL();
	switch (cam->pmode) {
	case SG_CAMERA_PERSPECTIVE:
		yMax = cam->zNear*SG_Tan(cam->fovY/2.0);
		yMin = -yMax;
		glFrustum(yMin*cam->aspect, yMax*cam->aspect,
		    yMin, yMax,
		    cam->zNear, cam->zFar);
		break;
	case SG_CAMERA_ORTHOGRAPHIC:
		{
			SG_Real zoom;
			SG_Vector t;

			SG_MatrixGetTranslation(&SGNODE(cam)->T, &t);
			zoom = -SG_Fabs(t.z);
			glOrtho(-1.0+zoom, 1.0-zoom, -1.0+zoom, 1.0-zoom,
			    -1.0/cam->zNear, cam->zFar);
		}
		break;
	case SG_CAMERA_USER_PROJ:
		SG_MultMatrixGL(&cam->userProj);
		break;
	}
	AG_UnlockGL();
}

/*
 * Apply the viewing transformation for a camera.
 * Must be called from widget draw context.
 */
void
SG_CameraSetup(SG_Camera *cam)
{
	SG_Matrix T;

	SG_GetNodeTransformInverse(cam, &T);
	MatTransposev(&T);			/* OpenGL is column-major */
	SG_MultMatrixGL(&T);

#ifdef DEBUG
	{
		static SG_Real iRot = 0.0, jRot = 0.0, kRot = 0.0;

		if (cam->flags & SG_CAMERA_ROT_I) {
			SG_RotateVecGL(iRot, VecI());
			iRot += cam->rotSpeed;
		}
		if (cam->flags & SG_CAMERA_ROT_J) {
			SG_RotateVecGL(jRot, VecJ());
			jRot += cam->rotSpeed;
		}
		if (cam->flags & SG_CAMERA_ROT_K) {
			SG_RotateVecGL(kRot, VecK());
			kRot += cam->rotSpeed;
		}
	}
#endif
}

/* Return the projection matrix for a camera. */
void
SG_CameraGetProjection(SG_Camera *cam, SG_Matrix *M)
{
	AG_LockGL();
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	SG_CameraProject(cam);
	SG_GetMatrixGL(GL_PROJECTION_MATRIX, M);
	MatTransposev(M);
	glPopMatrix();
	glPopAttrib();
	AG_UnlockGL();
}

void
SG_CameraSetPerspective(SG_Camera *cam, SG_Real fovY, SG_Real aspect)
{
	cam->pmode = SG_CAMERA_PERSPECTIVE;
	cam->fovY = fovY;
	cam->aspect = aspect;
}
	
void
SG_CameraSetOrthographic(SG_Camera *cam)
{
	cam->pmode = SG_CAMERA_ORTHOGRAPHIC;
}

void
SG_CameraSetClipPlanes(SG_Camera *cam, SG_Real zNear, SG_Real zFar)
{
	cam->zNear = zNear;
	cam->zFar = zFar;
}

void
SG_CameraSetUser(SG_Camera *cam, const SG_Matrix *P)
{
	cam->userProj = MatTransposep(P);
}

static void
UpdateProjection(AG_Event *event)
{
	SG_View *sv = AG_PTR(1);
	AG_Window *pWin;

	SG_ViewUpdateProjection(sv);
	if ((pWin = AG_WidgetParentWindow(sv)) != NULL)
		AG_WindowUpdate(pWin);
}

void
SG_CameraEdit(void *p, AG_Widget *box, SG_View *sgv)
{
	SG_Camera *cam = p;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_Numerical *num;
	AG_Radio *rad;
	AG_Checkbox *cb;
	const char *rasModes[] = {
		N_("Points"),
		N_("Wireframe"),
		N_("Flat shaded"),
		N_("Smooth shaded"),
		NULL
	};

	nb = AG_NotebookNew(box, AG_NOTEBOOK_EXPAND);
	ntab = AG_NotebookAddTab(nb, _("Proj"), AG_BOX_VERT);
	{
		const char *projModes[] = {
			N_("Perspective"),
			N_("Orthographic"),
			NULL
		};
		AG_Box *vbox, *sbox;

		sbox = AG_BoxNewVert(ntab, AG_BOX_EXPAND);

		rad = AG_RadioNew(sbox, 0, projModes);
		AG_WidgetBindInt(rad, "value", &cam->pmode);
		AG_SetEvent(rad, "radio-changed", UpdateProjection, "%p", sgv);
	
		AG_SeparatorNewHoriz(sbox);
		
		vbox = AG_BoxNewVert(sbox, AG_BOX_HFILL);
		{
			num = AG_NumericalNew(vbox, 0, "deg",
			    _("Field of View: "));
			SG_WidgetBindReal(num, "value", &cam->fovY);
			AG_SetEvent(num, "numerical-changed",
			    UpdateProjection, "%p", sgv);

			num = AG_NumericalNew(vbox, 0, NULL,
			    _("Aspect Ratio: "));
			AG_NumericalSetIncrement(num, 0.1);
			SG_WidgetBindReal(num, "value", &cam->aspect);
			AG_SetEvent(num, "numerical-changed",
			    UpdateProjection, "%p", sgv);

			num = AG_NumericalNew(vbox, 0, NULL, _("Near Plane: "));
			SG_WidgetBindReal(num, "value", &cam->zNear);
			AG_SetEvent(num, "numerical-changed",
			    UpdateProjection, "%p", sgv);

			num = AG_NumericalNew(vbox, 0, NULL, _("Far Plane: "));
			SG_WidgetBindReal(num, "value", &cam->zFar);
			AG_SetEvent(num, "numerical-changed",
			    UpdateProjection, "%p", sgv);
		}
	}
	ntab = AG_NotebookAddTab(nb, _("Pos"), AG_BOX_VERT);
	{
		SG_EditTranslate3(ntab, _("Eye coordinates: "),
		    &SGNODE(cam)->T);
	}
	ntab = AG_NotebookAddTab(nb, _("Polygons"), AG_BOX_VERT);
	{
		AG_LabelNewStaticString(ntab, 0, _("Front-facing:"));
		rad = AG_RadioNew(ntab, 0, rasModes);
		AG_WidgetBindInt(rad, "value", &cam->polyFace.mode);
		cb = AG_CheckboxNew(ntab, 0, _("Cull all"));
		AG_WidgetBindInt(cb, "state", &cam->polyFace.cull);
		
		AG_SeparatorNewHoriz(ntab);
		
		AG_LabelNewStaticString(ntab, 0, _("Back-facing:"));
		rad = AG_RadioNew(ntab, 0, rasModes);
		AG_WidgetBindInt(rad, "value", &cam->polyBack.mode);
		cb = AG_CheckboxNew(ntab, 0, _("Cull all"));
		AG_WidgetBindInt(cb, "state", &cam->polyBack.cull);
	}
	ntab = AG_NotebookAddTab(nb, _("Rotation"), AG_BOX_VERT);
	{
		const char *rotModes[] = {
			N_("Ignore"),
			N_("Circular path"),
			N_("Elliptic path"),
			NULL
		};

		rad = AG_RadioNew(ntab, 0, rotModes);
		AG_WidgetBindInt(rad, "value", &cam->rotCtrl);

		AG_LabelNewPolled(ntab, AG_LABEL_HFILL,
		    "Center: %s", &cam->focus[0]->name);
	}
}

#ifdef DEBUG
static void
SetRotationSpeed(AG_Event *event)
{
	SG_Camera *cam = AG_PTR(1);
	SG_Real speed = AG_FLOAT(2);

	cam->rotSpeed = speed;
}
#endif

void
SG_CameraMenu(void *obj, AG_MenuItem *m, SG_View *sgv)
{
//	SG_Camera *cam = obj;

//	AG_MenuAction(m, _("Camera parameters..."), OBJEDIT_ICON,
//	    EditCameraParams, "%p,%p", cam, sgv);
#ifdef DEBUG

	{
		AG_MenuItem *mRot;
		SG_Camera *cam = obj;

		mRot = AG_MenuNode(m, _("Artificial rotation"), ANIM_PLAY_ICON);
		AG_MenuUintFlags(mRot, _("Rotate around i"), RIGHT_ARROW_ICON,
		    &cam->flags, SG_CAMERA_ROT_I, 0);
		AG_MenuUintFlags(mRot, _("Rotate around j"), RIGHT_ARROW_ICON,
		    &cam->flags, SG_CAMERA_ROT_J, 0);
		AG_MenuUintFlags(mRot, _("Rotate around k"), RIGHT_ARROW_ICON,
		    &cam->flags, SG_CAMERA_ROT_K, 0);
		AG_MenuSeparator(mRot);
		AG_MenuAction(mRot, _("Invert direction"), LEFT_ARROW_ICON,
		    SetRotationSpeed, "%p,%f", cam, -cam->rotSpeed);
		AG_MenuSeparator(mRot);
		AG_MenuAction(mRot, _("Very slow"), ANIM_PLAY_ICON,
		    SetRotationSpeed, "%p,%f", cam, 0.1);
		AG_MenuAction(mRot, _("Slow"), ANIM_PLAY_ICON,
		    SetRotationSpeed, "%p,%f", cam, 0.2);
		AG_MenuAction(mRot, _("Medium"), ANIM_PLAY_ICON,
		    SetRotationSpeed, "%p,%f", cam, 1.0);
		AG_MenuAction(mRot, _("Fast"), ANIM_PLAY_ICON,
		    SetRotationSpeed, "%p,%f", cam, 3.0);
		AG_MenuAction(mRot, _("Very fast"), ANIM_PLAY_ICON,
		    SetRotationSpeed, "%p,%f", cam, 6.0);
	}
#endif
}

void
SG_CameraDraw(void *cam, SG_View *sgv)
{
	GLUquadricObj *qo;

	if (cam == sgv->cam)
		return;

	qo = (GLUquadricObj *)gluNewQuadric();
	gluQuadricDrawStyle(qo, GLU_FILL);
	gluQuadricNormals(qo, GLU_SMOOTH);
	gluCylinder(qo, 0.0, 1.0, 3.0, 8, 4);
	gluDeleteQuadric(qo);
}

SG_Vector
SG_CameraVector(SG_Camera *cam)
{
	SG_Vector v = VecK();		/* Rotate k by convention. */
	SG_Matrix T;

	SG_GetNodeTransform(cam, &T);
	MatMultVectorv(&v, &T);
	VecNormv(&v);
	return (v);
}

void
SG_CameraSetRotCtrlCircular(SG_Camera *cam, SG_Node *focus)
{
	cam->rotCtrl = SG_CAMERA_ROT_CIRCULAR;
	cam->focus[0] = focus;
}

void
SG_CameraSetRotCtrlElliptic(SG_Camera *cam, SG_Node *foc1, SG_Node *foc2)
{
	cam->rotCtrl = SG_CAMERA_ROT_ELLIPTIC;
	cam->focus[0] = foc1;
	cam->focus[1] = foc2;
}

void
SG_CameraRotMouse(SG_Camera *cam, SG_View *sv, int x, int y)
{
	SG_Real iRot, jRot;

	switch (cam->rotCtrl) {
	case SG_CAMERA_ROT_CIRCULAR:
		iRot = sv->mouse.rsens.y*(SG_Real)y;
		jRot = sv->mouse.rsens.x*(SG_Real)x;
		if (cam->focus[0] == NULL || cam->focus[0] == SGNODE(cam)) {
			SG_Rotatev(cam, iRot, VecI());
			SG_Rotatev(cam, jRot, VecJ());
		} else {
			SG_Vector vFocus = SG_NodePos(cam->focus[0]);

			SG_Orbitv(cam, vFocus, VecI(), iRot);
			SG_Orbitv(cam, vFocus, VecJ(), jRot);
		}
		break;
	default:
		break;
	}
}

SG_NodeOps sgCameraOps = {
	"Camera",
	sizeof(SG_Camera),
	0,
	SG_CameraInit,
	NULL,		/* destroy */
	SG_CameraLoad,
	SG_CameraSave,
	SG_CameraEdit,
	SG_CameraMenu,
	NULL,		/* menuClass */
	SG_CameraDraw
};

#endif /* HAVE_OPENGL */
