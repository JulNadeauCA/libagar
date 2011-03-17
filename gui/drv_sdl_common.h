/*	Public domain	*/
/*
 * Code common to all drivers using the SDL library.
 */

#include <agar/config/have_sdl.h>
#ifdef HAVE_SDL

/* XXX */
#undef HAVE_SNPRINTF
#undef HAVE_VSNPRINTF
#undef HAVE_SYS_TYPES_H
#undef HAVE_STDIO_H
#undef HAVE_STDLIB_H
#undef HAVE_STDARG_H
#undef Uint8
#undef Sint8
#undef Uint16
#undef Sint16
#undef Uint32
#undef Sint32
#undef Uint64
#undef Sint64
#include <SDL.h>

#include <agar/gui/begin.h>

__BEGIN_DECLS
AG_PixelFormat *AG_SDL_GetPixelFormat(SDL_Surface *);
void            AG_SDL_BlitSurface(const AG_Surface *, const AG_Rect *,
                                   SDL_Surface *, int, int);
AG_Surface     *AG_SDL_ImportSurface(SDL_Surface *);

int             AG_SDL_SetRefreshRate(void *, int);

int             AG_SDL_InitDefaultCursor(void *);
int             AG_SDL_SetCursor(void *, AG_Cursor *);
void            AG_SDL_UnsetCursor(void *);
int             AG_SDL_CreateCursor(void *, AG_Cursor *);
void            AG_SDL_FreeCursor(void *, AG_Cursor *);
int             AG_SDL_GetCursorVisibility(void *);
void            AG_SDL_SetCursorVisibility(void *, int);

int             AG_SDL_PostEventCallback(void *);

int             AG_SDL_GetDisplaySize(Uint *, Uint *);
void            AG_SDL_BeginEventProcessing(void *);
int             AG_SDL_PendingEvents(void *);
void            AG_SDL_TranslateEvent(void *, const SDL_Event *, AG_DriverEvent *);
int             AG_SDL_GetNextEvent(void *, AG_DriverEvent *);
int             AG_SDL_ProcessEvent(void *, AG_DriverEvent *);
void            AG_SDL_GenericEventLoop(void *);
void            AG_SDL_EndEventProcessing(void *);
void            AG_SDL_Terminate(void);
__END_DECLS

#include <agar/gui/close.h>
#endif /* HAVE_SDL */
