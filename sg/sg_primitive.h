/*	$Csoft$	*/
/*	Public domain	*/

__BEGIN_DECLS
#define SG_Begin(p) glBegin(p)
#define SG_End(p) glEnd()

#define SG_POINTS GL_POINTS
#define SG_LINES GL_LINES
#define SG_LINE_STRIP GL_LINE_STRIP
#define SG_LINE_LOOP GL_LINE_LOOP
#define SG_TRIANGLES GL_TRIANGLES
#define SG_TRIANGLE_STRIP GL_TRIANGLE_STRIP
#define SG_TRIANGLE_FAN GL_TRIANGLE_FAN
#define SG_QUADS GL_QUADS
#define SG_QUAD_STRIP GL_QUAD_STRIP
#define SG_POLYGON GL_POLYGON

#ifdef SG_DOUBLE_PRECISION
#define SG_Vertex2(x,y) glVertex2d((x),(y))
#define SG_Vertex2v(v) glVertex2dv((GLdouble *)(void *)(v))
#define SG_Vertex3(x,y,z) glVertex3d((x),(y),(z))
#define SG_Vertex3v(v) glVertex3dv((GLdouble *)(void *)(v))
#define SG_Vertex4(x,y,z,w) glVertex4d((x),(y),(z),(w))
#define SG_Vertex4v(v) glVertex4dv((GLdouble *)(void *)(v))
#define SG_Normal3(x,y,z) glNormal3d((x),(y),(z))
#define SG_Normal3v(v) glNormal3dv((GLdouble *)(void *)(v))
#define SG_Color3v(c) glColor3d((c)->r,(c)->g,(c)->b)
#define SG_Color4v(c) glColor4d((c)->r,(c)->g,(c)->b,(c)->a)
#define SG_TexCoord1(s) glTexCoord1d((s))
#define SG_TexCoord2(s,t) glTexCoord2d((s),(t))
#define SG_TexCoord3(s,t,r) glTexCoord3d((s),(t),(r))
#define SG_TexCoord4(s,t,r,q) glTexCoord4d((s),(t),(r),(q))
#define SG_Materialf(face,pname,v) glMaterialf((face),(pname),(GLfloat)(v))
#define SG_MaterialColor(face,pname,c) do { \
		GLfloat _v[4]; \
		_v[0] = (GLfloat)(c)->r; \
		_v[1] = (GLfloat)(c)->g; \
		_v[2] = (GLfloat)(c)->b; \
		_v[3] = (GLfloat)(c)->a; \
		glMaterialfv((face),(pname),_v); \
	} while (0)
#else /* !SG_DOUBLE_PRECISION */
#define SG_Vertex2(x,y) glVertex2f((x),(y))
#define SG_Vertex2v(v) glVertex2fv((GLfloat *)(void *)(v))
#define SG_Vertex3(x,y,z) glVertex3f((x),(y),(z))
#define SG_Vertex3v(v) glVertex3fv((GLfloat *)(void *)(v))
#define SG_Vertex4(x,y,z,w) glVertex4f((x),(y),(z),(w))
#define SG_Vertex4v(v) glVertex4fv((GLfloat *)(void *)(v))
#define SG_Normal3(x,y,z) glNormal3f((x),(y),(z))
#define SG_Normal3v(v) glNormal3fv((GLfloat *)(void *)(v))
#define SG_Color3v(c) glColor3f((c)->r,(c)->g,(c)->b)
#define SG_Color4v(c) glColor4f((c)->r,(c)->g,(c)->b,(c)->a)
#define SG_TexCoord1(s) glTexCoord1f((s))
#define SG_TexCoord2(s,t) glTexCoord2f((s),(t))
#define SG_TexCoord3(s,t,r) glTexCoord3f((s),(t),(r))
#define SG_TexCoord4(s,t,r,q) glTexCoord4f((s),(t),(r),(q))
#define SG_Materialf(face,pname,v) glMaterialf((face),(pname),(v))
#define SG_MaterialColor(face,pname,c) glMaterialfv((face),(pname),\
                                                    (GLfloat *)(void *)c)
