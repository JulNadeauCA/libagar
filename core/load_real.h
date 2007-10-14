/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/have_long_double.h>
#else
#include <agar/config/have_long_double.h>
#endif

#include "begin_code.h"

__BEGIN_DECLS

static __inline__ float
AG_ReadFloat(AG_Netbuf *buf)
{
	float f;
	AG_NetbufRead(&f, sizeof(f), 1, buf);
	return (f);
}

static __inline__ void
AG_WriteFloat(AG_Netbuf *buf, float f)
{
	AG_NetbufWrite(&f, sizeof(f), 1, buf);
}

static __inline__ void
AG_PwriteFloat(AG_Netbuf *buf, float f, off_t offs)
{
	AG_NetbufPwrite(&f, sizeof(f), 1, offs, buf);
}

static __inline__ double
AG_ReadDouble(AG_Netbuf *buf)
{
	double f;
	AG_NetbufRead(&f, sizeof(f), 1, buf);
	return (f);
}

static __inline__ void
AG_WriteDouble(AG_Netbuf *buf, double f)
{
	AG_NetbufWrite(&f, sizeof(f), 1, buf);
}

static __inline__ void
AG_PwriteDouble(AG_Netbuf *buf, double f, off_t offs)
{
	AG_NetbufPwrite(&f, sizeof(f), 1, offs, buf);
}

#ifdef HAVE_LONG_DOUBLE
static __inline__ long double
AG_ReadLongDouble(AG_Netbuf *buf)
{
	long double f;
	AG_NetbufRead(&f, sizeof(f), 1, buf);
	return (f);
}
static __inline__ void
AG_WriteLongDouble(AG_Netbuf *buf, long double f)
{
	AG_NetbufWrite(&f, sizeof(f), 1, buf);
}
static __inline__ void
AG_PwriteLongDouble(AG_Netbuf *buf, long double f, off_t offs)
{
	AG_NetbufPwrite(&f, sizeof(f), 1, offs, buf);
}
#endif /* HAVE_LONG_DOUBLE */

__END_DECLS
#include "close_code.h"
