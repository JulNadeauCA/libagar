/*	Public domain	*/
/*
 * Serialization of floating-point numbers.
 */

#ifdef _AGAR_INTERNAL
#include <config/have_long_double.h>
#include <core/byteswap.h>
#else
#include <agar/config/have_long_double.h>
#include <agar/core/byteswap.h>
#endif

#include "begin_code.h"

__BEGIN_DECLS

static __inline__ float
AG_ReadFloat(AG_DataSource *ds)
{
	float f;
	if (AG_Read(ds, &f, sizeof(f), 1) != 0) { AG_FatalError(NULL); }
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEFLT(f) :
	                                              AG_SwapLEFLT(f));
}
static __inline__ void
AG_WriteFloat(AG_DataSource *ds, float fv)
{
	float f = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEFLT(fv) :
	                                                AG_SwapLEFLT(fv);
	if (AG_Write(ds, &f, sizeof(f), 1) != 0) { AG_FatalError(NULL); }
}
static __inline__ void
AG_WriteFloatAt(AG_DataSource *ds, float fv, off_t pos)
{
	float f = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEFLT(fv) :
	                                                AG_SwapLEFLT(fv);
	if (AG_WriteAt(ds, &f, sizeof(f), 1, pos) != 0) { AG_FatalError(NULL); }
}
static __inline__ double
AG_ReadDouble(AG_DataSource *ds)
{
	double f;
	if (AG_Read(ds, &f, sizeof(f), 1) != 0) { AG_FatalError(NULL); }
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEDBL(f) :
	                                              AG_SwapLEDBL(f));
}
static __inline__ void
AG_WriteDouble(AG_DataSource *ds, double fv)
{
	float f = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEDBL(fv) :
	                                                AG_SwapLEDBL(fv);
	if (AG_Write(ds, &f, sizeof(f), 1) != 0) { AG_FatalError(NULL); }
}
static __inline__ void
AG_WriteDoubleAt(AG_DataSource *ds, double fv, off_t pos)
{
	float f = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBEDBL(fv) :
	                                                AG_SwapLEDBL(fv);
	if (AG_WriteAt(ds, &f, sizeof(f), 1, pos) != 0) { AG_FatalError(NULL); }
}

#ifdef HAVE_LONG_DOUBLE
static __inline__ long double
AG_ReadLongDouble(AG_DataSource *ds)
{
	long double f;
	if (AG_Read(ds, &f, sizeof(f), 1) != 0) { AG_FatalError(NULL); }
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBELDBL(f) :
	                                              AG_SwapLELDBL(f));
}
static __inline__ void
AG_WriteLongDouble(AG_DataSource *ds, long double fv)
{
	float f = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBELDBL(fv) :
	                                                AG_SwapLELDBL(fv);
	if (AG_Write(ds, &f, sizeof(f), 1) != 0) { AG_FatalError(NULL); }
}
static __inline__ void
AG_WriteLongDoubleAt(AG_DataSource *ds, long double fv, off_t pos)
{
	float f = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBELDBL(fv) :
	                                                AG_SwapLELDBL(fv);
	if (AG_WriteAt(ds, &f, sizeof(f), 1, pos) != 0) { AG_FatalError(NULL); }
}
#endif /* HAVE_LONG_DOUBLE */

__END_DECLS
#include "close_code.h"
