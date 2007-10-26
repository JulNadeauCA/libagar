/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/have_long_double.h>
#else
#include <agar/config/have_long_double.h>
#endif

#include "begin_code.h"

__BEGIN_DECLS

static __inline__ float
AG_ReadFloat(AG_DataSource *ds)
{
	float f;
	if (AG_Read(ds, &f, sizeof(f), 1) != 0) { AG_FatalError(NULL); }
	return (f);
}
static __inline__ void
AG_WriteFloat(AG_DataSource *ds, float f)
{
	if (AG_Write(ds, &f, sizeof(f), 1) != 0) { AG_FatalError(NULL); }
}
static __inline__ void
AG_WriteFloatAt(AG_DataSource *ds, float f, off_t pos)
{
	if (AG_WriteAt(ds, &f, sizeof(f), 1, pos) != 0) { AG_FatalError(NULL); }
}
static __inline__ double
AG_ReadDouble(AG_DataSource *ds)
{
	double f;
	if (AG_Read(ds, &f, sizeof(f), 1) != 0) { AG_FatalError(NULL); }
	return (f);
}
static __inline__ void
AG_WriteDouble(AG_DataSource *ds, double f)
{
	if (AG_Write(ds, &f, sizeof(f), 1) != 0) { AG_FatalError(NULL); }
}
static __inline__ void
AG_WriteDoubleAt(AG_DataSource *ds, double f, off_t pos)
{
	if (AG_WriteAt(ds, &f, sizeof(f), 1, pos) != 0) { AG_FatalError(NULL); }
}

#ifdef HAVE_LONG_DOUBLE
static __inline__ long double
AG_ReadLongDouble(AG_DataSource *ds)
{
	long double f;
	if (AG_Read(ds, &f, sizeof(f), 1) != 0) { AG_FatalError(NULL); }
	return (f);
}
static __inline__ void
AG_WriteLongDouble(AG_DataSource *ds, long double f)
{
	if (AG_Write(ds, &f, sizeof(f), 1) != 0) { AG_FatalError(NULL); }
}
static __inline__ void
AG_WriteLongDoubleAt(AG_DataSource *ds, long double f, off_t pos)
{
	if (AG_WriteAt(ds, &f, sizeof(f), 1, pos) != 0) { AG_FatalError(NULL); }
}
#endif /* HAVE_LONG_DOUBLE */

__END_DECLS
#include "close_code.h"
