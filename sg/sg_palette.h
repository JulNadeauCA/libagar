/*	Public domain	*/

#define SG_PALETTE_PIGMENTS_MAX 1000000

typedef struct sg_pigment {
	Uint id;
	char name[64];		/* Pigment name */
	char ciName[28];	/* C.I. name */
	M_Real Tr;		/* Transparency (Very Opaque -> Transparent) */
	M_Real St;		/* Staining (Nonstaining -> Heavily Staining) */
	M_Real VR;		/* Masstone value vs. white paper (0-100) */
	M_Real Gr;		/* Granulation (Liquid -> Granular) */
	M_Real Bl;		/* Blossom (No blossom -> Strong blossom) */
	M_Real Df;		/* Diffusion (Inert -> Very active) */
	M_Real HA;		/* Hue Angle in CIELAB a*b* plane degrees */
	M_Real HS;		/* Undertone vs. Masstone hue shift */
	M_Real LfTint;		/* Lightfastness in tint (Very fugitive -> Very Lightfast) */
	M_Real LfMass;		/* Lightfastness in masstone (Very fugitive -> Very Lightfast) */
	AG_TAILQ_ENTRY(sg_pigment) pigments;
} SG_Pigment;

typedef struct sg_mixture {
	char name[64];			/* Mixture identifier */
	M_Real h, s, v;			/* Mixing point */
	SG_Pigment *_Nonnull mix[2];	/* Mixing colors */
	SG_Pigment *_Nullable adj;	/* Adjusting color */
	AG_TAILQ_ENTRY(sg_mixture) mixtures;
} SG_Mixture;

typedef struct sg_palette {
	struct ag_object _inherit;	/* AG_Object -> SG_Palette */
	Uint flags;
#define SG_PALETTE_SAVED 0
	int lastKeyDown;

	AG_TAILQ_HEAD_(sg_pigment) pigments;
	AG_TAILQ_HEAD_(sg_mixture) mixtures;
} SG_Palette;

__BEGIN_DECLS
extern AG_ObjectClass sgPaletteClass;

SG_Palette *_Nonnull  SG_PaletteNew(void *_Nullable, const char *_Nullable);
SG_Pigment *_Nullable SG_PaletteAddPigment(SG_Palette *_Nonnull);
void                  SG_PaletteDelPigment(SG_Palette *_Nonnull, SG_Pigment *_Nonnull);
SG_Mixture *_Nullable SG_PaletteAddMixture(SG_Palette *_Nonnull);
void                  SG_PaletteDelMixture(SG_Palette *_Nonnull, SG_Mixture *_Nonnull);
__END_DECLS
