/*	Public domain	*/
/*
 * Wrappers around GL calls taking M_Real, M_Vector[34] and M_Matrix44
 * arguments. Map to the correct float or double precision GL calls
 * based on the precision of the math library.
 *
 * Macros for other GL calls related to drawing are also provided for
 * symmetry.
 */

#ifndef _AGAR_MATH_GL_MACROS_H_
#define _AGAR_MATH_GL_MACROS_H_

__BEGIN_DECLS

#include <agar/config/single_precision.h>
#include <agar/config/double_precision.h>
#include <agar/config/quad_precision.h>
#include <agar/config/have_sse.h>

/*
 * M_Vector2 and M_Real argument wrappers
 */
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
# define GL_Vertex2(x,y)	glVertex2f((x),(y))
# define GL_Vertex2v(v)		glVertex2fv((const GLfloat *)(v))
# define GL_TexCoord1(s)	glTexCoord1f((s))
# define GL_TexCoord2(s,t)	glTexCoord2f((s),(t))
# define GL_TexCoord3(s,t,r)	glTexCoord3f((s),(t),(r))
# define GL_TexCoord4(s,t,r,q)	glTexCoord4f((s),(t),(r),(q))
#elif defined(DOUBLE_PRECISION)
# define GL_Vertex2(x,y)	glVertex2d((x),(y))
# define GL_Vertex2v(v)		glVertex2dv((const GLdouble *)(v))
# define GL_TexCoord1(s)	glTexCoord1d((s))
# define GL_TexCoord2(s,t)	glTexCoord2d((s),(t))
# define GL_TexCoord3(s,t,r)	glTexCoord3d((s),(t),(r))
# define GL_TexCoord4(s,t,r,q)	glTexCoord4d((s),(t),(r),(q))
#endif

/*
 * M_Vector3 and M_Vector4 argument wrappers
 */
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
# define GL_Vertex3(x,y,z)	glVertex3f((x),(y),(z))
# define GL_Vertex3v(v)		glVertex3fv((const GLfloat *)(v))
# define GL_Vertex4(x,y,z,w)	glVertex4f((x),(y),(z),(w))
# define GL_Vertex4v(v)		glVertex4fv((const GLfloat *)(v))
# define GL_Normal3(x,y,z)	glNormal3f((x),(y),(z))
# define GL_Normal3v(v)		glNormal3fv((const GLfloat *)(v))
#elif defined(DOUBLE_PRECISION)
# define GL_Vertex3(x,y,z)	glVertex3d((x),(y),(z))
# define GL_Vertex3v(v)		glVertex3dv((const GLdouble *)(v))
# define GL_Vertex4(x,y,z,w)	glVertex4d((x),(y),(z),(w))
# define GL_Vertex4v(v)		glVertex4dv((const GLdouble *)(void *)(v))
# define GL_Normal3(x,y,z)	glNormal3d((x),(y),(z))
# define GL_Normal3v(v)		glNormal3dv((const GLdouble *)(void *)(v))
#endif

/* 
 * Macros provided for symmetry
 */
#define GL_Begin(p)		glBegin(p)
#define GL_End()		glEnd()
#define GL_Enable(cap)		glEnable(cap)
#define GL_Disable(cap)		glDisable(cap)
#define GL_IsEnabled(cap)	glIsEnabled(cap)
#define GL_GetBooleanv(pname,p)	glGetBooleanv((pname),(p))
#define GL_GetFloatv(pname,p)	glGetFloatv((pname),(p))
#define GL_GetDoublev(pname,p)	glGetFloatv((pname),(p))
#define GL_GetIntegerv(pname,p)	glGetIntegerv((pname),(p))

