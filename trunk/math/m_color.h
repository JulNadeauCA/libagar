/*	Public domain	*/

__BEGIN_DECLS
void	M_WriteColor(AG_DataSource *, const M_Color *);
M_Color	M_ReadColor(AG_DataSource *);
M_Color	M_ColorHSVA(M_Real, M_Real, M_Real, M_Real);
void	M_ColorTo4fv(const M_Color *, float *);
void	M_ColorTo4dv(const M_Color *, double *);
#define M_ColorHSV(h,s,v) M_ColorHSVA((h),(s),(v),1.0)
#define M_ColorBlack() M_ColorRGB(0.0, 0.0, 0.0)
#define M_ColorWhite() M_ColorRGB(1.0, 1.0, 1.0)
#define M_ColorGray(x) M_ColorRGB((x),(x),(x))

static __inline__ M_Color
M_ColorRGB(M_Real r, M_Real g, M_Real b)
{
	M_Color c;

	c.r = r;
	c.g = g;
	c.b = b;
	c.a = 1.0;
	return (c);
}

static __inline__ M_Color
M_ColorRGBA(M_Real r, M_Real g, M_Real b, M_Real a)
{
	M_Color c;

	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;
	return (c);
}
__END_DECLS
