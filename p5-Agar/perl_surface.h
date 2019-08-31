/*	Public domain	*/

static const AP_FlagNames apSurfaceFlagNames[] = {
	{ "srcColorKey", AG_SURFACE_COLORKEY },
	{ "srcAlpha",    AG_SURFACE_ALPHA },
	{ "glTexture",   AG_SURFACE_GL_TEXTURE },
	{ "mapped",      AG_SURFACE_MAPPED },
	{ "static",      AG_SURFACE_STATIC },
	{ "extPixels",   AG_SURFACE_EXT_PIXELS },
	{ "animated",    AG_SURFACE_ANIMATED },
	{ "trace",       AG_SURFACE_TRACE },
	{ NULL,          0 }
};

typedef AG_PixelFormat * Agar__PixelFormat;
typedef AG_Surface * Agar__Surface;