#endif /* SG_DOUBLE_PRECISION */

#define SG_Normal3b(x,y,z) glNormal3b((x),(y),(z))
#define SG_Normal3d(x,y,z) glNormal3d((x),(y),(z))
#define SG_Normal3f(x,y,z) glNormal3f((x),(y),(z))
#define SG_Normal3i(x,y,z) glNormal3i((x),(y),(z))
#define SG_Normal3s(x,y,z) glNormal3s((x),(y),(z))
#define SG_Normal3bv(v) glNormal3bv(v)
#define SG_Normal3dv(v) glNormal3dv(v)
#define SG_Normal3fv(v) glNormal3fv(v)
#define SG_Normal3iv(v) glNormal3iv(v)
#define SG_Normal3sv(v) glNormal3sv(v)

#define SG_Color3b(r,g,b) glColor3b((r),(g),(b))
#define SG_Color3d(r,g,b) glColor3d((r),(g),(b))
#define SG_Color3f(r,g,b) glColor3f((r),(g),(b))
#define SG_Color3i(r,g,b) glColor3i((r),(g),(b))
#define SG_Color3s(r,g,b) glColor3s((r),(g),(b))
#define SG_Color3ub(r,g,b) glColor3ub((r),(g),(b))
#define SG_Color3ui(r,g,b) glColor3ui((r),(g),(b))
#define SG_Color3us(r,g,b) glColor3us((r),(g),(b))
#define SG_Color4b(r,g,b,a) glColor4b((r),(g),(b),(a))
#define SG_Color4d(r,g,b,a) glColor4d((r),(g),(b),(a))
#define SG_Color4f(r,g,b,a) glColor4f((r),(g),(b),(a))
#define SG_Color4i(r,g,b,a) glColor4i((r),(g),(b),(a))
#define SG_Color4s(r,g,b,a) glColor4s((r),(g),(b),(a))
#define SG_Color4ub(r,g,b,a) glColor4ub((r),(g),(b),(a))
#define SG_Color4ui(r,g,b,a) glColor4ui((r),(g),(b),(a))
#define SG_Color4us(r,g,b,a) glColor4us((r),(g),(b),(a))
#define SG_Color3bv(v) glColor3bv(v)
#define SG_Color3dv(v) glColor3dv(v)
#define SG_Color3fv(v) glColor3fv(v)
#define SG_Color3iv(v) glColor3iv(v)
#define SG_Color3sv(v) glColor3sv(v)
#define SG_Color3ubv(v) glColor3ubv(v)
#define SG_Color3uiv(v) glColor3uiv(v)
#define SG_Color3usv(v) glColor3usv(v)
#define SG_Color4bv(v) glColor4bv(v)
#define SG_Color4dv(v) glColor4dv(v)
#define SG_Color4fv(v) glColor4fv(v)
#define SG_Color4iv(v) glColor4iv(v)
#define SG_Color4sv(v) glColor4sv(v)
#define SG_Color4ubv(v) glColor4ubv(v)
#define SG_Color4uiv(v) glColor4uiv(v)
#define SG_Color4usv(v) glColor4usv(v)
#define SG_ColorPointer(s,t,r,p) glColorPointer((s),(t),(r),(p))
#define SG_Indexd(c) glIndexd(c)
#define SG_Indexf(c) glIndexf(c)
#define SG_Indexi(c) glIndexi(c)
#define SG_Indexs(c) glIndexs(c)
#define SG_Indexub(c) glIndexub(c)
#define SG_IndexPointer(t,s,p) glIndexPointer((t),(s),(p))

#define SG_VertexTN(vp) do { \
	SG_Vertex *_vt = (vp); \
	SG_TexCoord2(_vt->s,_vt->t); \
	SG_Normal3v(&_vt->n); \
	SG_Color4v(&_vt->c); \
	SG_Vertex3v(&_vt->v); \
} while (0)

void SG_WireBox2(SG_Vector, SG_Vector);
void SG_TessRect3(SG_Plane, SG_Real, SG_Real, SG_Real);
void SG_SolidBox2(SG_Vector, SG_Vector);
__END_DECLS
