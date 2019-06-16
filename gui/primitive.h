/*	Public domain	*/

#ifndef _AGAR_GUI_PRIMITIVE_H_
#define _AGAR_GUI_PRIMITIVE_H_

#include <agar/gui/widget.h>
#include <agar/gui/begin.h>

typedef enum ag_arrowline_type {
	AG_ARROWLINE_NONE,
	AG_ARROWLINE_FORWARD,
	AG_ARROWLINE_REVERSE,
	AG_ARROWLINE_BOTH
} AG_ArrowLineType;

__BEGIN_DECLS
#ifdef AG_INLINE_WIDGET
# define AG_INLINE_HEADER
# include <agar/gui/inline_primitive.h>
#else /* !AG_INLINE_WIDGET */
void ag_put_pixel(void *_Nonnull, int,int, const AG_Color *_Nonnull);
void ag_put_pixel_32(void *_Nonnull, int,int, Uint32);
void ag_put_pixel_rgb_8(void *_Nonnull, int,int, Uint8,Uint8,Uint8);
void ag_put_pixel_rgb(void *_Nonnull, int,int, AG_Component, AG_Component,
                      AG_Component);
void ag_blend_pixel(void *_Nonnull, int,int, const AG_Color *_Nonnull,  AG_AlphaFn);
void ag_blend_pixel_32(void *_Nonnull, int,int, Uint32, AG_AlphaFn);
void ag_blend_pixel_rgba(void *_Nonnull, int,int, Uint8 [_Nonnull 4], AG_AlphaFn);
# if AG_MODEL == AG_LARGE
void ag_put_pixel_64(void *_Nonnull, int,int, Uint64);
void ag_put_pixel_rgb_16(void *_Nonnull, int,int, Uint16,Uint16,Uint16);
void ag_blend_pixel_64(void *_Nonnull, int,int, Uint64, AG_AlphaFn);
# endif
void ag_draw_line(void *_Nonnull, int,int, int,int, const AG_Color *_Nonnull);
void ag_draw_line_h(void *_Nonnull, int,int, int, const AG_Color *_Nonnull);
void ag_draw_line_v(void *_Nonnull, int, int,int, const AG_Color *_Nonnull);
void ag_draw_line_blended(void *_Nonnull, int,int, int,int,
                          const AG_Color *_Nonnull, AG_AlphaFn);
void ag_draw_triangle(void *_Nonnull, const AG_Pt *, const AG_Pt *, const AG_Pt *,
                      const AG_Color *_Nonnull);
void ag_draw_arrow_up(void *_Nonnull, int,int, int, const AG_Color *_Nonnull);
void ag_draw_arrow_right(void *_Nonnull, int,int, int, const AG_Color *_Nonnull);
void ag_draw_arrow_down(void *_Nonnull, int,int, int, const AG_Color *_Nonnull);
void ag_draw_arrow_left(void *_Nonnull, int,int, int, const AG_Color *_Nonnull);
void ag_draw_box_rounded(void *_Nonnull, const AG_Rect *_Nonnull, int, int,
                         const AG_Color *_Nonnull);
void ag_draw_box_rounded_top(void *_Nonnull, const AG_Rect *_Nonnull, int, int,
                             const AG_Color *_Nonnull);
void ag_draw_circle(void *_Nonnull, int,int, int, const AG_Color *_Nonnull);
void ag_draw_circle_filled(void *_Nonnull, int,int, int, const AG_Color *_Nonnull);
void ag_draw_rect(void *_Nonnull, const AG_Rect *_Nonnull, const AG_Color *_Nonnull);
void ag_draw_rect_filled(void *_Nonnull, const AG_Rect *_Nonnull,
                         const AG_Color *_Nonnull);
void ag_draw_rect_blended(void *_Nonnull, const AG_Rect *_Nonnull,
                          const AG_Color *_Nonnull, AG_AlphaFn);
void ag_draw_rect_dithered(void *_Nonnull, const AG_Rect *_Nonnull,
                           const AG_Color *_Nonnull);
void ag_draw_frame(void *_Nonnull, const AG_Rect *_Nonnull, int,
                   const AG_Color *_Nonnull);
void ag_draw_box(void *_Nonnull, const AG_Rect *_Nonnull, int,
                 const AG_Color *_Nonnull);
void ag_draw_box_disabled(void *_Nonnull, const AG_Rect *_Nonnull, int,
                          const AG_Color *_Nonnull, const AG_Color *_Nonnull);
void ag_draw_frame_blended(void *_Nonnull, const AG_Rect *_Nonnull,
                           const AG_Color *_Nonnull, AG_AlphaFn);
void ag_draw_rect_outline(void *_Nonnull, const AG_Rect *_Nonnull,
                          const AG_Color *_Nonnull);
void ag_draw_plus(void *_Nonnull, const AG_Rect *_Nonnull,
                  const AG_Color *_Nonnull, AG_AlphaFn);
void ag_draw_minus(void *_Nonnull, const AG_Rect *_Nonnull,
                   const AG_Color *_Nonnull, AG_AlphaFn);
void ag_draw_line_2(void *_Nonnull, int,int, int,int, const AG_Color *_Nonnull);
void ag_draw_arrow_line(void *obj, int x1, int y1, int x2, int y2,
    AG_ArrowLineType t, int length, double theta, const AG_Color *C);

