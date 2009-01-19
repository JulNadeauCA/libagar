/*	Public domain	*/

#include <agar/core/begin.h>

/*
 * LEGACY Interface to AG_Variable(3).
 */

#define AG_Prop		AG_Variable
#define ag_prop		ag_variable
#define ag_prop_type	ag_variable_type

#define AG_PROP_UINT	AG_VARIABLE_UINT
#define AG_PROP_INT	AG_VARIABLE_INT
#define AG_PROP_UINT8	AG_VARIABLE_UINT8
#define AG_PROP_SINT8	AG_VARIABLE_SINT8
#define AG_PROP_UINT16	AG_VARIABLE_UINT16
#define AG_PROP_SINT16	AG_VARIABLE_SINT16
#define AG_PROP_UINT32	AG_VARIABLE_UINT32
#define AG_PROP_SINT32	AG_VARIABLE_SINT32
#define AG_PROP_FLOAT	AG_VARIABLE_FLOAT
#define AG_PROP_DOUBLE	AG_VARIABLE_DOUBLE
#define AG_PROP_STRING	AG_VARIABLE_STRING
#define AG_PROP_POINTER	AG_VARIABLE_POINTER
#define AG_PROP_BOOL	AG_VARIABLE_INT

__BEGIN_DECLS
int	 AG_PropLoad(void *, AG_DataSource *);
int	 AG_PropSave(void *, AG_DataSource *);
AG_Prop	*AG_SetProp(void *, const char *, enum ag_prop_type, ...);

AG_Prop	 *AG_GetProp(void *, const char *, int, void *);
AG_Prop	 *AG_FindProp(void *, const char *, int, void *);
int	  AG_PropCopyPath(char *, size_t, void *, const char *)
		          BOUNDED_ATTRIBUTE(__string__, 1, 2);
size_t	  AG_GetStringCopy(void *, const char *, char *, size_t);
int       AG_PropDefined(void *, const char *);

/* LEGACY */
#define AG_SetBool AG_SetInt
#define AG_PropPrint AG_VariablePrint

static __inline__ Uint
AG_GetUint(void *p, const char *key)
{
	Uint i;
	if (AG_GetProp(p, key, AG_PROP_UINT, (Uint *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ int
AG_GetInt(void *p, const char *key)
{
	int i;
	if (AG_GetProp(p, key, AG_PROP_INT, (int *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ int
AG_GetBool(void *p, const char *key)
{
	int i;
	if (AG_GetProp(p, key, AG_PROP_BOOL, (int *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Uint8
AG_GetUint8(void *p, const char *key)
{
	Uint8 i;
	if (AG_GetProp(p, key, AG_PROP_UINT8, (Uint8 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Sint8
AG_GetSint8(void *p, const char *key)
{
	Sint8 i;
	if (AG_GetProp(p, key, AG_PROP_SINT8, (Sint8 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Uint16
AG_GetUint16(void *p, const char *key)
{
	Uint16 i;
	if (AG_GetProp(p, key, AG_PROP_UINT16, (Uint16 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Sint16
AG_GetSint16(void *p, const char *key)
{
	Sint16 i;
	if (AG_GetProp(p, key, AG_PROP_SINT16, (Sint16 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Uint32
AG_GetUint32(void *p, const char *key)
{
	Uint32 i;
	if (AG_GetProp(p, key, AG_PROP_UINT32, (Uint32 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Sint32
AG_GetSint32(void *p, const char *key)
{
	Sint32 i;
	if (AG_GetProp(p, key, AG_PROP_SINT32, (Sint32 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}

static __inline__ float
AG_GetFloat(void *p, const char *key)
{
	float f;
	if (AG_GetProp(p, key, AG_PROP_FLOAT, (float *)&f) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (f);
}
static __inline__ double
AG_GetDouble(void *p, const char *key)
{
	double d;
	if (AG_GetProp(p, key, AG_PROP_DOUBLE, (double *)&d) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (d);
}

static __inline__ char *
AG_GetString(void *p, const char *key)
{
	char *s;
	if (AG_GetProp(p, key, AG_PROP_STRING, (char *)&s) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (s);
}

static __inline__ void *
AG_GetPointer(void *p, const char *key)
{
	void *np;
	if (AG_GetProp(p, key, AG_PROP_POINTER, (void *)&np) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (np);
}
__END_DECLS

#include <agar/core/close.h>
