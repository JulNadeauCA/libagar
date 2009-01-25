/*	Public domain	*/

/*
 * Legacy Widget interfaces, for the most part binding-related (bindings
 * have been replaced by AG_Variable(3) in Agar 1.3.4).
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
#define AG_WidgetBindingChanged(b)

__BEGIN_DECLS
AG_Variable *AG_WidgetBind(void *, const char *, AG_VariableType, ...);
AG_Variable *AG_WidgetBindMp(void *, const char *, AG_Mutex *, AG_VariableType, ...);
AG_Variable *AG_WidgetGetBinding(void *, const char *, ...);
int          AG_WidgetCopyBinding(void *, const char *, AG_Variable *);
void         AG_WidgetSetString(void *, const char *, const char *);
size_t       AG_WidgetCopyString(void *, const char *, char *, size_t);

/*
 * Acquire/release lock on a widget binding.
 */
#ifdef AG_THREADS
static __inline__ void
AG_WidgetLockBinding(AG_Variable *V)
{
	if (V->mutex != NULL)
		AG_MutexLock(V->mutex);
}
static __inline__ void
AG_WidgetUnlockBinding(AG_Variable *V)
{
	if (V->mutex != NULL)
		AG_MutexUnlock(V->mutex);
}
#else
# define AG_WidgetLockBinding(b)
# define AG_WidgetUnlockBinding(b)
#endif /* AG_THREADS */

/*
 * Return the value of a widget binding.
 */

static __inline__ int
AG_WidgetInt(void *wid, const char *name)
{
	AG_Variable *b;
	int *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, (int **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

static __inline__ Uint
AG_WidgetUint(void *wid, const char *name)
{
	AG_Variable *b;
	Uint *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, (Uint **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

static __inline__ Uint8
AG_WidgetUint8(void *wid, const char *name)
{
	AG_Variable *b;
	Uint8 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, (Uint8 **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

static __inline__ Sint8
AG_WidgetSint8(void *wid, const char *name)
{
	AG_Variable *b;
	Sint8 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, (Sint8 **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

static __inline__ Uint16
AG_WidgetUint16(void *wid, const char *name)
{
	AG_Variable *b;
	Uint16 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, (Uint16 **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

static __inline__ Sint16
AG_WidgetSint16(void *wid, const char *name)
{
	AG_Variable *b;
	Sint16 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, (Sint16 **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

static __inline__ Uint32
AG_WidgetUint32(void *wid, const char *name)
{
	AG_Variable *b;
	Uint32 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, (Uint32 **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

static __inline__ Sint32
AG_WidgetSint32(void *wid, const char *name)
{
	AG_Variable *b;
	Sint32 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, (Sint32 **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

static __inline__ float
AG_WidgetFloat(void *wid, const char *name)
{
	AG_Variable *b;
	float *f, rv;

	if ((b = AG_WidgetGetBinding(wid, name, (float **)&f)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	rv = *f;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

static __inline__ double
AG_WidgetDouble(void *wid, const char *name)
{
	AG_Variable *b;
	double *d, rv;

	if ((b = AG_WidgetGetBinding(wid, name, (double **)&d)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	rv = *d;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

static __inline__ char *
AG_WidgetString(void *wid, const char *name)
{
	AG_Variable *b;
	char *s, *sd;

	if ((b = AG_WidgetGetBinding(wid, name, (char **)&s)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	sd = AG_Strdup(s);
	AG_WidgetUnlockBinding(b);
	return (sd);
}

static __inline__ void *
AG_WidgetPointer(void *wid, const char *name)
{
	AG_Variable *b;
	void **p, *rv;

	if ((b = AG_WidgetGetBinding(wid, name, (void ***)&p)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	rv = *p;
	AG_WidgetUnlockBinding(b);
	return (p);
}

/*
 * Assign the value of an existing widget binding.
 */

static __inline__ void
AG_WidgetSetInt(void *wid, const char *name, int ni)
{
	AG_Variable *V;
	int *i;

	if ((V = AG_WidgetGetBinding(wid, name, (int **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	if (*i != ni) {
		*i = ni;
	}
	AG_WidgetUnlockBinding(V);
}

static __inline__ void
AG_WidgetSetUint(void *wid, const char *name, Uint ni)
{
	AG_Variable *V;
	Uint *i;

	if ((V = AG_WidgetGetBinding(wid, name, (Uint **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	if (*i != ni) {
		*i = ni;
	}
	AG_WidgetUnlockBinding(V);
}

static __inline__ void
AG_WidgetSetUint8(void *wid, const char *name, Uint8 ni)
{
	AG_Variable *V;
	Uint8 *i;

	if ((V = AG_WidgetGetBinding(wid, name, (Uint8 **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	if (*i != ni) {
		*i = ni;
	}
	AG_WidgetUnlockBinding(V);
}

static __inline__ void
AG_WidgetSetSint8(void *wid, const char *name, Sint8 ni)
{
	AG_Variable *V;
	Sint8 *i;

	if ((V = AG_WidgetGetBinding(wid, name, (Sint8 **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	if (*i != ni) {
		*i = ni;
	}
	AG_WidgetUnlockBinding(V);
}

static __inline__ void
AG_WidgetSetUint16(void *wid, const char *name, Uint16 ni)
{
	AG_Variable *V;
	Uint16 *i;

	if ((V = AG_WidgetGetBinding(wid, name, (Uint16 **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	if (*i != ni) {
		*i = ni;
	}
	AG_WidgetUnlockBinding(V);
}

static __inline__ void
AG_WidgetSetSint16(void *wid, const char *name, Sint16 ni)
{
	AG_Variable *V;
	Sint16 *i;

	if ((V = AG_WidgetGetBinding(wid, name, (Sint16 **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	if (*i != ni) {
		*i = ni;
	}
	AG_WidgetUnlockBinding(V);
}

static __inline__ void
AG_WidgetSetUint32(void *wid, const char *name, Uint32 ni)
{
	AG_Variable *V;
	Uint32 *i;

	if ((V = AG_WidgetGetBinding(wid, name, (Uint32 **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	if (*i != ni) {
		*i = ni;
	}
	AG_WidgetUnlockBinding(V);
}

static __inline__ void
AG_WidgetSetSint32(void *wid, const char *name, Sint32 ni)
{
	AG_Variable *V;
	Sint32 *i;

	if ((V = AG_WidgetGetBinding(wid, name, (Sint32 **)&i)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	if (*i != ni) {
		*i = ni;
	}
	AG_WidgetUnlockBinding(V);
}

static __inline__ void
AG_WidgetSetFloat(void *wid, const char *name, float nf)
{
	AG_Variable *V;
	float *f;

	if ((V = AG_WidgetGetBinding(wid, name, (float **)&f)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	if (*f != nf) {
		*f = nf;
	}
	AG_WidgetUnlockBinding(V);
}

static __inline__ void
AG_WidgetSetDouble(void *wid, const char *name, double nd)
{
	AG_Variable *V;
	double *d;

	if ((V = AG_WidgetGetBinding(wid, name, (double **)&d)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	if (*d != nd) {
		*d = nd;
	}
	AG_WidgetUnlockBinding(V);
}

static __inline__ void
AG_WidgetSetPointer(void *wid, const char *name, void *np)
{
	AG_Variable *V;
	void **p;

	if ((V = AG_WidgetGetBinding(wid, name, (void ***)&p)) == NULL) { AG_FatalError("%s", AG_GetError()); }
	if (*p != np) {
		*p = np;
	}
	AG_WidgetUnlockBinding(V);
}
__END_DECLS
