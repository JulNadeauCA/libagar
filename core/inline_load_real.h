/*	Public domain	*/
/*
 * Serialization of floating-point numbers.
 */

/*
 * single-precision floats
 */
#ifdef AG_INLINE_HEADER
static __inline__ float
AG_ReadFloat(AG_DataSource *_Nonnull ds)
#else
float
ag_read_float(AG_DataSource *ds)
#endif
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

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteFloat(AG_DataSource *_Nonnull ds, float x)
#else
void
ag_write_float(AG_DataSource *ds, float x)
#endif
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_FLOAT); }
#endif
	if (AG_Write(ds, &x, sizeof(float)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteFloatAt(AG_DataSource *_Nonnull ds, float x, AG_Offset pos)
#else
void
ag_write_float_at(AG_DataSource *ds, float x, AG_Offset pos)
#endif
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_FLOAT, pos); }
#endif
	if (AG_WriteAt(ds, &x, sizeof(float), AG_WRITEAT_OFFSET(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

/*
 * double-precision floats
 */
#ifdef AG_INLINE_HEADER
static __inline__ double
AG_ReadDouble(AG_DataSource *_Nonnull ds)
#else
double
ag_read_double(AG_DataSource *ds)
#endif
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

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteDouble(AG_DataSource *_Nonnull ds, double x)
#else
void
ag_write_double(AG_DataSource *ds, double x)
#endif
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_DOUBLE); }
#endif
	if (AG_Write(ds, &x, sizeof(double)) != 0)
		AG_DataSourceError(ds, NULL); 
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteDoubleAt(AG_DataSource *_Nonnull ds, double x, AG_Offset pos)
#else
void
ag_write_double_at(AG_DataSource *ds, double x, AG_Offset pos)
#endif
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_DOUBLE, pos); }
#endif
	if (AG_WriteAt(ds, &x, sizeof(double), AG_WRITEAT_OFFSET(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

/*
 * quad-precision floats
 */
#ifdef AG_HAVE_LONG_DOUBLE

# ifdef AG_INLINE_HEADER
static __inline__ long double
AG_ReadLongDouble(AG_DataSource *_Nonnull ds)
# else
long double
ag_read_long_double(AG_DataSource *ds)
# endif
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

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteLongDouble(AG_DataSource *_Nonnull ds, long double x)
# else
void
ag_write_long_double(AG_DataSource *ds, long double x)
# endif
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_LONG_DOUBLE); }
#endif
	if (AG_Write(ds, &x, sizeof(long double)) != 0)
		AG_DataSourceError(ds, NULL);
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteLongDoubleAt(AG_DataSource *_Nonnull ds, long double x, AG_Offset pos)
# else
void
ag_write_long_double_at(AG_DataSource *ds, long double x, AG_Offset pos)
# endif
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_LONG_DOUBLE, pos); }
#endif
	if (AG_WriteAt(ds, &x, sizeof(long double), AG_WRITEAT_OFFSET(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}
#endif /* AG_HAVE_LONG_DOUBLE */
