/*	Public domain	*/

struct sg_view;

#ifndef SG_TEXTURE_PROGS_MAX
#define SG_TEXTURE_PROGS_MAX	64
#endif
#ifndef SG_TEXTURE_SURFACES_MAX
#define SG_TEXTURE_SURFACES_MAX	1024
#endif

typedef struct sg_texture_surface {
	Uint flags;
#define SG_TEXTURE_SURFACE_NODUP	0x01	/* Surface is not a copy */
#define SG_TEXTURE_SURFACE_SUPPRESS	0x02	/* Disabled */
#define SG_TEXTURE_SURFACE_SAVED	(SG_TEXTURE_SURFACE_SUPPRESS)
	Uint32 _pad;
	AG_Surface *_Nonnull su;		/* Source surface */
	AG_Rect rSrc;				/* Source rectangle */
	AG_Rect rDst;				/* Destination rectangle */
	AG_TAILQ_ENTRY(sg_texture_surface) surfaces;
} SG_TextureSurface;

typedef struct sg_texture_program {
	Uint flags;
#define SG_TEXTURE_PROGRAM_SUPPRESS	0x01	/* Disabled */
#define SG_TEXTURE_PROGRAM_SAVED	(SG_TEXTURE_PROGRAM_SUPPRESS)
	char progName[AG_OBJECT_NAME_MAX];	/* Program name */
	SG_Program *_Nullable prog;		/* Resolved program */
	AG_TAILQ_ENTRY(sg_texture_program) programs;
} SG_TextureProgram;

typedef struct sg_texture {
	struct ag_object _inherit;	/* AG_Object -> SG_Texture */

	Uint flags;
#define SG_TEXTURE_NOLIGHT 0x01	/* Disable lighting for this material */
#define SG_TEXTURE_SAVED   (SG_TEXTURE_NOLIGHT)

	Uint w, h;			/* Pixel dimensions */
	Uint32 _pad1;
	M_Real shininess;		/* Specular exponent */
					/* (for fixed lighting model) */
#ifdef AG_DEBUG
	Uint32 _pad2;
	Uint32 _pad3;
#endif
	M_Color emissive;		/* Light emitted by surface (Ke) */
	M_Color ambient;		/* Ambient reflectance (Ka) */
	M_Color diffuse;		/* Diffuse reflectance (Kd) */
	M_Color specular;		/* Specular reflectance (Ks) */
	
	AG_TAILQ_HEAD_(sg_texture_program) progs;	/* Fragment shaders */
	Uint                              nProgs;

	Uint                 nSurfaces;
	AG_Surface *_Nullable surface;			/* Rendered surface */

	AG_TAILQ_HEAD_(sg_texture_surface) surfaces;	/* Input surface(s) */
	AG_TAILQ_HEAD_(sg_view_texture) vtex;	/* Active hardware textures */
} SG_Texture;

#define SGTEXTURE(obj)            ((SG_Texture *)(obj))
#define SGCTEXTURE(obj)           ((const SG_Texture *)(obj))
#define SG_TEXTURE_SELF()          SGTEXTURE( AG_OBJECT(0,"SG_Texture:*") )
#define SG_TEXTURE_PTR(n)          SGTEXTURE( AG_OBJECT((n),"SG_Texture:*") )
#define SG_TEXTURE_NAMED(n)        SGTEXTURE( AG_OBJECT_NAMED((n),"SG_Texture:*") )
#define AG_CONST_TEXTURE_SELF()   SGCTEXTURE( AG_CONST_OBJECT(0,"SG_Texture:*") )
#define AG_CONST_TEXTURE_PTR(n)   SGCTEXTURE( AG_CONST_OBJECT((n),"SG_Texture:*") )
#define AG_CONST_TEXTURE_NAMED(n) SGCTEXTURE( AG_CONST_OBJECT_NAMED((n),"SG_Texture:*") )

__BEGIN_DECLS
extern AG_ObjectClass sgTextureClass;

SG_Texture *_Nonnull SG_TextureNew(void *_Nullable, const char *_Nullable);

int  SG_TextureCompile(SG_Texture *_Nonnull);
void SG_TextureBind(SG_Texture *_Nonnull, struct sg_view *_Nonnull);
void SG_TextureUnbind(SG_Texture *_Nonnull, struct sg_view *_Nonnull);

SG_TextureProgram *_Nullable SG_TextureAddProgram(SG_Texture *_Nonnull, SG_Program *_Nullable);
void                         SG_TextureDelProgram(SG_Texture *_Nonnull, SG_TextureProgram *_Nonnull);

SG_TextureSurface *_Nullable SG_TextureAddSurface(SG_Texture *_Nonnull, const AG_Surface *_Nonnull);
SG_TextureSurface *_Nullable SG_TextureAddSurfaceNODUP(SG_Texture *_Nonnull, AG_Surface *_Nonnull);
void                         SG_TextureDelSurface(SG_Texture *_Nonnull, SG_TextureSurface *_Nonnull);
__END_DECLS
