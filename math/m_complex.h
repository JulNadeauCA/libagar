/*	Public domain	*/
/*
 * Complex number arithmetic.
 */

__BEGIN_DECLS
static __inline__ M_Complex
M_ComplexGet(M_Real r, M_Real i)
{
	M_Complex z;

	z.r = r;
	z.i = i;
	return (z);
}

static __inline__ M_Complex
M_ComplexAdditiveInverse(M_Complex z)
{
	M_Complex w;

	w.r = -z.r;
	w.i = -z.i;
	return (w);
}

static __inline__ M_Complex
M_ComplexMultiplicativeInverse(M_Complex z)
{
	M_Complex w;
	M_Real divisor;

	divisor = z.r*z.r + z.i*z.i;
	w.r =  z.r/divisor;
	w.i = -z.i/divisor;
	return (w);
}

static __inline__ M_Real
M_ComplexModulus(M_Complex z)
{
	return M_Sqrt(z.r*z.r + z.i*z.i);
}

static __inline__ M_Real
M_ComplexArg(M_Complex z)
{
	return M_Atan2(z.i, z.r);
}

static __inline__ void
M_ComplexPolar(M_Complex z, M_Real *r, M_Real *theta)
{
	*r = M_ComplexModulus(z);
	*theta = M_ComplexArg(z);
}

static __inline__ M_Complex
M_ComplexAdd(M_Complex a, M_Complex b)
{
	M_Complex w;

	w.r = a.r + b.r;
	w.i = a.i + b.i;
	return (w);
}

static __inline__ M_Complex
M_ComplexSub(M_Complex a, M_Complex b)
{
	M_Complex w;

	w.r = a.r - b.r;
	w.i = a.i - b.i;
	return (w);
}

static __inline__ M_Complex
M_ComplexMult(M_Complex a, M_Complex b)
{
	M_Complex rv;

	rv.r = a.r*b.r - a.i*b.i;
	rv.i = a.i*b.r + a.r*b.i;
	return (rv);
}
__END_DECLS

__BEGIN_DECLS
#define   M_ComplexI() M_ComplexGet(0.0, 1.0)
#define   M_ComplexMinusI() M_ComplexGet(0.0, -1.0)
#define   M_ComplexReal(z) ((z).r)
#define   M_ComplexImag(z) ((z).i)
M_Real    M_ComplexAbs(M_Complex);
M_Complex M_ComplexDiv(M_Complex, M_Complex) _Pure_Attribute;
M_Complex M_ComplexSqrt(M_Complex);
M_Complex M_ComplexLog(M_Complex);
M_Complex M_ComplexExp(M_Complex);
M_Complex M_ComplexPow(M_Complex, M_Complex);

M_Complex M_ComplexSin(M_Complex);
M_Complex M_ComplexCos(M_Complex);
M_Complex M_ComplexTan(M_Complex);
M_Complex M_ComplexCot(M_Complex);
M_Complex M_ComplexAsin(M_Complex);
M_Complex M_ComplexAcos(M_Complex);
M_Complex M_ComplexAtan(M_Complex);
M_Complex M_ComplexSinh(M_Complex);
M_Complex M_ComplexAsinh(M_Complex);
M_Complex M_ComplexCosh(M_Complex);
M_Complex M_ComplexAcosh(M_Complex);
M_Complex M_ComplexTanh(M_Complex);
M_Complex M_ComplexAtanh(M_Complex);
__END_DECLS
