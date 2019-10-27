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
#else
	G.v = AG_8to16(v);
	G.a = AG_8to16(a);
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
#else
	G.v = v;
	G.a = a;
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
#else
	G.v = AG_32to16(v);
	G.a = AG_32to16(a);
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
#else
	c->r = r;
	c->g = g;
	c->b = b;
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
#else
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;
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
#else
	c->r = AG_16to8(r);
	c->g = AG_16to8(g);
	c->b = AG_16to8(b);
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
#else
	c->r = AG_16to8(r);
	c->g = AG_16to8(g);
	c->b = AG_16to8(b);
	c->a = AG_16to8(a);
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
#else
	c->r = AG_4to8((h & 0xf000) >> 12);
	c->g = AG_4to8((h & 0x0f00) >> 8);
	c->b = AG_4to8((h & 0x00f0) >> 4);
	c->a = AG_4to8((h & 0x000f));
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
#else
	c->r = (h & 0xff000000) >> 24;
	c->g = (h & 0x00ff0000) >> 16;
	c->b = (h & 0x0000ff00) >> 8;
	c->a = (h & 0x000000ff);
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
 * Ligten a color c by a value of shade*8 (MD), or shade*11 (LG).
 * Leave alpha unchanged.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorLighten(AG_Color *_Nonnull c, int shade)
#else
void
ag_color_lighten(AG_Color *c, int shade)
#endif
{
#if AG_MODEL == AG_LARGE
	const int shadeOffs = (shade << 11);
#else
	const int shadeOffs = (shade << 3);
#endif
	if (c->r + shadeOffs < AG_COLOR_LAST-shadeOffs) { c->r += shadeOffs; }
	if (c->g + shadeOffs < AG_COLOR_LAST-shadeOffs) { c->g += shadeOffs; }
	if (c->b + shadeOffs < AG_COLOR_LAST-shadeOffs) { c->b += shadeOffs; }
}

/*
 * Darken a color c by a value of shade*8 (MD), or shade*11 (LG).
 * Leave alpha unchanged.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ColorDarken(AG_Color *_Nonnull c, int shade)
#else
void
ag_color_darken(AG_Color *c, int shade)
#endif
{
#if AG_MODEL == AG_LARGE
	const int shadeOffs = (shade << 11);
#else
	const int shadeOffs = (shade << 3);
#endif
	if (c->r - shadeOffs >= 0) { c->r -= shadeOffs; }
	if (c->g - shadeOffs >= 0) { c->g -= shadeOffs; }
	if (c->b - shadeOffs >= 0) { c->b -= shadeOffs; }
}

/*
 * Compute the point in RGB space between c1 and c2 closest to (num/denom).
 * Return the alpha from c1 unchanged.
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
	dst->a = c1->a;
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

/*
 * Convert between RGB and HSV color representations.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_Color2HSV(const AG_Color *_Nonnull c, float *_Nonnull h, float *_Nonnull s,
    float *_Nonnull v)
#else
void
ag_color_2_hsv(const AG_Color *c, float *h, float *s, float *v)
#endif
{
	AG_MapRGB_HSVf(c->r, c->g, c->b, h,s,v);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_HSV2Color(float h, float s, float v, AG_Color *_Nonnull c)
#else
void
ag_hsv_2_color(float h, float s, float v, AG_Color *c)
#endif
{
	AG_MapHSVf_RGB(h,s,v, &c->r, &c->g, &c->b);
}
