/*	$Csoft: vg_math.h,v 1.4 2004/05/24 03:32:22 vedge Exp $	*/
/*	Public domain	*/

#define SC_FLOAT	0x01
#define SC_DOUBLE	0x02
#define SC_LONG_DOUBLE	0x04
#define SC_PRECISION	SC_DOUBLE

#if SC_PRECISION == SC_LONG_DOUBLE
# define SC_WIDGET_REAL AG_WIDGET_LONG_DOUBLE
# define SC_PROP_REAL AG_PROP_LONG_DOUBLE
# define SC_SetReal AG_SetLongDouble
# define SC_GetReal AG_LongDouble
# define SC_SetRealWrFn AG_SetLongDoubleWrFn
# define SC_SetRealRdFn AG_SetLongDoubleRdFn
typedef long double SC_Real;
#elif SC_PRECISION == SC_DOUBLE
# define SC_WIDGET_REAL AG_WIDGET_DOUBLE
# define SC_PROP_REAL AG_PROP_DOUBLE
# define SC_SetReal AG_SetDouble
# define SC_GetReal AG_Double
# define SC_SetRealWrFn AG_SetDoubleWrFn
# define SC_SetRealRdFn AG_SetDoubleRdFn
typedef double SC_Real;
#else
# define SC_WIDGET_REAL AG_WIDGET_FLOAT
# define SC_PROP_REAL AG_PROP_FLOAT
# define SC_SetReal AG_SetFloat
# define SC_GetReal AG_Float
# define SC_SetRealWrFn AG_SetFloatWrFn
# define SC_SetRealRdFn AG_SetFloatRdFn
typedef float SC_Real;
#endif

typedef Uint32 SC_QTime;
typedef SC_Real SC_Time;

#define SC_WIDGET_QTIME	AG_WIDGET_UINT32
#define SC_WIDGET_TIME	SC_WIDGET_REAL

typedef struct sc_range {
	SC_Real min;
	SC_Real typ;
	SC_Real max;
} SC_Range;

typedef struct sc_qtime_range {
	SC_QTime min;
	SC_QTime typ;
	SC_QTime max;
} SC_QTimeRange;

#if SC_PRECISION == SC_LONG_DOUBLE
# define SC_Sqrt(x) sqrtl(x)
# define SC_Sin(x) sinl(x)
# define SC_Cos(x) cosl(x)
# define SC_Tan(x) tanl(x)
# define SC_Cot(x) (1.0/tanl(x))
# define SC_Sec(x) (1.0/cosl(x))
# define SC_Csc(x) (1.0/sinl(x))
# define SC_Asin(x) asinl(x)
# define SC_Acos(x) acosl(x)
# define SC_Atan(x) atanl(x)
# define SC_Atan2(y,x) atan2l(y,x)
# define SC_Fabs(x) fabsl(x)
# define SC_Pow(x,y) powl((x),(y))
#elif SC_PRECISION == SC_DOUBLE
# define SC_Sqrt(x) sqrt(x)
# define SC_Sin(x) sin(x)
# define SC_Cos(x) cos(x)
# define SC_Tan(x) tan(x)
# define SC_Cot(x) (1.0/tan(x))
# define SC_Sec(x) (1.0/cos(x))
# define SC_Csc(x) (1.0/sin(x))
# define SC_Asin(x) asin(x)
# define SC_Acos(x) acos(x)
# define SC_Atan(x) atan(x)
# define SC_Atan2(y,x) atan2(y,x)
# define SC_Fabs(x) fabs(x)
# define SC_Pow(x,y) pow((x),(y))
#elif (SC_PRECISION == SC_FLOAT)
# define SC_Sqrt(r) sqrtf(r)
# define SC_Sin(x) sinf(x)
# define SC_Cos(x) cosf(x)
# define SC_Tan(x) tanf(x)
# define SC_Cot(x) (1.0/tanf(x))
# define SC_Sec(x) (1.0/cosf(x))
# define SC_Csc(x) (1.0/sinf(x))
# define SC_Asin(x) asinf(x)
# define SC_Acos(x) acosf(x)
# define SC_Atan(x) atanf(x)
# define SC_Atan2(y,x) atan2f(y,x)
# define SC_Fabs(x) fabsf(x)
# define SC_Pow(x,y) powf((x),(y))
#endif

#define SC_Radians(x) ((x)/360.0*2.0*M_PI)
#define SC_Degrees(x) (360.0*((x)/(2.0*M_PI)))

#ifndef SC_TINY_VAL
#define SC_TINY_VAL	1.0e-22
#endif

__BEGIN_DECLS
__inline__ SC_Real SC_Rad2Deg(SC_Real);
__inline__ SC_Real SC_Deg2Rad(SC_Real);

SC_Real SC_ReadReal(AG_Netbuf *);
void	SC_CopyReal(AG_Netbuf *, SC_Real *);
void	SC_WriteReal(AG_Netbuf *, SC_Real);
#define SC_ReadTime(buf) SC_ReadReal(buf)
#define SC_CopyTime(buf,t) SC_CopyReal((buf),(t))
#define SC_WriteTime(buf,t) SC_WriteReal((buf),(t))
#define SC_ReadQTime(buf) (SC_QTime)AG_ReadUint32(buf)
#define SC_CopyQTime(buf,t) AG_CopyUint32((buf),(t))
#define SC_WriteQTime(buf,t) AG_WriteUint32((buf),(Uint32)(t))

SC_Range	SC_ReadRange(AG_Netbuf *);
void		SC_CopyRange(AG_Netbuf *, SC_Range *);
void		SC_WriteRange(AG_Netbuf *, SC_Range);
SC_QTimeRange	SC_ReadQTimeRange(AG_Netbuf *);
void		SC_CopyQTimeRange(AG_Netbuf *, SC_QTimeRange *);
void		SC_WriteQTimeRange(AG_Netbuf *, SC_QTimeRange);

__END_DECLS
