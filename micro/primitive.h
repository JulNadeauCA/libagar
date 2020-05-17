/*	Public domain	*/

#ifndef _AGAR_MICRO_PRIMITIVE_H_
#define _AGAR_MICRO_PRIMITIVE_H_

#include <agar/micro/widget.h>
#include <agar/micro/begin.h>

__BEGIN_DECLS
void AG_PutPixel(void *_Nonnull, Uint8,Uint8, const MA_Color *_Nonnull);
void AG_PutPixel_8(void *_Nonnull, Uint8,Uint8, Uint8);
void AG_BlendPixel(void *_Nonnull, Uint8,Uint8, const MA_Color *_Nonnull);
void AG_DrawLine(void *_Nonnull, Uint8,Uint8, Uint8, Uint8, const MA_Color *_Nonnull);
void AG_DrawLineH(void *_Nonnull, Uint8,Uint8, Uint8, const MA_Color *_Nonnull);
void AG_DrawLineV(void *_Nonnull, Uint8, Uint8,Uint8, const MA_Color *_Nonnull);
void AG_DrawPolygon(void *_Nonnull, const Uint8 *_Nonnull, Uint8, const MA_Color *_Nonnull);
void AG_DrawCircle(void *_Nonnull, Uint8,Uint8, Uint8, const MA_Color *_Nonnull);
void AG_DrawCircleFilled(void *_Nonnull, Uint8,Uint8, Uint8, const MA_Color *_Nonnull);
void AG_DrawRect(void *_Nonnull, const MA_Rect *_Nonnull, const MA_Color *_Nonnull);
void AG_DrawRectFilled(void *_Nonnull, const MA_Rect *_Nonnull, const MA_Color *_Nonnull);
void AG_DrawBox(void *_Nonnull, const MA_Rect *_Nonnull, Sint8, const MA_Color *_Nonnull);
void AG_DrawFrame(void *_Nonnull, const MA_Rect *_Nonnull, Sint8, const MA_Color *_Nonnull);
void AG_DrawBoxRounded(void *_Nonnull, const MA_Rect *_Nonnull, Sint8, Uint8, const MA_Color *_Nonnull);
void AG_DrawBoxRoundedTop(void *_Nonnull, const MA_Rect *_Nonnull, Sint8, Uint8, const MA_Color *_Nonnull);
__END_DECLS

#include <agar/micro/close.h>
#endif	/* _AGAR_MICRO_PRIMITIVE_H_ */
