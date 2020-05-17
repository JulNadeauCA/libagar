/*	Public domain	*/

#ifndef VG_TEXT_MAX
#define VG_TEXT_MAX       252
#endif
#ifndef VG_TEXT_MAX_PTRS
#define VG_TEXT_MAX_PTRS  32
#endif
#ifndef VG_FONT_FACE_MAX
#define VG_FONT_FACE_MAX  32
#endif
#ifndef VG_FONT_STYLE_MAX
#define VG_FONT_STYLE_MAX 16
#endif
#ifndef VG_FONT_SIZE_MIN
#define VG_FONT_SIZE_MIN  4
#endif
#ifndef VG_FONT_SIZE_MAX
#define VG_FONT_SIZE_MAX  48
#endif

typedef struct vg_text {
	struct vg_node _inherit;
	VG_Point *_Nullable p1;		/* First line endpoint */
	VG_Point *_Nullable p2;		/* Second line endpoint */
	enum vg_alignment align;	/* Text alignment around line */

	char  fontFace[VG_FONT_FACE_MAX]; /* Font face */
	Uint  fontFlags;                  /* Font flags */
	float fontSize;                   /* Font size */

#define VG_TEXT_BOLD      0x01		 /* Bold style */
#define VG_TEXT_ITALIC    0x02		 /* Italic style */
#define VG_TEXT_UNDERLINE 0x04		 /* Underlined */
#define VG_TEXT_SCALED    0x08		 /* Try to scale the text */

	char text[VG_TEXT_MAX];		/* Text or format string */

	Uint            argsCount;	/* Number of arguments */
	Uint32 _pad;
	void *_Nullable args;		/* Argument data */
	int  *_Nullable argSizes;	/* Sizes of format strings in text */

	void *_Nullable vsObj;		/* Object for $(foo) expansion */
} VG_Text;

#define VGTEXT(p) ((VG_Text *)(p))

__BEGIN_DECLS
extern VG_NodeOps vgTextOps;

VG_Text *_Nonnull VG_TextNew(void *_Nullable, VG_Point *_Nonnull,
                             VG_Point *_Nonnull);

void VG_TextAlignment(VG_Text *_Nonnull, enum vg_alignment);
void VG_TextFontFace(VG_Text *_Nonnull, const char *_Nonnull);
void VG_TextFontSize(VG_Text *_Nonnull, float);
void VG_TextFontFlags(VG_Text *_Nonnull, Uint);
void VG_TextSubstObject(VG_Text *_Nonnull, void *_Nullable);

void VG_TextString(VG_Text *_Nonnull, const char *_Nullable);
void VG_TextPrintf(VG_Text *_Nonnull, const char *_Nullable, ...);
__END_DECLS
