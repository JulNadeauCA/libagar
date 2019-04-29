/*	Public domain	*/

#ifdef AG_INLINE_HEADER
static __inline__ AG_Grayscale _Const_Attribute
AG_Grayscale_8(Uint8 v, Uint8 a)
#else
AG_Grayscale
ag_grayscale_8(Uint8 v, Uint8 a)
#endif
{
	AG_Grayscale G;
#if AG_MODEL == AG_LARGE
	G.v = AG_8to32(v);
	G.a = AG_8to32(a);
#else
	G.v = AG_8to16(v);
	G.a = AG_8to16(a);
#endif
	return (G);
}

#ifdef AG_INLINE_HEADER
static __inline__ AG_Grayscale _Const_Attribute
AG_Grayscale_16(Uint16 v, Uint16 a)
#else
AG_Grayscale
ag_grayscale_16(Uint16 v, Uint16 a)
#endif
{
	AG_Grayscale G;
#if AG_MODEL == AG_LARGE
	G.v = AG_16to32(v);
	G.a = AG_16to32(a);
#else
	G.v = v;
	G.a = a;
#endif
	return (G);
}

#ifdef AG_INLINE_HEADER
static __inline__ AG_Grayscale _Const_Attribute
AG_Grayscale_32(Uint32 v, Uint32 a)
#else
AG_Grayscale
ag_grayscale_32(Uint32 v, Uint32 a)
#endif
{
	AG_Grayscale G;
#if AG_MODEL == AG_LARGE
	G.v = v;
	G.a = a;
#else
	G.v = AG_32to16(v);
	G.a = AG_32to16(a);
#endif
	return (G);
}

/* Return AG_Color from 8- or 16-bit RGB (or RGB + alpha) components. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorRGB_8(AG_Color *_Nonnull c, Uint8 r, Uint8 g, Uint8 b)
#else
void
ag_color_rgb_8(AG_Color *c, Uint8 r, Uint8 g, Uint8 b)
#endif
{
#if (AG_MODEL == AG_LARGE)
	c->r = AG_8to16(r);
	c->g = AG_8to16(g);
	c->b = AG_8to16(b);
#else
	c->r = r;
	c->g = g;
	c->b = b;
#endif
	c->a = AG_OPAQUE;
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorRGBA_8(AG_Color *_Nonnull c, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
#else
void
ag_color_rgba_8(AG_Color *c, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
#endif
{
#if (AG_MODEL == AG_LARGE)
	c->r = AG_8to16(r);
	c->g = AG_8to16(g);
	c->b = AG_8to16(b);
	c->a = AG_8to16(a);
#else
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;
#endif
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorRGB_16(AG_Color *_Nonnull c, Uint16 r, Uint16 g, Uint16 b)
#else
void
ag_color_rgb_16(AG_Color *c, Uint16 r, Uint16 g, Uint16 b)
#endif
{
#if (AG_MODEL == AG_LARGE)
	c->r = r;
	c->g = g;
	c->b = b;
#else
	c->r = AG_16to8(r);
	c->g = AG_16to8(g);
	c->b = AG_16to8(b);
#endif
	c->a = AG_OPAQUE;
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorRGBA_16(AG_Color *_Nonnull c, Uint16 r, Uint16 g, Uint16 b, Uint16 a)
#else
void
ag_color_rgba_16(AG_Color *c, Uint16 r, Uint16 g, Uint16 b, Uint16 a)
#endif
{
#if (AG_MODEL == AG_LARGE)
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;
#else
	c->r = AG_16to8(r);
	c->g = AG_16to8(g);
	c->b = AG_16to8(b);
	c->a = AG_16to8(a);
#endif
}

/* Return AG_Color from 4-bit components packed as 0xRGBA. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorHex16(AG_Color *_Nonnull c, Uint16 h)
#else
void
ag_color_hex_16(AG_Color *c, Uint16 h)
#endif
{
#if (AG_MODEL == AG_LARGE)
	c->r = AG_4to16((h & 0xf000) >> 3);
	c->g = AG_4to16((h & 0x0f00) >> 2);
	c->b = AG_4to16((h & 0x00f0) >> 1);
	c->a = AG_4to16((h & 0x000f));
#else
	c->r = AG_4to8((h & 0xf000) >> 3);
	c->g = AG_4to8((h & 0x0f00) >> 2);
	c->b = AG_4to8((h & 0x00f0) >> 1);
	c->a = AG_4to8((h & 0x000f));
#endif
}

/* Return AG_Color from 8-bit components packed as 0xRRGGBBAA. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorHex32(AG_Color *_Nonnull c, Uint32 h)
#else
void
ag_color_hex_32(AG_Color *c, Uint32 h)
#endif
{
#if (AG_MODEL == AG_LARGE)
	c->r = AG_8to16((h & 0xff000000) >> 24);
	c->g = AG_8to16((h & 0x00ff0000) >> 16);
	c->b = AG_8to16((h & 0x0000ff00) >> 8);
	c->a = AG_8to16((h & 0x000000ff));
#else
	c->r = (h & 0xff000000) >> 24;
	c->g = (h & 0x00ff0000) >> 16;
	c->b = (h & 0x0000ff00) >> 8;
	c->a = (h & 0x000000ff);
#endif
}

#if AG_MODEL == AG_LARGE
/* Return AG_Color from 16-bit components packed as 0xRRRRGGGGBBBBAAAA. */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorHex64(AG_Color *_Nonnull c, Uint64 h)
# else
void
ag_color_hex_64(AG_Color *c, Uint64 h)
# endif
{
	c->r = (h & 0xffff000000000000) >> 48;
	c->g = (h & 0x0000ffff00000000) >> 32;
	c->b = (h & 0x00000000ffff0000) >> 16;
	c->a = (h & 0x000000000000ffff);
}
#endif /* AG_LARGE */

