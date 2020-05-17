/*	Public domain	*/
/*
 * Code common to all drivers using OpenGL.
 */

#include <agar/gui/begin.h>

struct ag_glyph;

/* Saved blending state */
typedef struct ag_gl_blend_state {
	GLboolean enabled;		/* GL_BLEND enable bit */
	char _pad[3];
	GLint srcFactor;		/* GL_BLEND_SRC mode */
	GLint dstFactor;		/* GL_BLEND_DST mode */
} AG_GL_BlendState;

/* Common OpenGL context data */
typedef struct ag_gl_context {
	AG_ClipRect *_Nullable clipRects;	/* Clipping rectangle coords */
	Uint                  nClipRects;
	Uint                maxClipRects;

	AG_GL_BlendState *_Nullable blendStates;  /* Alpha blending states */
	Uint                       nBlendStates;
	Uint                     maxBlendStates;

	Uint *_Nullable textureGC;	  /* Textures queued for deletion */
	Uint           nTextureGC;
	Uint         maxTextureGC;

	Uint *_Nullable listGC;           /* Display lists queued for deletion */
	Uint           nListGC;
	Uint         maxListGC;

	Uint32 dither[32];		  /* 32x32 stipple pattern */
} AG_GL_Context;

__BEGIN_DECLS
void AG_GL_InitContext(void *_Nonnull, AG_GL_Context *_Nonnull);
void AG_GL_SetViewport(AG_GL_Context *_Nonnull, const AG_Rect *_Nonnull);
void AG_GL_DestroyContext(void *_Nonnull);

void AG_GL_StdPushClipRect(void *_Nonnull, const AG_Rect *_Nonnull);
void AG_GL_StdPopClipRect(void *_Nonnull);
void AG_GL_StdPushBlendingMode(void *_Nonnull, AG_AlphaFn, AG_AlphaFn);
void AG_GL_StdPopBlendingMode(void *_Nonnull);

void AG_GL_StdUploadTexture(void *_Nonnull, Uint *_Nonnull, AG_Surface *_Nonnull,
                            AG_TexCoord *_Nullable);
void AG_GL_StdUpdateTexture(void *_Nonnull, Uint, AG_Surface *_Nonnull,
                            AG_TexCoord *_Nullable);

void AG_GL_StdDeleteTexture(void *_Nonnull, Uint);
void AG_GL_StdDeleteList(void *_Nonnull, Uint);

void AG_GL_BlitSurface(void *_Nonnull, AG_Widget *_Nonnull,
                       AG_Surface *_Nonnull, int,int);

void AG_GL_BlitSurfaceFrom(void *_Nonnull, AG_Widget *_Nonnull, int,
                           const AG_Rect *_Nullable, int,int);
void AG_GL_BlitSurfaceGL(void *_Nonnull, AG_Widget *_Nonnull,
                         AG_Surface *_Nonnull, float,float);
void AG_GL_BlitSurfaceFromGL(void *_Nonnull, AG_Widget *_Nonnull, int,
                             float,float);
void AG_GL_BlitSurfaceFlippedGL(void *_Nonnull, AG_Widget *_Nonnull, int,
                                float,float);
void AG_GL_BackupSurfaces(void *_Nonnull, AG_Widget *_Nonnull);
void AG_GL_RestoreSurfaces(void *_Nonnull, AG_Widget *_Nonnull);
int  AG_GL_RenderToSurface(void *_Nonnull, AG_Widget *_Nonnull,
                           AG_Surface *_Nonnull *_Nullable);

void AG_GL_FillRect(void *_Nonnull, const AG_Rect *_Nonnull,
                    const AG_Color *_Nonnull);