#define GL_Normal3b(x,y,z)	glNormal3b((x),(y),(z))
#define GL_Normal3d(x,y,z)	glNormal3d((x),(y),(z))
#define GL_Normal3f(x,y,z)	glNormal3f((x),(y),(z))
#define GL_Normal3i(x,y,z)	glNormal3i((x),(y),(z))
#define GL_Normal3s(x,y,z)	glNormal3s((x),(y),(z))
#define GL_Normal3bv(v)		glNormal3bv(v)
#define GL_Normal3dv(v)		glNormal3dv(v)
#define GL_Normal3fv(v)		glNormal3fv(v)
#define GL_Normal3iv(v)		glNormal3iv(v)
#define GL_Normal3sv(v)		glNormal3sv(v)
#define GL_Color3b(r,g,b)	glColor3b((r),(g),(b))
#define GL_Color3d(r,g,b)	glColor3d((r),(g),(b))
#define GL_Color3f(r,g,b)	glColor3f((r),(g),(b))
#define GL_Color3i(r,g,b)	glColor3i((r),(g),(b))
#define GL_Color3s(r,g,b)	glColor3s((r),(g),(b))
#define GL_Color3ub(r,g,b)	glColor3ub((r),(g),(b))
#define GL_Color3ui(r,g,b)	glColor3ui((r),(g),(b))
#define GL_Color3us(r,g,b)	glColor3us((r),(g),(b))
#define GL_Color4b(r,g,b,a)	glColor4b((r),(g),(b),(a))
#define GL_Color4d(r,g,b,a)	glColor4d((r),(g),(b),(a))
#define GL_Color4f(r,g,b,a)	glColor4f((r),(g),(b),(a))
#define GL_Color4i(r,g,b,a)	glColor4i((r),(g),(b),(a))
#define GL_Color4s(r,g,b,a)	glColor4s((r),(g),(b),(a))
#define GL_Color4ub(r,g,b,a)	glColor4ub((r),(g),(b),(a))
#define GL_Color4ui(r,g,b,a)	glColor4ui((r),(g),(b),(a))
#define GL_Color4us(r,g,b,a)	glColor4us((r),(g),(b),(a))
#define GL_Color3bv(v)		glColor3bv(v)
#define GL_Color3dv(v)		glColor3dv(v)
#define GL_Color3fv(v)		glColor3fv(v)
#define GL_Color3iv(v)		glColor3iv(v)
#define GL_Color3sv(v)		glColor3sv(v)
#define GL_Color3ubv(v)		glColor3ubv(v)
#define GL_Color3uiv(v)		glColor3uiv(v)
#define GL_Color3usv(v)		glColor3usv(v)
#define GL_Color4bv(v)		glColor4bv(v)
#define GL_Color4dv(v)		glColor4dv(v)
#define GL_Color4fv(v)		glColor4fv(v)
#define GL_Color4iv(v)		glColor4iv(v)
#define GL_Color4sv(v)		glColor4sv(v)
#define GL_Color4ubv(v)		glColor4ubv(v)
#define GL_Color4uiv(v)		glColor4uiv(v)
#define GL_Color4usv(v)		glColor4usv(v)
#define GL_ColorPointer(s,t,r,p) glColorPointer((s),(t),(r),(p))
#define GL_Indexd(c)		glIndexd(c)
#define GL_Indexf(c)		glIndexf(c)
#define GL_Indexi(c)		glIndexi(c)
#define GL_Indexs(c)		glIndexs(c)
#define GL_Indexub(c)		glIndexub(c)
#define GL_IndexPointer(t,s,p)	glIndexPointer((t),(s),(p))
#define GL_LineWidth(w)		glLineWidth(w)
#define GL_LineStipple(f,p)	glLineStipple((f),(p))
#define GL_PointSize(s)		glPointSize(s)
#define GL_ShadeModel(m)	glShadeModel(m)
#define GL_PushAttrib(m)	glPushAttrib(m)
#define GL_PopAttrib()		glPopAttrib()
#define GL_MatrixMode(m)	glMatrixMode(m)
#define GL_PushMatrix()		glPushMatrix()
#define GL_PopMatrix()		glPopMatrix()
#define GL_LoadIdentity()	glLoadIdentity()
#define GL_Material(f,p,v)	glMaterialf((f),(p),(GLfloat)(v))
#define GL_PolygonMode(f,m)	glPolygonMode((f),(m))
#define GL_BlendFunc(sf,df)	glBlendFunc((sf),(df))
#define GL_TexParameterf(t,n,p)	glTexParameterf((t),(n),(p))
#define GL_TexParameteri(t,n,p)	glTexParameteri((t),(n),(p))

static __inline__ void
GL_CallList(GLuint list)
{
	(void)glGetError();
	glCallList(list);
#ifdef GL_DEBUG
	{
		GLenum rv;

		if ((rv = glGetError()) != GL_NO_ERROR) {
			printf("glCallList(%u): Error %u\n", list, rv);
		} else {
			printf("glCallList(%u)\n", list);
		}
	}
#endif
}

static __inline__ void
GL_BindTexture(GLenum target, GLuint texture)
{
	glBindTexture(target, texture);
#ifdef GL_DEBUG
	printf("glBindTexture(%s, %u)\n",
	    (target == GL_TEXTURE_1D) ? "GL_TEXTURE_1D" :
	    (target == GL_TEXTURE_2D) ? "GL_TEXTURE_2D" :
	    (target == GL_TEXTURE_3D) ? "GL_TEXTURE_3D" : "",
	    texture);
#endif
}

static __inline__ GLuint
GL_GenLists(GLsizei range)
{
	GLuint rv;

	(void)glGetError();
	if ((rv = glGenLists(range)) == 0) {
		AG_SetError("glGenLists: Error %d", (int)glGetError());
	}
	return (rv);
}

static __inline__ int
GL_NewList(GLuint name, GLenum mode)
{
#ifdef GL_DEBUG
	printf("glNewList(%d, %s)\n", name,
	    (mode == GL_COMPILE) ? "GL_COMPILE" :
	    (mode == GL_COMPILE_AND_EXECUTE) ? "GL_COMPILE_AND_EXECUTE" :
	    "");
#endif
	(void)glGetError();
	glNewList(name, mode);
	if (glGetError() != GL_NO_ERROR) {
		AG_SetError("Cannot allocate display list");
		return (-1);
	}
	return (0);
}

