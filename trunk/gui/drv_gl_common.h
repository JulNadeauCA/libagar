/*	Public domain	*/
/*
 * Code common to all drivers using OpenGL.
 */

#include <agar/gui/begin.h>

struct ag_glyph;

/* Saved blending state */
typedef struct ag_gl_blending_state {
	GLboolean enabled;		/* GL_BLEND enable bit */
	GLint srcFactor;		/* GL_BLEND_SRC mode */
	GLint dstFactor;		/* GL_BLEND_DST mode */
	GLfloat texEnvMode;		/* GL_TEXTURE_ENV mode */
} AG_GL_BlendState;

__BEGIN_DECLS

void AG_GL_InitContext(AG_Rect);
void AG_GL_UploadTexture(Uint *, AG_Surface *, AG_TexCoord *);
int  AG_GL_UpdateTexture(Uint, AG_Surface *, AG_TexCoord *);
void AG_GL_PrepareTexture(void *, int);
void AG_GL_DeleteTexture(void *, Uint);
void AG_GL_DeleteList(void *, Uint);

void AG_GL_BlitSurface(void *, AG_Widget *, AG_Surface *, int, int);
void AG_GL_BlitSurfaceFrom(void *, AG_Widget *, AG_Widget *, int, AG_Rect *, int, int);
void AG_GL_BlitSurfaceGL(void *, AG_Widget *, AG_Surface *, float, float);
void AG_GL_BlitSurfaceFromGL(void *, AG_Widget *, int, float, float);
void AG_GL_BlitSurfaceFlippedGL(void *, AG_Widget *, int, float, float);
void AG_GL_BackupSurfaces(void *, AG_Widget *);
void AG_GL_RestoreSurfaces(void *, AG_Widget *);
int  AG_GL_RenderToSurface(void *, AG_Widget *, AG_Surface **);

void AG_GL_FillRect(void *, AG_Rect, AG_Color);
void AG_GL_PutPixel(void *, int, int, AG_Color);
void AG_GL_PutPixel32(void *, int, int, Uint32);
void AG_GL_PutPixelRGB(void *, int, int, Uint8, Uint8, Uint8);
void AG_GL_BlendPixel(void *, int, int, AG_Color, AG_BlendFn, AG_BlendFn);
void AG_GL_DrawLine(void *, int, int, int, int, AG_Color);
void AG_GL_DrawLineH(void *, int, int, int, AG_Color);
void AG_GL_DrawLineV(void *, int, int, int, AG_Color);
void AG_GL_DrawLineBlended(void *, int, int, int, int, AG_Color, AG_BlendFn, AG_BlendFn);
void AG_GL_DrawArrowUp(void *, int, int, int, AG_Color [2]);
void AG_GL_DrawArrowDown(void *, int, int, int, AG_Color [2]);
void AG_GL_DrawArrowLeft(void *, int, int, int, AG_Color [2]);
void AG_GL_DrawArrowRight(void *, int, int, int, AG_Color [2]);
void AG_GL_DrawRectDithered(void *, AG_Rect, AG_Color);
void AG_GL_DrawBoxRounded(void *, AG_Rect, int, int, AG_Color [3]);
void AG_GL_DrawBoxRoundedTop(void *, AG_Rect, int, int, AG_Color [3]);
void AG_GL_DrawCircle(void *, int, int, int, AG_Color);
void AG_GL_DrawCircle2(void *, int, int, int, AG_Color);
void AG_GL_DrawRectFilled(void *, AG_Rect, AG_Color);
void AG_GL_DrawRectBlended(void *, AG_Rect, AG_Color, AG_BlendFn, AG_BlendFn);
void AG_GL_UpdateGlyph(void *, struct ag_glyph *);
void AG_GL_DrawGlyph(void *, const struct ag_glyph *, int, int);

/* Get corresponding GL blending function */
static __inline__ GLenum
AG_GL_GetBlendingFunc(AG_BlendFn fn)
{
	switch (fn) {
	case AG_ALPHA_ONE:		return (GL_ONE);
	case AG_ALPHA_ZERO:		return (GL_ZERO);
	case AG_ALPHA_SRC:		return (GL_SRC_ALPHA);
	case AG_ALPHA_DST:		return (GL_DST_ALPHA);
	case AG_ALPHA_ONE_MINUS_DST:	return (GL_ONE_MINUS_DST_ALPHA);
	case AG_ALPHA_ONE_MINUS_SRC:	return (GL_ONE_MINUS_SRC_ALPHA);
	case AG_ALPHA_OVERLAY:		return (GL_ONE);	/* XXX */
	default:			return (GL_ONE);
	}
}

__END_DECLS

#include <agar/gui/close.h>
