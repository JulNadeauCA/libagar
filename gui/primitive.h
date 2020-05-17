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

typedef enum ag_vector_element_type {
	AG_VE_POINT,		/* Marker (crosshairs) on vertex A */
	AG_VE_LINE,		/* Line from vertex A to vertex B */
	AG_VE_POLYGON,		/* Polygon from vertex array C (indices A..B) */
	AG_VE_CIRCLE,		/* Circle of radius B centered at vertex A */
	AG_VE_ARC1,		/* Arc (0-90 deg) of radius B centered at A */
	AG_VE_ARC2,		/* Arc (90-180 deg) "" */
	AG_VE_ARC3,		/* Arc (180-270 deg) "" */
	AG_VE_ARC4,		/* Arc (270-360 deg) "" */
	AG_VE_LAST
} AG_VectorElementType;

typedef struct ag_vector_element {
	AG_VectorElementType type;		/* Element type (or terminator) */
	int a,b;				/* Immediate values A and B */
	int thick;				/* Line thickness */
	int color;				/* Color index into palette */
	Uint flags;
#define AG_VE_BEVELED	0x0001			/* Beveled endpoints (LINE) */
#define AG_VE_ROUNDED	0x0002			/* Rounded endpoints (LINE) */
#define AG_VE_FILLED	0x0004			/* Filled (CIRCLE) */
#define AG_VE_TAG_MASK	0xff00			/* Mask to decode tag */
	const void *_Nullable p;		/* Array or object parameter */
} AG_VectorElement;

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
                          const AG_Color *_Nonnull, AG_AlphaFn, AG_AlphaFn);
void ag_draw_line_w(void *_Nonnull, int,int, int,int, const AG_Color *_Nonnull,
                    float);
void ag_draw_line_w_sti16(void *_Nonnull, int,int, int,int,
                          const AG_Color *_Nonnull, float, Uint16);

void ag_draw_triangle(void *_Nonnull, const AG_Pt *, const AG_Pt *, const AG_Pt *,
                      const AG_Color *_Nonnull);
void ag_draw_polygon(void *_Nonnull, const AG_Pt *_Nonnull, Uint, const AG_Color *);
void ag_draw_polygon_sti32(void *_Nonnull, const AG_Pt *_Nonnull, Uint,
                           const AG_Color *_Nonnull, const Uint8 *_Nonnull);
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
void ag_draw_rect_filled(void *_Nonnull, const AG_Rect *_Nonnull, const AG_Color *_Nonnull);
void ag_draw_rect_blended(void *_Nonnull, const AG_Rect *_Nonnull,
                          const AG_Color *_Nonnull,
			  AG_AlphaFn, AG_AlphaFn);
void ag_draw_rect_dithered(void *_Nonnull, const AG_Rect *_Nonnull, const AG_Color *_Nonnull);
void ag_draw_frame(void *_Nonnull, const AG_Rect *_Nonnull, int, const AG_Color *_Nonnull);
void ag_draw_frame_raised(void *_Nonnull, const AG_Rect *_Nonnull);
void ag_draw_frame_sunk(void *_Nonnull, const AG_Rect *_Nonnull);
void ag_draw_box(void *_Nonnull, const AG_Rect *_Nonnull, int, const AG_Color *_Nonnull);
void ag_draw_box_raised(void *_Nonnull, const AG_Rect *_Nonnull, const AG_Color *_Nonnull);
void ag_draw_box_sunk(void *_Nonnull, const AG_Rect *_Nonnull, const AG_Color *_Nonnull);
void ag_draw_box_disabled(void *_Nonnull, const AG_Rect *_Nonnull, int,
                          const AG_Color *_Nonnull, const AG_Color *_Nonnull);
void ag_draw_rect_outline(void *_Nonnull, const AG_Rect *_Nonnull,
                          const AG_Color *_Nonnull);
void ag_draw_arrow_line(void *obj, int x1, int y1, int x2, int y2,
    AG_ArrowLineType t, int length, double theta, const AG_Color *C);

