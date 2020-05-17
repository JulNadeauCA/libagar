/*	Public domain	*/
/*
 * Complex number arithmetic.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

static M_Real Ctans(M_Complex);

/*
 * Compute the complex absolute value (sqrt(r^2 + i^2)). If the magnitude
 * of either the real or imaginary parts are outside half of the precision
 * range, both parts are rescaled prior to squaring.
 */
M_Real
M_ComplexAbs(M_Complex z)
{
	M_Real x, y, b, rPart, iPart;
	int ex, ey, e;

	if (z.r ==  M_INFINITY || z.i ==  M_INFINITY ||
	    z.r == -M_INFINITY || z.i == -M_INFINITY) {
		return (M_INFINITY);
	}
	if ((rPart = Fabs(z.r)) == 0.0) { return Fabs(z.i); }
	if ((iPart = Fabs(z.i)) == 0.0) { return Fabs(z.r); }
	x = Frexp(rPart, &ex);
	y = Frexp(iPart, &ey);
	e = ex - ey;
	if (e >  M_PRECISION_2) { return (rPart); }
	if (e < -M_PRECISION_2) { return (iPart); }
	e = (ex + ey)/2;
	x = Ldexp(rPart, -e);
	y = Ldexp(iPart, -e);
	b = Sqrt(x*x + y*y);
	y = Frexp(b, &ey);
	ey = e + ey;
	if (ey < -M_EXPMIN) { return (0.0); }
	if (ey >  M_EXPMAX) { return (M_INFINITY); }
	b = Ldexp(b, e);
	return (b);
}

/* Divide two complex numbers. */
M_Complex
M_ComplexDiv(M_Complex a, M_Complex b)
{
	M_Real y, p, q, w;
	M_Complex z;

	y = a.r*a.r + a.i*a.i;
	p = b.r*a.r + b.i*a.i;
	q = b.i*a.r - b.r*a.i;
	if (y < 1.0) {
		w = M_INFINITY*y;
		if (Fabs(p) > w || Fabs(q) > w || y == 0.0) {
			z.r = M_INFINITY;
			z.i = M_INFINITY;
			return (z);
		}
	}
	z.r = p/y;
	z.i = q/y;
	return (z);
}

/* Compute the complex square root of z. */
M_Complex
M_ComplexSqrt(M_Complex z)
{
	M_Complex w;
	M_Real x = z.r;
	M_Real y = z.i;
	M_Real r, t, scale;

	if (y == 0.0) {
		if (x == 0.0) {
			w.r = 0.0;
			w.i = y;
		} else {
			r = Fabs(x);
			r = Sqrt(r);
			if (x < 0.0) {
				w.r = 0.0;
				w.i = r;
			} else {
				w.r = r;
				w.i = y;
			}
		}
		return (w);
	}
	if (x == 0.0) {
		r = Fabs(y);
		r = Sqrt(r/2.0);
		if (y > 0) {
			w.r =  r;
			w.i =  r;
		} else {
			w.r =  r;
			w.i = -r;
		}
		return (w);
	}
	if (Fabs(x) > 4.0 || Fabs(y) > 4.0) {		/* Rescale */
		x /= 4.0;
		y /= 4.0;
		scale = 2.0;
	} else {
		x *= 1.8014398509481984e16;	 /* 2^54 */
		y *= 1.8014398509481984e16;	 /* 2^54 */
		scale = 7.450580596923828125e-9; /* 2^-27 */
	}
	w.r = x;
	w.i = y;
	r = M_ComplexAbs(w);
	if (x > 0) {
		t = Sqrt(0.5*r + 0.5*x);
		r = scale*Fabs((0.5*y)/t);
		t *= scale;
	} else {
		r = Sqrt(0.5*r - 0.5*x);
		t = scale*Fabs((0.5*y)/r);
		r *= scale;
	}
	if (y < 0) {
		w.r =  t;
		w.i = -r;
	} else {
		w.r = t;
		w.i = r;
	}
	return (w);

}

/* Compute the complex natural logarithm of z. */
M_Complex
M_ComplexLog(M_Complex z)
{
	M_Complex w;
	M_Real p, rr;

	rr = M_ComplexAbs(z);
	p = Log(rr);
	rr = Atan2(z.i, z.r);
	w.r = p;
	w.i = rr;
	return (w);
}

