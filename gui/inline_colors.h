/*	Public domain	*/

/*
 * Return a Grayscale from 8-bit Value + 8-bit Alpha.
 */
#ifdef AG_INLINE_HEADER
static __inline__ AG_Grayscale 
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
#elif AG_MODEL == AG_MEDIUM
	G.v = AG_8to16(v);
	G.a = AG_8to16(a);
#elif AG_MODEL == AG_SMALL
	G.v = v;
	G.a = a;
#endif
	return (G);
}

/*
 * Return a Grayscale from 16-bit Value + 16-bit Alpha.
 */
#ifdef AG_INLINE_HEADER
static __inline__ AG_Grayscale
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
#elif AG_MODEL == AG_MEDIUM
	G.v = v;
	G.a = a;
#elif AG_MODEL == AG_SMALL
	G.v = AG_16to8(v);
	G.a = AG_16to8(a);
#endif
	return (G);
}

/*
 * Return a Grayscale from 32-bit Value + 32-bit Alpha.
 */
#ifdef AG_INLINE_HEADER
static __inline__ AG_Grayscale
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
#elif AG_MODEL == AG_MEDIUM
	G.v = AG_32to16(v);
	G.a = AG_32to16(a);
#elif AG_MODEL == AG_SMALL
	G.v = AG_32to8(v);
	G.a = AG_32to8(a);
#endif
	return (G);
}

/*
 * Set Color from 8-bit values R,G,B (full opaque).
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorRGB_8(AG_Color *_Nonnull c, Uint8 r, Uint8 g, Uint8 b)
#else
void
ag_color_rgb_8(AG_Color *c, Uint8 r, Uint8 g, Uint8 b)
#endif
{
#if AG_MODEL == AG_LARGE
	c->r = AG_8to16(r);
	c->g = AG_8to16(g);
	c->b = AG_8to16(b);
#elif AG_MODEL == AG_MEDIUM
	c->r = r;
	c->g = g;
	c->b = b;
#elif AG_MODEL == AG_SMALL
	c->r = AG_8to4(r);
	c->g = AG_8to4(g);
	c->b = AG_8to4(b);
#endif
	c->a = AG_OPAQUE;
}

/*
 * Set Color from 8-bit values R,G,B,A.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorRGBA_8(AG_Color *_Nonnull c, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
#else
void
ag_color_rgba_8(AG_Color *c, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
#endif
{
#if AG_MODEL == AG_LARGE
	c->r = AG_8to16(r);
	c->g = AG_8to16(g);
	c->b = AG_8to16(b);
	c->a = AG_8to16(a);
#elif AG_MODEL == AG_MEDIUM
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;
#elif AG_MODEL == AG_SMALL
	c->r = AG_8to4(r);
	c->g = AG_8to4(g);
	c->b = AG_8to4(b);
	c->a = AG_8to4(a);
#endif
}

/*
 * Set Color from 16-bit values R,G,B (full opaque).
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorRGB_16(AG_Color *_Nonnull c, Uint16 r, Uint16 g, Uint16 b)
#else
void
ag_color_rgb_16(AG_Color *c, Uint16 r, Uint16 g, Uint16 b)
#endif
{
#if AG_MODEL == AG_LARGE
	c->r = r;
	c->g = g;
	c->b = b;
#elif AG_MODEL == AG_MEDIUM
	c->r = AG_16to8(r);
	c->g = AG_16to8(g);
	c->b = AG_16to8(b);
#elif AG_MODEL == AG_SMALL
	c->r = AG_16to4(r);
	c->g = AG_16to4(g);
	c->b = AG_16to4(b);
#endif
	c->a = AG_OPAQUE;
}

/*
 * Set Color from 16-bit values R,G,B,A.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorRGBA_16(AG_Color *_Nonnull c, Uint16 r, Uint16 g, Uint16 b, Uint16 a)
#else
void
ag_color_rgba_16(AG_Color *c, Uint16 r, Uint16 g, Uint16 b, Uint16 a)
#endif
{
#if AG_MODEL == AG_LARGE
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;
#elif AG_MODEL == AG_MEDIUM
	c->r = AG_16to8(r);
	c->g = AG_16to8(g);
	c->b = AG_16to8(b);
	c->a = AG_16to8(a);
#elif AG_MODEL == AG_SMALL
	c->r = AG_16to4(r);
	c->g = AG_16to4(g);
	c->b = AG_16to4(b);
	c->a = AG_16to4(a);
#endif
}

/*
 * Set Color from 4-bit components packed as 0x[RGBA].
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorHex16(AG_Color *_Nonnull c, Uint16 h)
#else
void
ag_color_hex_16(AG_Color *c, Uint16 h)
#endif
{
#if AG_MODEL == AG_LARGE
	c->r = AG_4to16((h & 0xf000) >> 12);
	c->g = AG_4to16((h & 0x0f00) >> 8);
	c->b = AG_4to16((h & 0x00f0) >> 4);
	c->a = AG_4to16((h & 0x000f));
#elif AG_MODEL == AG_MEDIUM
	c->r = AG_4to8((h & 0xf000) >> 12);
	c->g = AG_4to8((h & 0x0f00) >> 8);
	c->b = AG_4to8((h & 0x00f0) >> 4);
	c->a = AG_4to8((h & 0x000f));
#elif AG_MODEL == AG_SMALL
	c->r = (h & 0xf000) >> 3;
	c->g = (h & 0x0f00) >> 2;
	c->b = (h & 0x00f0) >> 1;
	c->a = (h & 0x000f);
#endif
}

/*
 * Set Color from 8-bit components packed as 0x[RRGGBBAA].
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorHex32(AG_Color *_Nonnull c, Uint32 h)
#else
void
ag_color_hex_32(AG_Color *c, Uint32 h)
#endif
{
#if AG_MODEL == AG_LARGE
	c->r = AG_8to16((h & 0xff000000) >> 24);
	c->g = AG_8to16((h & 0x00ff0000) >> 16);
	c->b = AG_8to16((h & 0x0000ff00) >> 8);
	c->a = AG_8to16((h & 0x000000ff));
#elif AG_MODEL == AG_MEDIUM
	c->r = (h & 0xff000000) >> 24;
	c->g = (h & 0x00ff0000) >> 16;
	c->b = (h & 0x0000ff00) >> 8;
	c->a = (h & 0x000000ff);
#elif AG_MODEL == AG_SMALL
	c->r = AG_8to4((h & 0xff000000) >> 24);
	c->g = AG_8to4((h & 0x00ff0000) >> 16);
	c->b = AG_8to4((h & 0x0000ff00) >> 8);
	c->a = AG_8to4((h & 0x000000ff));
#endif
}

#if AG_MODEL == AG_LARGE
/*
 * Return AG_Color from 16-bit components packed as 0x[RRRRGGGGBBBBAAAA].
 */
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

