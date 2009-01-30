/*	Public domain	*/

/*
 * Legacy Widget interfaces, mostly related to Widget bindings (which have
 * been replaced by AG_Variable(3) as of Agar-1.3.4).
 */

#define AG_WIDGET_NONE		AG_VARIABLE_P_NULL
#define AG_WIDGET_BOOL		AG_VARIABLE_P_INT
#define AG_WIDGET_UINT		AG_VARIABLE_P_UINT
#define AG_WIDGET_INT		AG_VARIABLE_P_INT
#define AG_WIDGET_UINT8		AG_VARIABLE_P_UINT8
#define AG_WIDGET_SINT8		AG_VARIABLE_P_SINT8
#define AG_WIDGET_UINT16	AG_VARIABLE_P_UINT16
#define AG_WIDGET_SINT16	AG_VARIABLE_P_SINT16
#define AG_WIDGET_UINT32	AG_VARIABLE_P_UINT32
#define AG_WIDGET_SINT32	AG_VARIABLE_P_SINT32
#define AG_WIDGET_UINT64	AG_VARIABLE_P_UINT64
#define AG_WIDGET_SINT64	AG_VARIABLE_P_SINT64
#define AG_WIDGET_FLOAT		AG_VARIABLE_P_FLOAT
#define AG_WIDGET_DOUBLE	AG_VARIABLE_P_DOUBLE
#define AG_WIDGET_LONG_DOUBLE	AG_VARIABLE_P_LONG_DOUBLE
#define AG_WIDGET_STRING	AG_VARIABLE_P_STRING
#define AG_WIDGET_POINTER	AG_VARIABLE_P_POINTER
#define AG_WIDGET_FLAG		AG_VARIABLE_P_FLAG
#define AG_WIDGET_FLAG8		AG_VARIABLE_P_FLAG8
#define AG_WIDGET_FLAG16	AG_VARIABLE_P_FLAG16
#define AG_WIDGET_FLAG32	AG_VARIABLE_P_FLAG32

#define AG_WidgetBinding	AG_Variable
#define AG_WidgetBindingType	AG_VariableType
#define ag_widget_binding	ag_variable
#define ag_widget_binding_type	ag_variable_type

#define AG_WidgetBindBool(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_BOOL,(p))
#define AG_WidgetBindInt(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_INT,(p))
#define AG_WidgetBindUint(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_UINT,(p))
#define AG_WidgetBindUint8(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_UINT8,(p))
#define AG_WidgetBindSint8(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_SINT8,(p))
#define AG_WidgetBindUint16(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_UINT16,(p))
#define AG_WidgetBindSint16(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_SINT16,(p))
#define AG_WidgetBindUint32(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_UINT32,(p))
#define AG_WidgetBindSint32(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_SINT32,(p))
#define AG_WidgetBindFloat(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_FLOAT,(p))
#define AG_WidgetBindDouble(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_DOUBLE,(p))
#define AG_WidgetBindPointer(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_POINTER,(p))
#define AG_WidgetBindString(w,b,p,len) AG_WidgetBind((w),(b),AG_WIDGET_STRING,(p),(len))
#define AG_WidgetBindFlag(w,b,p,mask) AG_WidgetBind((w),(b),AG_WIDGET_FLAG,(p),(mask))
#define AG_WidgetBindFlag8(w,b,p,mask) AG_WidgetBind((w),(b),AG_WIDGET_FLAG8,(p),(mask))
#define AG_WidgetBindFlag16(w,b,p,mask) AG_WidgetBind((w),(b),AG_WIDGET_FLAG16,(p),(mask))
#define AG_WidgetBindFlag32(w,b,p,mask) AG_WidgetBind((w),(b),AG_WIDGET_FLAG32,(p),(mask))
#define AG_WidgetBool AG_WidgetInt
#define AG_WidgetSetBool AG_WidgetSetInt
#define AG_WidgetSetString AG_
#define AG_WidgetBindingChanged(b)
#define AG_WidgetGetBinding(obj,name,p) AG_GetVariable((obj),(name),(p))
#ifdef AG_THREADS
#define AG_WidgetLockBinding(V) AG_LockVariable(V)
#define AG_WidgetUnlockBinding(V) AG_UnlockVariable(V)
#endif

__BEGIN_DECLS
AG_Variable *AG_WidgetBind(void *, const char *, AG_VariableType, ...);
AG_Variable *AG_WidgetBindMp(void *, const char *, AG_Mutex *, AG_VariableType, ...);
int AG_WidgetCopyBinding(void *, const char *, AG_Variable *);
size_t AG_WidgetCopyString(void *, const char *, char *, size_t);
int AG_WidgetInt(void *, const char *);
Uint AG_WidgetUint(void *, const char *);
Uint8 AG_WidgetUint8(void *, const char *);
Sint8 AG_WidgetSint8(void *, const char *);
Uint16 AG_WidgetUint16(void *, const char *);
Sint16 AG_WidgetSint16(void *, const char *);
Uint32 AG_WidgetUint32(void *, const char *);
Sint32 AG_WidgetSint32(void *, const char *);
float AG_WidgetFloat(void *, const char *);
double AG_WidgetDouble(void *, const char *);
char *AG_WidgetString(void *, const char *);
void *AG_WidgetPointer(void *, const char *);
void AG_WidgetSetInt(void *, const char *, int);
void AG_WidgetSetUint(void *, const char *, Uint);
void AG_WidgetSetUint8(void *, const char *, Uint8);
void AG_WidgetSetSint8(void *, const char *, Sint8);
void AG_WidgetSetUint16(void *, const char *, Uint16);
void AG_WidgetSetSint16(void *, const char *, Sint16);
void AG_WidgetSetUint32(void *, const char *, Uint32);
void AG_WidgetSetSint32(void *, const char *, Sint32);
void AG_WidgetSetFloat(void *, const char *, float);
void AG_WidgetSetDouble(void *, const char *, double);
void AG_WidgetSetPointer(void *, const char *, void *);
__END_DECLS