/* Compute the exponential of z. */
M_Complex
M_ComplexExp(M_Complex z)
{
	M_Complex w;
	M_Real r;

	r = Exp(z.r);
	w.r = r*Cos(z.i);
	w.i = r*Sin(z.i);
	return (w);
}

/* Compute a to the complex power z. */
M_Complex
M_ComplexPow(M_Complex a, M_Complex z)
{
	M_Complex w;
	M_Real aAbs, aArg;
	M_Real r, theta;

	if ((aAbs = M_ComplexAbs(a)) == 0.0) {
		w.r = 0.0;
		w.i = 0.0;
		return (w);
	}
	aArg = M_ComplexArg(a);
	r = Pow(aAbs, z.r);
	theta = z.r*aArg;
	if (z.i != 0.0) {
		r = r*Exp(-z.i*aArg);
		theta += z.i*Log(aAbs);
	}
	w.r = r*Cos(theta);
	w.i = r*Sin(theta);
	return (w);
}

/* Compute the complex sine of z. */
M_Complex
M_ComplexSin(M_Complex z)
{
	M_Complex w;
	M_Real ch, sh;

	if (Fabs(z.i) <= 0.5) {
		ch = Cosh(z.i);
		sh = Sinh(z.i);
	} else {
		M_Real e, ei;
  		e = Exp(z.i);
		ei = 0.5/e;
		e = 0.5*e;
		ch = e+ei;
		sh = e-ei;
	}
	w.r = Sin(z.r)*ch;
	w.i = Cos(z.r)*sh;
	return (w);
}

/* Compute the complex cosine of z. */
M_Complex
M_ComplexCos(M_Complex z)
{
	M_Complex w;
	M_Real ch, sh;
	
	if (Fabs(z.i) <= 0.5) {
		ch = Cosh(z.i);
		sh = Sinh(z.i);
	} else {
		M_Real e, ei;
  		e = Exp(z.i);
		ei = 0.5/e;
		e = 0.5*e;
		ch = e+ei;
		sh = e-ei;
	}
	w.r =  Cos(z.r)*ch;
	w.i = -Sin(z.r)*sh;
	return (w);
}

/* Compute the complex tangent of z. */
M_Complex
M_ComplexTan(M_Complex z)
{
	M_Complex w;
	M_Real d;

	d = Cos(2.0*z.r) + Cosh(2.0*z.i);
	if (Fabs(d) < 0.25) {
		d = Ctans(z);
	}
	if (d == 0.0) {
		w.r = M_INFINITY;
		w.i = M_INFINITY;
		return (w);
	}
	w.r = Sin(2.0*z.r) / d;
	w.i = Sinh(2.0*z.i) / d;
	return (w);
}

/* Compute the complex cotangent of z. */
M_Complex
M_ComplexCot(M_Complex z)
{
	M_Complex w;
	M_Real d;

	d = Cosh(2.0*z.i) - Cos(2.0*z.r);
	if (Fabs(d) < 0.25) {
		d = Ctans(z);
	}
	if (d == 0.0) {
		w.r = M_INFINITY;
		w.i = M_INFINITY;
		return (w);
	}
	w.r =   Sin(2.0*z.r) / d;
	w.i = -Sinh(2.0*z.i) / d;
	return (w);
}

/* Subtract nearest integer multiple of pi. */ 
static M_Real /* _Const_Attribute */
SubNearestIntMultOfPi(M_Real x)
{
	const M_Real dp1 = 3.14159265160560607910E0;
	const M_Real dp2 = 1.98418714791870343106E-9;
	const M_Real dp3 = 1.14423774522196636802E-17;
	M_Real t;
	long i;

	t = x/M_PI;
	if (t >= 0.0) {
		t += 0.5;
	} else {
		t -= 0.5;
	}
	i = t;
	t = i;
	t = ((x - t*dp1) - t*dp2) - t*dp3;
	return (t);
}