# define AG_PutPixel(o,x,y,c)			ag_put_pixel((o),(x),(y),(c))
# define AG_PutPixel32(o,x,y,c)			ag_put_pixel_32((o),(x),(y),(c))
# define AG_PutPixel64(o,x,y,px)		ag_put_pixel_64((o),(x),(y),(px))
# define AG_PutPixelRGB_8(o,x,y,r,g,b)		ag_put_pixel_rgb_8((o),(x),(y),(r),(g),(b))
# define AG_PutPixelRGB_16(o,x,y,r,g,b)		ag_put_pixel_rgb_16((o),(x),(y),(r),(g),(b))
# define AG_BlendPixel(o,x,y,c,fn)		ag_blend_pixel((o),(x),(y),(c),(fn))
# define AG_BlendPixel32(o,x,y,px,fn)		ag_blend_pixel_32((o),(x),(y),(px),(fn))
# define AG_BlendPixel64(o,x,y,px,fn)		ag_blend_pixel_64((o),(x),(y),(px),(fn))
# define AG_BlendPixelRGBA(o,x,y,c,fn)		ag_blend_pixel_rgba((o),(x),(y),(c),(fn))
# define AG_DrawLine(o,x1,y1,x2,y2,c)		ag_draw_line((o),(x1),(y1),(x2),(y2),(c))
# define AG_DrawLineH(o,x1,x2,y,c)		ag_draw_line_h((o),(x1),(x2),(y),(c))
# define AG_DrawLineV(o,x,y1,y2,c)		ag_draw_line_v((o),(x),(y1),(y2),(c))
# define AG_DrawLineBlended(o,x1,y1,x2,y2,c,fn)	ag_draw_line_blended((o),(x1),(y1),(x2),(y2),(c),(fn))
# define AG_DrawTriangle(o,v1,v2,v3,c)		ag_draw_triangle((o),(v1),(v2),(v3),(c))
# define AG_DrawArrowUp(o,x,y,h,c)		ag_draw_arrow_up((o),(x),(y),(h),(c))
# define AG_DrawArrowRight(o,x,y,h,c)		ag_draw_arrow_right((o),(x),(y),(h),(c))
# define AG_DrawArrowDown(o,x,y,h,c)		ag_draw_arrow_down((o),(x),(y),(h),(c))
# define AG_DrawArrowLeft(o,x,y,h,c)		ag_draw_arrow_left((o),(x),(y),(h),(c))
# define AG_DrawBoxRounded(o,r,z,rad,c)		ag_draw_box_rounded((o),(r),(z),(rad),(c))
# define AG_DrawBoxRoundedTop(o,r,z,rad,c)	ag_draw_box_rounded_top((o),(r),(z),(rad),(c))
# define AG_DrawCircle(o,x,y,rad,c)		ag_draw_circle((o),(x),(y),(rad),(c))
# define AG_DrawCircleFilled(o,x,y,rad,c)	ag_draw_circle_filled((o),(x),(y),(rad),(c))
# define AG_DrawRect(o,r,c)			ag_draw_rect((o),(r),(c))
# define AG_DrawRectFilled(o,r,c)		ag_draw_rect_filled((o),(r),(c))
# define AG_DrawRectBlended(o,r,c,fn)		ag_draw_rect_blended((o),(r),(c),(fn))
# define AG_DrawRectDithered(o,r,c)		ag_draw_rect_dithered((o),(r),(c))
# define AG_DrawFrame(o,r,z,c)			ag_draw_frame((o),(r),(z),(c))
# define AG_DrawBox(o,r,z,c)			ag_draw_box((o),(r),(z),(c))
# define AG_DrawBoxDisabled(o,r,z,c1,c2)	ag_draw_box_disabled((o),(r),(z),(c1),(c2))
# define AG_DrawFrameBlended(o,r,c,fn)		ag_draw_frame_blended((o),(r),(c),(fn))
# define AG_DrawRectOutline(o,r,c)		ag_draw_rect_outline((o),(r),(c))
# define AG_DrawPlus(o,r,c,fn)			ag_draw_plus((o),(r),(c),(fn))
# define AG_DrawMinus(o,r,c,fn)			ag_draw_minus((o),(r),(c),(fn))
# define AG_DrawLine2(o,x1,y1,x2,y2,c)		ag_draw_line_2((o),(x1),(y1),(x2),(y2),(c))
#ifdef AG_HAVE_FLOAT
#define AG_DrawArrowLine(o,x1,y1,x2,y2,t,l,th,c) ag_draw_arrow_line((o),(x1),(y1),(x2),(y2),(t),(l),(th),(c))
#endif /* AG_HAVE_FLOAT */
#endif /* !AG_INLINE_WIDGET */

void AG_DrawTiling(void *_Nonnull, const AG_Rect *_Nonnull, int, int,
                   const AG_Color *_Nonnull, const AG_Color *_Nonnull);
int AG_GetLineIntersection(long x1, long y1, long x2, long y2, long x3,
                           long y3, long x4, long y4, long *xi, long *yi);
#ifdef AG_HAVE_FLOAT
void AG_ClipLine(int ax, int ay, int aw, int ah, int x1, int y1, int *x2, int *y2);
void AG_ClipLineCircle(int xc, int yc, int r, int x1, int y1,
                       int x2, int y2, int *xi, int *yi);
void AG_DrawArrowhead(void *_Nonnull obj, int x1, int y1,
		int x2, int y2, int length, double theta,
		const AG_Color *_Nonnull c);
#endif /* AG_HAVE_FLOAT */
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_GUI_PRIMITIVE_H_ */
