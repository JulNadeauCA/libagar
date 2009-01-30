/*	Public domain	*/

/*
 * LEGACY Interface to AG_Variable(3).
 */
#include <agar/core/begin.h>

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
#define  AG_PropLoad AG_ObjectLoadVariables
#define  AG_PropSave AG_ObjectSaveVariables
AG_Prop	*AG_SetProp(void *, const char *, enum ag_prop_type, ...);
AG_Prop	*AG_GetProp(void *, const char *, int, void *);

#define AG_PropDefined	 AG_Defined
#define AG_GetStringCopy AG_GetString
__END_DECLS

#include <agar/core/close.h>
