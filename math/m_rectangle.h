/*	Public domain	*/

__BEGIN_DECLS
M_Rectangle2 M_RectangleRead2(AG_DataSource *_Nonnull);
M_Rectangle3 M_RectangleRead3(AG_DataSource *_Nonnull);
void         M_RectangleWrite2(AG_DataSource *_Nonnull, M_Rectangle2 *_Nonnull);
void         M_RectangleWrite3(AG_DataSource *_Nonnull, M_Rectangle3 *_Nonnull);
int          M_PointInRectangle2(M_Rectangle2, M_Vector2);

static __inline__ M_Rectangle2
M_RectangleFromPts2(M_Vector2 a, M_Vector2 b, M_Vector2 c, M_Vector2 d)
{
	M_Rectangle2 R;
	R.a = a;
	R.b = b;
	R.c = c;
	R.d = d;
	return (R);
}

static __inline__ M_Rectangle3
M_RectangleFromPts3(M_Vector3 a, M_Vector3 b, M_Vector3 c, M_Vector3 d)
{
	M_Rectangle3 R;
	R.a = a;
	R.b = b;
	R.c = c;
	R.d = d;
	return (R);
}

static __inline__ M_Real
M_RectangleWidth3(M_Rectangle3 R)
{
	return M_VecDistance3(R.b, R.c);
}
static __inline__ M_Real
M_RectangleWidth3v(const M_Rectangle3 *_Nonnull R)
{
	return M_VecDistance3(R->b, R->c);
}

static __inline__ M_Real
M_RectangleHeight3(M_Rectangle3 R)
{
	return M_VecDistance3(R.a, R.b);
}
static __inline__ M_Real
M_RectangleHeight3v(const M_Rectangle3 *_Nonnull R)
{
	return M_VecDistance3(R->a, R->b);
}
__END_DECLS