void AG_GL_PutPixel(void *_Nonnull, int,int, const AG_Color *_Nonnull);
void AG_GL_PutPixel32(void *_Nonnull, int,int, Uint32);
void AG_GL_PutPixel64(void *_Nonnull, int,int, Uint64);
void AG_GL_PutPixelRGB8(void *_Nonnull, int,int, Uint8,Uint8,Uint8);
void AG_GL_PutPixelRGB16(void *_Nonnull, int,int, Uint16,Uint16,Uint16);
void AG_GL_BlendPixel(void *_Nonnull, int,int, const AG_Color *_Nonnull,
                      AG_AlphaFn, AG_AlphaFn);
void AG_GL_DrawLine(void *_Nonnull, int,int, int,int, const AG_Color *_Nonnull);
void AG_GL_DrawLineH(void *_Nonnull, int,int, int, const AG_Color *_Nonnull);
void AG_GL_DrawLineV(void *_Nonnull, int, int,int, const AG_Color *_Nonnull);
void AG_GL_DrawLineBlended(void *_Nonnull, int,int, int,int,
                           const AG_Color *_Nonnull, AG_AlphaFn, AG_AlphaFn);
void AG_GL_DrawLineW(void *_Nonnull, int,int, int,int, const AG_Color *, float);
void AG_GL_DrawLineW_Sti16(void *_Nonnull, int,int, int,int, const AG_Color *,
                     float, Uint16);
void AG_GL_DrawTriangle(void *_Nonnull, const AG_Pt *_Nonnull,
                        const AG_Pt *_Nonnull, const AG_Pt *_Nonnull,
                        const AG_Color *_Nonnull);
void AG_GL_DrawPolygon(void *_Nonnull, const AG_Pt *_Nonnull, Uint,
                       const AG_Color *_Nonnull);
void AG_GL_DrawPolygon_Sti32(void *_Nonnull, const AG_Pt *_Nonnull, Uint,
                             const AG_Color *_Nonnull, const Uint8 *_Nonnull);
void AG_GL_DrawArrow(void *_Nonnull, Uint8, int,int, int,
                     const AG_Color *_Nonnull);
void AG_GL_DrawRectDithered(void *_Nonnull, const AG_Rect *_Nonnull,
                            const AG_Color *_Nonnull);
void AG_GL_DrawBoxRounded(void *_Nonnull, const AG_Rect *_Nonnull, int, int,
                          const AG_Color *_Nonnull,
			  const AG_Color *_Nonnull,
			  const AG_Color *_Nonnull);
void AG_GL_DrawBoxRoundedTop(void *_Nonnull, const AG_Rect *_Nonnull, int, int,
                             const AG_Color *_Nonnull,
			     const AG_Color *_Nonnull,
			     const AG_Color *_Nonnull);
void AG_GL_DrawCircle(void *_Nonnull, int,int, int, const AG_Color *_Nonnull);
void AG_GL_DrawCircle2(void *_Nonnull, int,int, int, const AG_Color *_Nonnull);
void AG_GL_DrawCircleFilled(void *_Nonnull, int,int, int, const AG_Color *_Nonnull);
void AG_GL_DrawRectFilled(void *_Nonnull, const AG_Rect *_Nonnull,
                          const AG_Color *_Nonnull);
void AG_GL_DrawRectBlended(void *_Nonnull, const AG_Rect *_Nonnull,
                           const AG_Color *_Nonnull, AG_AlphaFn, AG_AlphaFn);
void AG_GL_UpdateGlyph(void *_Nonnull, struct ag_glyph *_Nonnull);
void AG_GL_DrawGlyph(void *_Nonnull, const struct ag_glyph *_Nonnull, int,int);

void AG_GL_UploadTexture(void *_Nonnull, Uint *_Nonnull, AG_Surface *_Nonnull,
                         AG_TexCoord *_Nullable);
void AG_GL_UpdateTexture(void *_Nonnull, Uint, AG_Surface *_Nonnull,
                         AG_TexCoord *_Nullable);
void AG_GL_DeleteTexture(void *_Nonnull, Uint);
void AG_GL_DeleteList(void *_Nonnull, Uint);
__END_DECLS

#include <agar/gui/close.h>