/*
 * Compute the clamped sum of a color c and an offset offs.
 */
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

/*
 * Compute the sum of a color c and an offset offs added (factor) times
 * and clamped to AG_COLOR_LAST.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorAddScaled(AG_Color *_Nonnull dst, const AG_Color *_Nonnull c,
    const AG_ColorOffset *_Nonnull offs, int factor)
#else
void
ag_color_add_scaled(AG_Color *dst, const AG_Color *c,
    const AG_ColorOffset *offs, int factor)
#endif
{
	dst->r = AG_MIN(AG_COLOR_LAST, c->r + offs->r*factor);
	dst->g = AG_MIN(AG_COLOR_LAST, c->g + offs->g*factor);
	dst->b = AG_MIN(AG_COLOR_LAST, c->b + offs->b*factor);
	dst->a = AG_MIN(AG_COLOR_LAST, c->a + offs->a*factor);
}

/*
 * Compute the point in RGB space between c1 and c2 closest to (num/denom).
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorInterpolate(AG_Color *_Nonnull dst, const AG_Color *_Nonnull c1,
    const AG_Color *_Nonnull c2, int num, int denom)
#else
void
ag_color_interpolate(AG_Color *dst, const AG_Color *c1, const AG_Color *c2,
    int num, int denom)
#endif
{
	dst->r = c1->r + (c2->r - c1->r)*num/denom;
	dst->g = c1->g + (c2->g - c1->g)*num/denom;
	dst->b = c1->b + (c2->b - c1->b)*num/denom;
	dst->a = c1->a + (c2->a - c1->a)*num/denom;
}

/* Compute the difference between two colors. */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
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
#endif /* AG_HAVE_FLOAT */