/* Component-wise clamped addition */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorAdd(AG_Color *_Nonnull dst, const AG_Color *_Nonnull c,
    const AG_ColorOffset *_Nonnull offs)
#else
void
ag_color_add(AG_Color *dst, const AG_Color *c, const AG_ColorOffset *offs)
#endif
{
	dst->r = AG_MIN(AG_COLOR_LAST, c->r + offs->r);
	dst->g = AG_MIN(AG_COLOR_LAST, c->g + offs->g);
	dst->b = AG_MIN(AG_COLOR_LAST, c->b + offs->b);
	dst->a = AG_MIN(AG_COLOR_LAST, c->a + offs->a);
}

/* Compare two colors. */
#ifdef AG_INLINE_HEADER
static __inline__ int _Const_Attribute
AG_ColorCompare(const AG_Color *_Nonnull A, const AG_Color *_Nonnull B)
#else
int
ag_color_compare(const AG_Color *A, const AG_Color *B)
#endif
{
	return !(A->r == B->r &&
	         A->g == B->g &&
	         A->b == B->b &&
	         A->a == B->a);
}

#ifdef AG_HAVE_FLOAT
/*
 * Convert between RGB and HSV color representations.
 * TODO make this a _Const_Attribute function returning an AG_ColorHSV.
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_Color2HSV(const AG_Color *_Nonnull c, float *_Nonnull h, float *_Nonnull s,
    float *_Nonnull v)
# else
void
ag_color_2_hsv(const AG_Color *c, float *h, float *s, float *v)
# endif
{
	AG_MapRGB_HSVf(c->r, c->g, c->b, h,s,v);
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_HSV2Color(float h, float s, float v, AG_Color *_Nonnull c)
# else
void
ag_hsv_2_color(float h, float s, float v, AG_Color *c)
# endif
{
	AG_MapHSVf_RGB(h,s,v, &c->r, &c->g, &c->b);
}
#endif /* HAVE_FLOAT */
