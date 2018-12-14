/*	Public domain	*/

#define AG_ObjectFreeDataset(o)     AG_ObjectReset(o)
#define AG_ObjectIsClass(o,c)       AG_OfClass((o),(c))
#define AG_GetStringCopy(o,n,b,s)   AG_GetString((o),(n),(b),(s))

/* Redundant const pointer types removed from AG_Variable(3) in Agar 1.6.0 */
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

/* Improved AG_Timer(3) API replaced AG_Timeout in Agar 1.5.0 */
#define AG_LockTimeouts(o)   AG_LockTimers(o)
#define AG_UnlockTimeouts(o) AG_UnlockTimers(o)
void AG_SetTimeout(AG_Timeout *_Nonnull, Uint32 (*_Nonnull)(void *_Nonnull, Uint32, void *_Nullable), void *_Nullable, Uint) DEPRECATED_ATTRIBUTE;
void AG_ScheduleTimeout(void *_Nullable, AG_Timeout *_Nonnull, Uint32) DEPRECATED_ATTRIBUTE;
# define AG_TIMEOUT_INITIALIZER { -1, NULL, 0, 0, 0, NULL }
# define AG_TIMEOUTS_QUEUED() (!AG_TAILQ_EMPTY(&agTimerObjQ))
# define AG_CANCEL_ONDETACH	0x10	/* Don't cancel on ObjectDetach() */
# define AG_CANCEL_ONLOAD	0x20	/* In queue for execution */
# define AG_TimeoutWait(obj,to,d) AG_TimerWait((obj),(to),(d))
# define AG_TimeoutIsScheduled(obj,to) AG_TimerIsRunning((obj),(to))
# define AG_DelTimeout(obj,to) AG_DelTimer((obj),(to))
# define AG_ProcessTimeout(x) AG_ProcessTimeouts(x)
# define AG_AddTimeout(p,to,dt) AG_ScheduleTimeout((p),(to),(dt))
# define AG_ReplaceTimeout(p,to,dt) AG_ScheduleTimeout((p),(to),(dt))

/* AG_Prop was replaced by AG_Variable in Agar 1.3.4 */
#define AG_Prop AG_Variable
#define ag_prop ag_variable
#define ag_prop_type ag_variable_type
#define AG_ObjectFreeProps(o) AG_ObjectFreeVariables(o)
#define AG_PropLoad(o,ds)     AG_ObjectLoadVariables((o),(ds))
#define AG_PropSave(o,ds)     AG_ObjectSaveVariables((o),(ds))
#define AG_PropDefined(o,n)   AG_Defined((o),(n))
#define AG_OBJECT_RELOAD_PROPS AG_OBJECT_FLOATING_VARS
#define AG_PROP_UINT    AG_VARIABLE_UINT
#define AG_PROP_INT     AG_VARIABLE_INT
#define AG_PROP_UINT8   AG_VARIABLE_UINT8
#define AG_PROP_SINT8   AG_VARIABLE_SINT8
#define AG_PROP_UINT16	AG_VARIABLE_UINT16
#define AG_PROP_SINT16	AG_VARIABLE_SINT16
#define AG_PROP_UINT32	AG_VARIABLE_UINT32
#define AG_PROP_SINT32	AG_VARIABLE_SINT32
#define AG_PROP_FLOAT   AG_VARIABLE_FLOAT
#define AG_PROP_DOUBLE	AG_VARIABLE_DOUBLE
#define AG_PROP_STRING	AG_VARIABLE_STRING
#define AG_PROP_POINTER AG_VARIABLE_POINTER
#define AG_PROP_BOOL    AG_VARIABLE_INT

__BEGIN_DECLS
AG_Prop	*_Nullable AG_SetProp(void *_Nonnull, const char *_Nonnull,
                              enum ag_prop_type, ...)
			     DEPRECATED_ATTRIBUTE;

AG_Prop	*_Nullable AG_GetProp(void *_Nonnull, const char *_Nonnull, int,
                              void *_Nonnull)
			     DEPRECATED_ATTRIBUTE;

AG_Variable *_Nullable AG_GetVariableLocked(void *_Nonnull,
                                            const char *_Nonnull)
					   DEPRECATED_ATTRIBUTE;
__END_DECLS