/* Taylor series expansion for cosh(2y) - cos(2x) */
static M_Real /* _Pure_Attribute */
Ctans(M_Complex z)
{
	M_Real x2 = 1.0, y2 = 1.0, f = 1.0, rn = 0.0, d = 0.0;
	M_Real x, y, t;

	x = Fabs(2.0*z.r);
	y = Fabs(2.0*z.i);
	x = SubNearestIntMultOfPi(x);
	x = x*x;
	y = y*y;
	do {
		rn += 1.0;	f *= rn;
		rn += 1.0;	f *= rn;
		x2 *= x;	y2 *= y;
		t = y2+x2;	t /= f;		d += t;
		rn += 1.0;	f *= rn;
		rn += 1.0;	f *= rn;
		x2 *= x;	y2 *= y;
		t = y2-x2;	t /= f;		d += t;
	} while (Fabs(t/d) > M_MACHEP);
	return (d);
}

/* Compute the complex arc sine of z. */
M_Complex
M_ComplexAsin(M_Complex z)
{
	M_Complex ca, ct, zz, z2;
	M_Real x = z.r, y = z.i;

	if (y == 0.0) {
		M_Complex w;
		w.r = (Fabs(x) > 1.0) ?  M_PI_2 : Asin(x);
		w.i = 0.0;
		return (w);
	}
	ca.r = x;
	ca.i = y;
	ct = M_ComplexMult(ca, M_ComplexI());
	zz.r = (x-y)*(x+y);
	zz.i = (2.0*x*y);
	zz.r = 1.0-zz.r;
	zz.i = -zz.i;
	z2 = M_ComplexSqrt(zz);
	zz = M_ComplexAdd(ct, z2);
	zz = M_ComplexLog(zz);
	return M_ComplexMult(zz, M_ComplexMinusI());
}

/* Compute the complex arc cosine of z. */
M_Complex
M_ComplexAcos(M_Complex z)
{
	M_Complex w;
	
	w = M_ComplexAsin(z);
	w.r = M_PI_2 - w.r;
	w.i = -w.i;
	return (w);
}

/* Compute the complex arc tangent of z. */
M_Complex
M_ComplexAtan(M_Complex z)
{
	M_Complex w;
	M_Real a, t, x, x2, y;

	x = z.r;
	y = z.i;

	if ((x == 0.0) && (y > 1.0)) {
		goto oflow;
	}
	x2 = x*x;
	a = 1.0 - x2 - y*y;
	if (a == 0.0) { goto oflow; }
	t = Atan2(2.0*x,a)/2.0;

	w.r = SubNearestIntMultOfPi(t);
	w.i = 0.0;

	t = y-1.0;
	a = (x2 + t*t);
	if (a == 0.0) { goto oflow; }
	t = y+1.0;
	a = (x2 + t*t)/a;
	w.i += Log(a)/4.0;
	return (w);
oflow:
	w.r = M_INFINITY;
	w.i = M_INFINITY;
	return (w);
}

/* Compute the complex hyperbolic sine of z. */
M_Complex
M_ComplexSinh(M_Complex z)
{
	M_Complex w;

	w.r = Sinh(z.r) * Cos(z.i);
	w.i = Cosh(z.r) * Sin(z.i);
	return (w);
}

/* Compute the complex hyperbolic arc sine of z. */
M_Complex
M_ComplexAsinh(M_Complex z)
{
	M_Complex zi;

	zi = M_ComplexMult(z, M_ComplexI());
	return M_ComplexMult(M_ComplexAsin(zi), M_ComplexMinusI());
}

/* Compute the complex hyperbolic cosine of z. */
M_Complex
M_ComplexCosh(M_Complex z)
{
	M_Complex w;

	w.r = Cosh(z.r)*Cos(z.i);
	w.i = Sinh(z.r)*Sin(z.i);
	return (w);
}

/* Compute the complex hyperbolic arc cosine of z. */
M_Complex
M_ComplexAcosh(M_Complex z)
{
	return M_ComplexMult(M_ComplexI(), M_ComplexAcos(z));
}

/* Compute the complex hyperbolic tangent of z. */
M_Complex
M_ComplexTanh(M_Complex z)
{
	M_Complex w;
	M_Real d;

	d = Cosh(2.0*z.r) + Cos(2.0*z.i);
	w.r = Sinh(2.0*z.r)/d;
	w.i = Sin (2.0*z.i)/d;
	return (w);
}

/* Compute the complex hyperbolic arc tangent of z. */
M_Complex
M_ComplexAtanh(M_Complex z)
{
	M_Complex zi;

	zi = M_ComplexMult(z, M_ComplexI());
	return M_ComplexMult(zi, M_ComplexMinusI());
}