# define AG_PutPixel(o,x,y,c)			  ag_put_pixel((o),(x),(y),(c))
# define AG_PutPixel32(o,x,y,c)			  ag_put_pixel_32((o),(x),(y),(c))
# define AG_PutPixel64(o,x,y,px)		  ag_put_pixel_64((o),(x),(y),(px))
# define AG_PutPixelRGB_8(o,x,y,r,g,b)		  ag_put_pixel_rgb_8((o),(x),(y),(r),(g),(b))
# define AG_PutPixelRGB_16(o,x,y,r,g,b)		  ag_put_pixel_rgb_16((o),(x),(y),(r),(g),(b))
# define AG_BlendPixel(o,x,y,c,fn)		  ag_blend_pixel((o),(x),(y),(c),(fn))
# define AG_BlendPixel32(o,x,y,px,fn)		  ag_blend_pixel_32((o),(x),(y),(px),(fn))
# define AG_BlendPixel64(o,x,y,px,fn)		  ag_blend_pixel_64((o),(x),(y),(px),(fn))
# define AG_BlendPixelRGBA(o,x,y,c,fn)		  ag_blend_pixel_rgba((o),(x),(y),(c),(fn))
# define AG_DrawLine(o,x1,y1,x2,y2,c)		  ag_draw_line((o),(x1),(y1),(x2),(y2),(c))
# define AG_DrawLineH(o,x1,x2,y,c)		  ag_draw_line_h((o),(x1),(x2),(y),(c))
# define AG_DrawLineV(o,x,y1,y2,c)		  ag_draw_line_v((o),(x),(y1),(y2),(c))
# define AG_DrawLineBlended(o,x,y,x2,y2,c,fs,fd)  ag_draw_line_blended((o),(x),(y),(x2),(y2),(c),(fs),(fd))
# define AG_DrawLineW(o,x1,y1,x2,y2,c,w)	  ag_draw_line_w((o),(x1),(y1),(x2),(y2),(c),(w))
# define AG_DrawLineW_Sti16(o,x1,y1,x2,y2,c,w,m)  ag_draw_line_w_sti16((o),(x1),(y1),(x2),(y2),(c),(w),(m))
# define AG_DrawTriangle(o,v1,v2,v3,c)		  ag_draw_triangle((o),(v1),(v2),(v3),(c))
# define AG_DrawPolygon(o,pts,n,c)		  ag_draw_polygon((o),(pts),(n),(c))
# define AG_DrawPolygon_Sti32(o,pts,n,c,sti)	  ag_draw_polygon_sti32((o),(pts),(n),(c),(sti))
# define AG_DrawArrowUp(o,x,y,h,c)		  ag_draw_arrow_up((o),(x),(y),(h),(c))
# define AG_DrawArrowRight(o,x,y,h,c)		  ag_draw_arrow_right((o),(x),(y),(h),(c))
# define AG_DrawArrowDown(o,x,y,h,c)		  ag_draw_arrow_down((o),(x),(y),(h),(c))
# define AG_DrawArrowLeft(o,x,y,h,c)		  ag_draw_arrow_left((o),(x),(y),(h),(c))
# define AG_DrawBoxRounded(o,r,z,rad,c)		  ag_draw_box_rounded((o),(r),(z),(rad),(c))
# define AG_DrawBoxRoundedTop(o,r,z,rad,c)	  ag_draw_box_rounded_top((o),(r),(z),(rad),(c))
# define AG_DrawCircle(o,x,y,rad,c)		  ag_draw_circle((o),(x),(y),(rad),(c))
# define AG_DrawCircleFilled(o,x,y,rad,c)	  ag_draw_circle_filled((o),(x),(y),(rad),(c))
# define AG_DrawRect(o,r,c)			  ag_draw_rect((o),(r),(c))
# define AG_DrawRectFilled(o,r,c)		  ag_draw_rect_filled((o),(r),(c))
# define AG_DrawRectBlended(o,r,c,sf,df)	  ag_draw_rect_blended((o),(r),(c),(sf),(df))
# define AG_DrawRectDithered(o,r,c)		  ag_draw_rect_dithered((o),(r),(c))
# define AG_DrawFrame(o,r,z,c)			  ag_draw_frame((o),(r),(z),(c))
# define AG_DrawFrameRaised(o,r)		  ag_draw_frame_raised((o),(r))
# define AG_DrawFrameSunk(o,r)			  ag_draw_frame_sunk((o),(r))
# define AG_DrawBox(o,r,z,c)			  ag_draw_box((o),(r),(z),(c))
# define AG_DrawBoxRaised(o,r,c)		  ag_draw_box_raised((o),(r),(c))
# define AG_DrawBoxSunk(o,r,c)		  	  ag_draw_box_sunk((o),(r),(c))
# define AG_DrawBoxDisabled(o,r,z,c1,c2)	  ag_draw_box_disabled((o),(r),(z),(c1),(c2))
# define AG_DrawRectOutline(o,r,c)		  ag_draw_rect_outline((o),(r),(c))
# define AG_DrawArrowLine(o,x1,y1,x2,y2,t,l,th,c) ag_draw_arrow_line((o),(x1),(y1),(x2),(y2),(t),(l),(th),(c))
#endif /* !AG_INLINE_WIDGET */

void ag_draw_rect_noop(void *_Nonnull, const AG_Rect *_Nonnull, const AG_Color *_Nonnull);

int AG_GetLineIntersection(int,int, int,int, int,int, int,int,
                           int *_Nonnull, int *_Nonnull);

void AG_ClipLine(int,int, int,int, int,int, int *_Nonnull,int *_Nonnull);

void AG_ClipLineCircle(int,int, int, int,int, int,int,
                       int *_Nonnull,int *_Nonnull);

void AG_DrawArrowhead(void *_Nonnull, int, int, int,int, int, double,
                      const AG_Color *_Nonnull);

void AG_DrawVector(void *_Nonnull, int,int, const AG_Rect *_Nonnull,
                   const AG_Color *_Nonnull, const AG_VectorElement *_Nonnull,
                   int,int);

void AG_DrawFrame_Blended(AG_Widget *_Nonnull, const AG_Rect *_Nonnull,
                          const AG_Color *_Nonnull, const AG_Color *_Nonnull,
                          int, int);
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_GUI_PRIMITIVE_H_ */
