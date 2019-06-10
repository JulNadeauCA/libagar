/*	Public domain	*/

/* Calls renamed for clarity in 1.6.0 */
#define AG_ObjectFreeDataset(o)     AG_ObjectReset(o)
#define AG_ObjectIsClass(o,c)       AG_OfClass((o),(c))
#define AG_GetStringCopy(o,n,b,s)   AG_GetString((o),(n),(b),(s))
#define AG_PrtString                AG_SetStringF

/* Redundant const pointer types removed from AG_Variable in 1.6.0 */
#define AG_VARIABLE_CONST_STRING	AG_VARIABLE_STRING
#define AG_VARIABLE_P_CONST_STRING	AG_VARIABLE_P_STRING
#define AG_VARIABLE_CONST_POINTER	AG_VARIABLE_POINTER
#define AG_VARIABLE_P_CONST_POINTER	AG_VARIABLE_P_POINTER
#define AG_SetConstString(o,n,v)	AG_SetString((o),(n),(char *)(v))
#define AG_BindConstString		AG_BindString
#define AG_BindConstStringMp		AG_BindStringMp
#define AG_BindConstStringFn		AG_BindStringFn
#define AG_GetConstPointer(o,x)		((const void *)AG_GetPointer((o),(x)))
#define AG_SetConstPointer(o,n,v)	AG_SetPointer((o),(n),(void *)(v))
#define AG_InitConstPointer(var,v)	AG_InitPointer((var),(void *)(v))
#define AG_BindConstPointer		AG_BindPointer
#define AG_BindConstPointerFn		AG_BindPointerFn
#define AG_BindConstPointerMp		AG_BindPointerMp
