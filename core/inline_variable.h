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
	if (name[0] != '\0') {
#ifdef AG_DEBUG
		if (AG_Strlcpy(V->name, name, sizeof(V->name)) >= sizeof(V->name))
			AG_FatalError("Variable name too long");
#else
		AG_Strlcpy(V->name, name, sizeof(V->name));
#endif
	} else {
#ifdef AG_DEBUG
		memset(V->name, '\0', sizeof(V->name));
#else
		V->name[0] = '\0';
#endif
	}
	V->type = type;
#ifdef AG_THREADS
	V->mutex = NULL;
#endif
#ifdef AG_DEBUG
	memset(&V->info, 0, sizeof(V->info));
	memset(&V->data, 0, sizeof(V->data));
#else
	V->info.pFlags = 0;
	V->data.p = NULL;
#endif
}

#ifdef AG_THREADS
/* Acquire any locking device associated with a variable. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_LockVariable(AG_Variable *_Nonnull V)
#else
void
ag_lock_variable(AG_Variable *V)
#endif
{
	if (V->mutex != NULL)
		AG_MutexLock(V->mutex);
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
	if (V->mutex != NULL)
		AG_MutexUnlock(V->mutex);
}
#endif /* AG_THREADS */

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
