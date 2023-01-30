/*	Public domain	*/
/*
 * Code common to all drivers using the SDL2 library.
 */

#include <agar/config/have_sdl2.h>
#ifdef HAVE_SDL2

/* XXX */
#undef HAVE_CLOCK_GETTIME
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

#ifdef _USE_SDL_FRAMEWORK
# include <SDL2/SDL.h>
# ifdef main
#  undef main
# endif
#else
# include <SDL2/SDL.h>
#endif

#include <agar/gui/begin.h>

__BEGIN_DECLS
struct ag_window *_Nullable AG_SDL_GetWindowFromID(AG_Driver *, Uint32);

AG_PixelFormat *_Nonnull AG_SDL2_GetPixelFormat(const SDL_Surface *_Nonnull);
void                     AG_SDL2_BlitSurface(const AG_Surface *_Nonnull,
                                             const AG_Rect *_Nullable,
                                             SDL_Surface *_Nonnull, int,int);
AG_Surface *_Nullable   AG_SDL2_ImportSurface(SDL_Surface *_Nonnull);
int                     AG_SDL2_SetRefreshRate(void *_Nonnull, int);

void                    AG_SDL2_InitDefaultCursor(void *_Nonnull);
int                     AG_SDL2_SetCursor(void *_Nonnull, AG_Cursor *_Nonnull);
void                    AG_SDL2_UnsetCursor(void *_Nonnull);
AG_Cursor *_Nullable    AG_SDL2_CreateCursor(void *_Nonnull, Uint,Uint,
                                             const Uint8 *_Nonnull,
                                             const Uint8 *_Nonnull, int,int);
void                    AG_SDL2_FreeCursor(void *_Nonnull, AG_Cursor *_Nonnull);
int                     AG_SDL2_GetCursorVisibility(void *_Nonnull);
void                    AG_SDL2_SetCursorVisibility(void *_Nonnull, int);

int  AG_SDL2_GetDisplaySize(Uint *_Nonnull, Uint *_Nonnull);
void AG_SDL2_GetPrefDisplaySettings(void *_Nonnull, Uint *_Nonnull,
                                    Uint *_Nonnull, int *_Nonnull);
void AG_SDL2_BeginEventProcessing(void *_Nullable);
int  AG_SDL2_PendingEvents(void *_Nonnull);
void AG_SDL2_TranslateEvent(void *_Nonnull, const SDL_Event *_Nonnull,
                            AG_DriverEvent *_Nonnull);
int  AG_SDL2_GetNextEvent(void *_Nonnull, AG_DriverEvent *_Nonnull);
int  AG_SDL2_ProcessEvent_SW(void *_Nullable, AG_DriverEvent *_Nonnull);
int  AG_SDL2_ProcessEvent_MW(void *_Nullable, AG_DriverEvent *_Nonnull);
int  AG_SDL2_EventSink_SW(AG_EventSink *_Nonnull, AG_Event *_Nonnull);
int  AG_SDL2_EventEpilogue(AG_EventSink *_Nonnull, AG_Event *_Nonnull);
void AG_SDL2_EndEventProcessing(void *_Nonnull);

void AG_SDL2_ControllerAdded(void *, int);
void AG_SDL2_JoystickAdded(void *, int);
void AG_SDL2_JoystickRemoved(void *, int);

Uint AG_SDL_KeySymToUcs4(Uint32);
__END_DECLS

#include <agar/gui/close.h>
#endif /* HAVE_SDL2 */
