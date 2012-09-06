/*	Public domain	*/
/*
 * Serialization of floating-point numbers.
 */

#ifndef	_AGAR_CORE_LOAD_REAL_H_
#define	_AGAR_CORE_LOAD_REAL_H_

#include <agar/config/have_long_double.h>
#include <agar/core/byteswap.h>

#include <agar/core/begin.h>

__BEGIN_DECLS

/*
 * single-precision floats
 */
static __inline__ float
AG_ReadFloat(AG_DataSource *ds)
{
	float f;
	
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_FLOAT) == -1) {
		return (0.0f);
	}
	if (AG_Read(ds, &f, sizeof(f)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0.0f);
	}
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEFLT(f) :
	                                              AG_SwapLEFLT(f));
}
static __inline__ int
AG_ReadFloatv(AG_DataSource *ds, float *fv)
{
	float f;

	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_FLOAT) == -1) {
		return (-1);
	}
	if (AG_Read(ds, &f, sizeof(f)) != 0) {
		return (-1);
	}
	*fv = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEFLT(f) :
	                                            AG_SwapLEFLT(f);
	return (0);
}
static __inline__ void
AG_WriteFloat(AG_DataSource *ds, float fv)
{
	float f = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEFLT(fv) :
	                                                AG_SwapLEFLT(fv);

	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_FLOAT); }
	if (AG_Write(ds, &f, sizeof(f)) != 0)
		AG_DataSourceError(ds, NULL);
}
static __inline__ int
AG_WriteFloatv(AG_DataSource *ds, float *fv)
{
	float f = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEFLT(*fv) :
	                                                AG_SwapLEFLT(*fv);

	if (ds->debug && AG_WriteTypeCodeE(ds, AG_SOURCE_FLOAT) == -1) {
		return (-1);
	}
	return AG_Write(ds, &f, sizeof(f));
}
static __inline__ void
AG_WriteFloatAt(AG_DataSource *ds, float fv, off_t pos)
{
	float f = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEFLT(fv) :
	                                                AG_SwapLEFLT(fv);

	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_FLOAT, pos); }
	if (AG_WriteAt(ds, &f, sizeof(f), AG_WRITEAT_DEBUGOFFS(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

/*
 * double-precision floats
 */
static __inline__ double
AG_ReadDouble(AG_DataSource *ds)
{
	double f;

	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_DOUBLE) == -1) {
		return (0.0);
	}
	if (AG_Read(ds, &f, sizeof(f)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0.0);
	}
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEDBL(f) :
	                                              AG_SwapLEDBL(f));
}
static __inline__ int
AG_ReadDoublev(AG_DataSource *ds, double *fv)
{
	double f;

	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_DOUBLE) == -1) {
		return (-1);
	}
	if (AG_Read(ds, &f, sizeof(f)) != 0) {
		return (-1);
	}
	*fv = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEDBL(f) :
	                                            AG_SwapLEDBL(f);
	return (0);
}
static __inline__ void
AG_WriteDouble(AG_DataSource *ds, double fv)
{
	double f = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEDBL(fv) :
	                                                AG_SwapLEDBL(fv);

	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_DOUBLE); }
	if (AG_Write(ds, &f, sizeof(f)) != 0)
		AG_DataSourceError(ds, NULL); 
}
static __inline__ int
AG_WriteDoublev(AG_DataSource *ds, double *fv)
{
	double f = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEDBL(*fv) :
	                                                 AG_SwapLEDBL(*fv);

	if (ds->debug && AG_WriteTypeCodeE(ds, AG_SOURCE_DOUBLE) == -1) {
		return (-1);
	}
	return AG_Write(ds, &f, sizeof(f));
}
static __inline__ void
AG_WriteDoubleAt(AG_DataSource *ds, double fv, off_t pos)
{
	double f = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEDBL(fv) :
	                                                 AG_SwapLEDBL(fv);

	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_DOUBLE, pos); }
	if (AG_WriteAt(ds, &f, sizeof(f), AG_WRITEAT_DEBUGOFFS(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_HAVE_LONG_DOUBLE
/*
 * quad-precision floats
 */
static __inline__ long double
AG_ReadLongDouble(AG_DataSource *ds)
{
	long double f;

	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_LONG_DOUBLE) == -1) {
		return (0.0l);
	}
	if (AG_Read(ds, &f, sizeof(f)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0.0l);
	}
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBELDBL(f) :
	                                              AG_SwapLELDBL(f));
}
static __inline__ int
AG_ReadLongDoublev(AG_DataSource *ds, long double *fv)
{
	long double f;

	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_LONG_DOUBLE) == -1) {
		return (-1);
	}
	if (AG_Read(ds, &f, sizeof(f)) != 0) {
		return (-1);
	}
	*fv = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBELDBL(f) :
	                                            AG_SwapLELDBL(f);
	return (0);
}
static __inline__ void
AG_WriteLongDouble(AG_DataSource *ds, long double fv)
{
	long double f = (ds->byte_order==AG_BYTEORDER_BE) ? AG_SwapBELDBL(fv) :
	                                                    AG_SwapLELDBL(fv);

	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_LONG_DOUBLE); }
	if (AG_Write(ds, &f, sizeof(f)) != 0)
		AG_DataSourceError(ds, NULL);
}
static __inline__ int
AG_WriteLongDoublev(AG_DataSource *ds, long double *fv)
{
	long double f = (ds->byte_order==AG_BYTEORDER_BE) ? AG_SwapBELDBL(*fv) :
	                                                    AG_SwapLELDBL(*fv);

	if (ds->debug && AG_WriteTypeCodeE(ds, AG_SOURCE_LONG_DOUBLE) == -1) {
		return (-1);
	}
	return AG_Write(ds, &f, sizeof(f));
}
static __inline__ void
AG_WriteLongDoubleAt(AG_DataSource *ds, long double fv, off_t pos)
{
	long double f = (ds->byte_order==AG_BYTEORDER_BE) ? AG_SwapBELDBL(fv) :
	                                                    AG_SwapLELDBL(fv);

	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_LONG_DOUBLE, pos); }
	if (AG_WriteAt(ds, &f, sizeof(f), AG_WRITEAT_DEBUGOFFS(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}
#endif /* AG_HAVE_LONG_DOUBLE */

__END_DECLS
#include <agar/core/close.h>

#endif /* _AGAR_CORE_LOAD_REAL_H_ */
