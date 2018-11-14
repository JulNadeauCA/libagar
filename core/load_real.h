/*	Public domain	*/
/*
 * Serialization of floating-point numbers.
 */

#ifndef	_AGAR_CORE_LOAD_REAL_H_
#define	_AGAR_CORE_LOAD_REAL_H_
#include <agar/core/begin.h>
#ifdef AG_HAVE_FLOAT

__BEGIN_DECLS
/*
 * single-precision floats
 */
static __inline__ float
AG_ReadFloat(AG_DataSource *_Nonnull ds)
{
	float f;

#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_FLOAT) == -1)
		return (0.0f);
#endif
	if (AG_Read(ds, &f, sizeof(float)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0.0f);
	}
	return (f);
}
static __inline__ int
AG_ReadFloatv(AG_DataSource *_Nonnull ds, float *_Nonnull x)
{
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_FLOAT) == -1)
		return (-1);
#endif
	return AG_Read(ds, x, sizeof(float));
}
static __inline__ void
AG_WriteFloat(AG_DataSource *_Nonnull ds, float x)
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_FLOAT); }
#endif
	if (AG_Write(ds, &x, sizeof(float)) != 0)
		AG_DataSourceError(ds, NULL);
}
static __inline__ int
AG_WriteFloatv(AG_DataSource *_Nonnull ds, float *_Nonnull x)
{
#ifdef AG_DEBUG
	if (ds->debug && AG_WriteTypeCodeE(ds, AG_SOURCE_FLOAT) == -1)
		return (-1);
#endif
	return AG_Write(ds, x, sizeof(float));
}
static __inline__ void
AG_WriteFloatAt(AG_DataSource *_Nonnull ds, float x, AG_Offset pos)
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_FLOAT, pos); }
#endif
	if (AG_WriteAt(ds, &x, sizeof(float), AG_WRITEAT_DEBUGOFFS(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

/*
 * double-precision floats
 */
static __inline__ double
AG_ReadDouble(AG_DataSource *_Nonnull ds)
{
	double f;

#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_DOUBLE) == -1)
		return (0.0);
#endif
	if (AG_Read(ds, &f, sizeof(f)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0.0);
	}
	return (f);
}
static __inline__ int
AG_ReadDoublev(AG_DataSource *_Nonnull ds, double *_Nonnull x)
{
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_DOUBLE) == -1)
		return (-1);
#endif
	if (AG_Read(ds, x, sizeof(double)) != 0) {
		return (-1);
	}
	return (0);
}
static __inline__ void
AG_WriteDouble(AG_DataSource *_Nonnull ds, double x)
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_DOUBLE); }
#endif
	if (AG_Write(ds, &x, sizeof(double)) != 0)
		AG_DataSourceError(ds, NULL); 
}
static __inline__ int
AG_WriteDoublev(AG_DataSource *_Nonnull ds, double *_Nonnull x)
{
#ifdef AG_DEBUG
	if (ds->debug && AG_WriteTypeCodeE(ds, AG_SOURCE_DOUBLE) == -1)
		return (-1);
#endif
	return AG_Write(ds, x, sizeof(double));
}
static __inline__ void
AG_WriteDoubleAt(AG_DataSource *_Nonnull ds, double x, AG_Offset pos)
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_DOUBLE, pos); }
#endif
	if (AG_WriteAt(ds, &x, sizeof(double), AG_WRITEAT_DEBUGOFFS(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_HAVE_LONG_DOUBLE
/*
 * quad-precision floats
 */
static __inline__ long double
AG_ReadLongDouble(AG_DataSource *_Nonnull ds)
{
	long double f;

#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_LONG_DOUBLE) == -1)
		return (0.0l);
#endif
	if (AG_Read(ds, &f, sizeof(f)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0.0l);
	}
	return (f);
}
static __inline__ int
AG_ReadLongDoublev(AG_DataSource *_Nonnull ds, long double *_Nonnull x)
{
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_LONG_DOUBLE) == -1)
		return (-1);
#endif
	return AG_Read(ds, &x, sizeof(long double));
}
static __inline__ void
AG_WriteLongDouble(AG_DataSource *_Nonnull ds, long double x)
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_LONG_DOUBLE); }
#endif
	if (AG_Write(ds, &x, sizeof(long double)) != 0)
		AG_DataSourceError(ds, NULL);
}
static __inline__ int
AG_WriteLongDoublev(AG_DataSource *_Nonnull ds, long double *_Nonnull x)
{
#ifdef AG_DEBUG
	if (ds->debug && AG_WriteTypeCodeE(ds, AG_SOURCE_LONG_DOUBLE) == -1)
		return (-1);
#endif
	return AG_Write(ds, &x, sizeof(long double));
}
static __inline__ void
AG_WriteLongDoubleAt(AG_DataSource *_Nonnull ds, long double x, AG_Offset pos)
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_LONG_DOUBLE, pos); }
#endif
	if (AG_WriteAt(ds, &x, sizeof(long double), AG_WRITEAT_DEBUGOFFS(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}
#endif /* AG_HAVE_LONG_DOUBLE */

__END_DECLS
#endif /* AG_HAVE_FLOAT */

#include <agar/core/close.h>
#endif /* _AGAR_CORE_LOAD_REAL_H_ */
