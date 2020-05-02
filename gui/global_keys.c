/*
 * Copyright (c) 2001-2020 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Application-wide hotkey bindings.
 */

#include <agar/core/core.h>
#include <agar/gui/window.h>

struct ag_global_key {
	AG_KeySym keysym;
	AG_KeyMod keymod;
	void (*_Nullable fn)(void);
	void (*_Nullable fn_ev)(AG_Event *_Nonnull);
	SLIST_ENTRY(ag_global_key) gkeys;
};
static SLIST_HEAD_(ag_global_key) agGlobalKeys;
#ifdef AG_THREADS
static AG_Mutex agGlobalKeysLock;
#endif

/* Initialize the global keys table. */
void
AG_InitGlobalKeys(void)
{
	AG_MutexInitRecursive(&agGlobalKeysLock);
	SLIST_INIT(&agGlobalKeys);
}

/* Clear the global keys table. */
void
AG_DestroyGlobalKeys(void)
{
	AG_ClearGlobalKeys();
	AG_MutexDestroy(&agGlobalKeysLock);
}

/* Tie a global hotkey to a callback function. */
void
AG_BindGlobalKey(AG_KeySym keysym, AG_KeyMod keymod, void (*fn)(void))
{
	struct ag_global_key *gk;

	gk = Malloc(sizeof(struct ag_global_key));
	gk->keysym = keysym;
	gk->keymod = keymod;
	gk->fn = fn;
	gk->fn_ev = NULL;

	AG_MutexLock(&agGlobalKeysLock);
	SLIST_INSERT_HEAD(&agGlobalKeys, gk, gkeys);
	AG_MutexUnlock(&agGlobalKeysLock);
}

/*
 * Set up some standard, platform-specific hotkey bindings for controlling
 * zoom or exiting the application.
 */
void
AG_BindStdGlobalKeys(void)
{
#ifdef __APPLE__
	AG_KeyMod mod = AG_KEYMOD_META;
#else
	AG_KeyMod mod = AG_KEYMOD_CTRL;
#endif
	/* Zoom in, zoom out and reset to 1:1 */
	AG_BindGlobalKey(AG_KEY_EQUALS,	mod,			AG_ZoomIn);
	AG_BindGlobalKey(AG_KEY_PLUS,   mod|AG_KEYMOD_SHIFT,	AG_ZoomIn);
	AG_BindGlobalKey(AG_KEY_MINUS,	mod,			AG_ZoomOut);
	AG_BindGlobalKey(AG_KEY_0,      mod,			AG_ZoomReset);
#ifdef AG_EVENT_LOOP
	/* Terminate the application immediately. */
	AG_BindGlobalKey(AG_KEY_Q,      mod,			AG_QuitGUI);
#endif

	/*
	 * Close the active window and gracefully terminate the application
	 * when there are no more windows.
	 */
	AG_BindGlobalKey(AG_KEY_ESCAPE,	AG_KEYMOD_ANY, AG_CloseFocusedWindow);
}

/* Tie a global hotkey to a callback function (AG_Event style). */
void
AG_BindGlobalKeyEv(AG_KeySym keysym, AG_KeyMod keymod, void (*fn_ev)(AG_Event *))
{
	struct ag_global_key *gk;

	gk = Malloc(sizeof(struct ag_global_key));
	gk->keysym = keysym;
	gk->keymod = keymod;
	gk->fn = NULL;
	gk->fn_ev = fn_ev;
	
	AG_MutexLock(&agGlobalKeysLock);
	SLIST_INSERT_HEAD(&agGlobalKeys, gk, gkeys);
	AG_MutexUnlock(&agGlobalKeysLock);
}

/* Unregister a hotkey. */
int
AG_UnbindGlobalKey(AG_KeySym keysym, AG_KeyMod keymod)
{
	struct ag_global_key *gk;

	AG_MutexLock(&agGlobalKeysLock);

	SLIST_FOREACH(gk, &agGlobalKeys, gkeys) {
		if (gk->keysym == keysym && gk->keymod == keymod) {
			SLIST_REMOVE(&agGlobalKeys, gk, ag_global_key, gkeys);
			AG_MutexUnlock(&agGlobalKeysLock);
			free(gk);
			return (0);
		}
	}

	AG_MutexUnlock(&agGlobalKeysLock);

	AG_SetErrorS("No such key binding");
	return (-1);
}

/* Free the table of hotkeys. */
void
AG_ClearGlobalKeys(void)
{
	struct ag_global_key *gk, *gkNext;

	AG_MutexLock(&agGlobalKeysLock);

	for (gk = SLIST_FIRST(&agGlobalKeys);
	     gk != SLIST_END(&agGlobalKeys);
	     gk = gkNext) {
		gkNext = SLIST_NEXT(gk, gkeys);
		free(gk);
	}
	SLIST_INIT(&agGlobalKeys);

	AG_MutexUnlock(&agGlobalKeysLock);
}

static __inline__ _Const_Attribute int
TestKeyMod(AG_KeyMod mod, AG_KeyMod mask)
{
	if (mod == 0) {
		return (1);
	}
	if ((mod & AG_KEYMOD_CTRL_SHIFT) &&
	    (mask & AG_KEYMOD_CTRL) && (mask & AG_KEYMOD_SHIFT)) {
		return (1);
	}
	if ((mod & AG_KEYMOD_CTRL_ALT) &&
	    (mask & AG_KEYMOD_CTRL) && (mask & AG_KEYMOD_ALT)) {
		return (1);
	}
	return (mod & mask);
}

/* Execute any action tied to a hotkey. */
int
AG_ExecGlobalKeys(AG_KeySym sym, AG_KeyMod mod)
{
	struct ag_global_key *gk;
	int rv = 0;

	AG_MutexLock(&agGlobalKeysLock);

	SLIST_FOREACH(gk, &agGlobalKeys, gkeys) {
		if ((gk->keysym == AG_KEY_ANY    || gk->keysym == sym) &&
		    (gk->keymod == AG_KEYMOD_ANY || TestKeyMod(gk->keymod, mod))) {
			if (gk->fn != NULL) {
				gk->fn();
			} else if (gk->fn_ev != NULL) {
				AG_Event dummy;

				AG_EventInit(&dummy);
				gk->fn_ev(&dummy);
			}
			rv = 1;
		}
	}

	AG_MutexUnlock(&agGlobalKeysLock);
	return (rv);
}