static __inline__ void
GL_EndList(void)
{
	glEndList();
#ifdef GL_DEBUG
	printf("glEndList()\n");
#endif
}

/* Convert an OpenGL matrix to a M_Matrix44 */
static __inline__ void
GL_FetchMatrixv(GLenum which, M_Matrix44 *T)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glGetFloatv(which, &T->m[0][0]);
#elif defined(DOUBLE_PRECISION)
	glGetDoublev(which, &T->m[0][0]);
#endif
}

/* Load an OpenGL matrix from a M_Matrix44. */
static __inline__ void
GL_LoadMatrixv(const M_Matrix44 *T)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glLoadMatrixf(&T->m[0][0]);
#elif defined(DOUBLE_PRECISION)
	glLoadMatrixd(&T->m[0][0]);
#endif
}

/* Multiply the current OpenGL matrix with a M_Matrix44. */
static __inline__ void
GL_MultMatrixv(const M_Matrix44 *T)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glMultMatrixf(&T->m[0][0]);
#elif defined(DOUBLE_PRECISION)
	glMultMatrixd(&T->m[0][0]);
#endif
}

/* Multiply the current OpenGL matrix with a translation matrix. */
static __inline__ void
GL_Translate(M_Vector3 v)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glTranslatef(v.x, v.y, v.z);
#elif defined(DOUBLE_PRECISION)
	glTranslated(v.x, v.y, v.z);
#endif
}
static __inline__ void
GL_Translatev(const M_Vector3 *v)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glTranslatef(v->x, v->y, v->z);
#elif defined(DOUBLE_PRECISION)
	glTranslated(v->x, v->y, v->z);
#endif
}

/* Multiply the current OpenGL matrix with a rotation matrix. */
static __inline__ void
GL_Rotate(M_Real theta, M_Vector3 a)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glRotatef(theta, a.x, a.y, a.z);
#elif defined(DOUBLE_PRECISION)
	glRotated(theta, a.x, a.y, a.z);
#endif
}
static __inline__ void
GL_Rotatev(M_Real theta, const M_Vector3 *a)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glRotatef(theta, a->x, a->y, a->z);
#elif defined(DOUBLE_PRECISION)
	glRotated(theta, a->x, a->y, a->z);
#endif
}

/* Multiply the current OpenGL matrix with a scaling matrix. */
static __inline__ void
GL_Scale3(M_Real x, M_Real y, M_Real z)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glScalef(x, y, z);
#elif defined(DOUBLE_PRECISION)
	glScaled(x, y, z);
#endif
}

/* Set the OpenGL color from a M_Color(3) */
static __inline__ void
GL_Color3(M_Color C)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glColor3fv((const GLfloat *)&C);
#elif defined(DOUBLE_PRECISION)
	glColor3dv((const GLdouble *)&C);
#endif
}
static __inline__ void
GL_Color3v(const M_Color *C)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glColor3fv((const GLfloat *)C);
#elif defined(DOUBLE_PRECISION)
	glColor3dv((const GLdouble *)C);
#endif
}
static __inline__ void
GL_Color4(M_Color C)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glColor4fv((const GLfloat *)&C);
#elif defined(DOUBLE_PRECISION)
	glColor4dv((const GLdouble *)&C);
#endif
}
static __inline__ void
GL_Color4v(const M_Color *C)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glColor4fv((const GLfloat *)C);
#elif defined(DOUBLE_PRECISION)
	glColor4dv((const GLdouble *)C);
#endif
}

/* Set the OpenGL material color from a M_Color(3) */
static __inline__ void
GL_MaterialColorv(GLenum face, GLenum pname, const M_Color *C)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	glMaterialfv(face, pname, (const GLfloat *)C);
#else
	GLfloat v[4];

	v[0] = (GLfloat)C->r;
	v[1] = (GLfloat)C->g;
	v[2] = (GLfloat)C->b;
	v[3] = (GLfloat)C->a;
	glMaterialfv(face, pname, v);
#endif
}

/* Convert an OpenGL numerical value to an M_Real */
static __inline__ void
GL_GetRealv(GLenum pname, M_Real *v)
{
#if defined(SINGLE_PRECISION)
	glGetFloatv(pname, v);
#elif defined(DOUBLE_PRECISION)
	glGetDoublev(pname, v);
#endif
}

static __inline__ void
GL_EnableSave(GLenum cap, int *save)
{
	if (!(*save = glIsEnabled(cap)))
		glEnable(cap);
}
static __inline__ void
GL_DisableSave(GLenum cap, int *save)
{
	if ((*save = glIsEnabled(cap)))
		glDisable(cap);
}
static __inline__ void
GL_EnableSaved(GLenum cap, int save)
{
	if (save)
		glEnable(cap);
}
static __inline__ void
GL_DisableSaved(GLenum cap, int save)
{
	if (!save)
		glDisable(cap);
}
__END_DECLS
#endif /* _AGAR_MATH_GL_MACROS_H_ */
