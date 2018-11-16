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

/* Common OpenGL context data */
typedef struct ag_gl_context {
	AG_ClipRect *_Nullable clipRects; /* Clipping rectangle coords */
	Uint                  nClipRects;
	int clipStates[4];		  /* Clipping states */
	AG_GL_BlendState bs[1];		  /* Alpha blending states */
	Uint *_Nullable textureGC;	  /* Textures queued for deletion */
	Uint           nTextureGC;
	Uint *_Nullable listGC;           /* Display lists queued for deletion */
	Uint           nListGC;
	Uint8 dither[128];		  /* 32x32 stipple pattern */
} AG_GL_Context;

__BEGIN_DECLS
int  AG_GL_InitContext(void *_Nonnull, AG_GL_Context *_Nonnull);
void AG_GL_SetViewport(AG_GL_Context *_Nonnull, AG_Rect);
void AG_GL_DestroyContext(void *_Nonnull);

void AG_GL_StdPushClipRect(void *_Nonnull, AG_Rect);
void AG_GL_StdPopClipRect(void *_Nonnull);
void AG_GL_StdPushBlendingMode(void *_Nonnull, AG_AlphaFn, AG_AlphaFn);
void AG_GL_StdPopBlendingMode(void *_Nonnull);

void AG_GL_StdUploadTexture(void *_Nonnull, Uint *_Nonnull, AG_Surface *_Nonnull,
                            AG_TexCoord *_Nullable);
int  AG_GL_StdUpdateTexture(void *_Nonnull, Uint, AG_Surface *_Nonnull,
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

void AG_GL_FillRect(void *_Nonnull, AG_Rect, AG_Color);
void AG_GL_PutPixel(void *_Nonnull, int,int, AG_Color);
void AG_GL_PutPixel32(void *_Nonnull, int,int, Uint32);
void AG_GL_PutPixel64(void *_Nonnull, int,int, Uint64);
void AG_GL_PutPixelRGB8(void *_Nonnull, int,int, Uint8,Uint8,Uint8);
void AG_GL_PutPixelRGB16(void *_Nonnull, int,int, Uint16,Uint16,Uint16);
void AG_GL_BlendPixel(void *_Nonnull, int,int, AG_Color, AG_AlphaFn, AG_AlphaFn);
void AG_GL_DrawLine(void *_Nonnull, int,int, int,int, AG_Color);
void AG_GL_DrawLineH(void *_Nonnull, int,int, int, AG_Color);
void AG_GL_DrawLineV(void *_Nonnull, int, int,int, AG_Color);
void AG_GL_DrawLineBlended(void *_Nonnull, int,int, int,int, AG_Color,
                           AG_AlphaFn, AG_AlphaFn);
void AG_GL_DrawTriangle(void *_Nonnull, AG_Pt,AG_Pt,AG_Pt, AG_Color);
void AG_GL_DrawArrow(void *_Nonnull, Uint8, int,int, int, AG_Color);
void AG_GL_DrawRectDithered(void *_Nonnull, AG_Rect, AG_Color);
void AG_GL_DrawBoxRounded(void *_Nonnull, AG_Rect, int, int, AG_Color,AG_Color,AG_Color);
void AG_GL_DrawBoxRoundedTop(void *_Nonnull, AG_Rect, int, int, AG_Color,AG_Color,AG_Color);
void AG_GL_DrawCircle(void *_Nonnull, int,int, int, AG_Color);
void AG_GL_DrawCircle2(void *_Nonnull, int,int, int, AG_Color);
void AG_GL_DrawCircleFilled(void *_Nonnull, int,int, int, AG_Color);
void AG_GL_DrawRectFilled(void *_Nonnull, AG_Rect, AG_Color);
void AG_GL_DrawRectBlended(void *_Nonnull, AG_Rect, AG_Color, AG_AlphaFn, AG_AlphaFn);
void AG_GL_UpdateGlyph(void *_Nonnull, struct ag_glyph *_Nonnull);
void AG_GL_DrawGlyph(void *_Nonnull, const struct ag_glyph *_Nonnull, int,int);

/* Upload a texture. */
static __inline__ void
AG_GL_UploadTexture(void *_Nonnull obj, Uint *_Nonnull name,
    AG_Surface *_Nonnull su, AG_TexCoord *_Nullable tc)
{
	AG_Driver *drv = obj;
	AG_DriverClass *dc = AGDRIVER_CLASS(drv);

	dc->uploadTexture(drv, name, su, tc);
}

/* Update the contents of an existing GL texture. */
static __inline__ int
AG_GL_UpdateTexture(void *_Nonnull obj, Uint name, AG_Surface *_Nonnull su,
    AG_TexCoord *_Nullable tc)
{
	AG_Driver *drv = obj;
	AG_DriverClass *dc = AGDRIVER_CLASS(drv);

	return dc->updateTexture(drv, name, su, tc);
}

/*
 * Make sure that the named texture is available for hardware rendering.
 * Upload or update hardware textures if needed.
 */
static __inline__ void
AG_GL_PrepareTexture(void *_Nonnull obj, int name)
{
	AG_Widget *wid = obj;
	AG_Driver *drv = wid->drv;

	if (wid->textures[name] == 0) {
		AG_GL_UploadTexture(drv, &wid->textures[name],
		                          wid->surfaces[name],
		                         &wid->texcoords[name]);
	} else if (wid->surfaceFlags[name] & AG_WIDGET_SURFACE_REGEN) {
		wid->surfaceFlags[name] &= ~(AG_WIDGET_SURFACE_REGEN);
		AG_GL_UpdateTexture(drv, wid->textures[name],
		                         wid->surfaces[name],
					&wid->texcoords[name]);
	}
}

/* Queue a GL texture for deletion. */
static __inline__ void
AG_GL_DeleteTexture(void *_Nonnull obj, Uint name)
{
	AG_Driver *drv = obj;
	AG_DriverClass *dc = AGDRIVER_CLASS(drv);

	dc->deleteTexture(drv, name);
}

/* Queue a GL display list for deletion. */
static __inline__ void
AG_GL_DeleteList(void *_Nonnull obj, Uint name)
{
	AG_Driver *drv = obj;
	AG_DriverClass *dc = AGDRIVER_CLASS(drv);

	if (dc->deleteList != NULL)
		dc->deleteList(drv, name);
}
__END_DECLS

#include <agar/gui/close.h>
