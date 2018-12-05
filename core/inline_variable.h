/*	Public domain	*/

/* Initialize the generic part of an AG_Variable. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_InitVariable(AG_Variable *_Nonnull V, AG_VariableType type,
    const char *_Nonnull name)
#else
void
ag_init_variable(AG_Variable *V, AG_VariableType type, const char *name)
#endif
{
#if 0 /* for better gdbabillity */
	memset(V->name, '\0', sizeof(V->name));
#endif
	if (name[0] != '\0') {
		AG_Strlcpy(V->name, name, sizeof(V->name));
	} else {
		V->name[0] = '\0';
	}
	V->type = type;
	V->mutex = NULL;
	V->fn.fnVoid = NULL;
	V->info.size = 0;
	V->info.varName = NULL;
	V->data.s = NULL;
}

/* Acquire any locking device associated with a variable. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_LockVariable(AG_Variable *_Nonnull V)
#else
void
ag_lock_variable(AG_Variable *V)
#endif
{
#ifdef AG_THREADS
	if (V->mutex != NULL) { AG_MutexLock(V->mutex); }
#else
# ifdef __CC65__
	if (V != NULL) { /* Unused */ }
# endif
#endif
}

/* Release any locking device associated with a variable. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_UnlockVariable(AG_Variable *_Nonnull V)
#else
void
ag_unlock_variable(AG_Variable *V)
#endif
{
#ifdef AG_THREADS
	if (V->mutex != NULL) { AG_MutexUnlock(V->mutex); }
#else
# ifdef __CC65__
	if (V != NULL) { /* Unused */ }
# endif
#endif
}

/* Release all resources allocated by a variable. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_FreeVariable(AG_Variable *_Nonnull V)
#else
void
ag_free_variable(AG_Variable *V)
#endif
{
	switch (V->type) {
	case AG_VARIABLE_STRING:
		if (V->info.size == 0) {
			AG_Free(V->data.s);
		}
		break;
	case AG_VARIABLE_P_VARIABLE:
		AG_Free(V->info.varName);
		break;
	default:
		break;
	}
}
