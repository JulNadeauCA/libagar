/*	Public domain	*/

#define VG_TEXT_MAX       256
#define VG_TEXT_MAX_PTRS  32
#define VG_FONT_FACE_MAX  32
#define VG_FONT_STYLE_MAX 16
#define VG_FONT_SIZE_MIN  4
#define VG_FONT_SIZE_MAX  48

typedef struct vg_text {
	struct vg_node _inherit;
	VG_Point *p;			/* Position */
	enum vg_alignment align;	/* Text alignment around point */
	float angle;			/* Text rotation (degs) */

	char fontFace[VG_FONT_FACE_MAX]; /* Font face */
	int  fontSize;			 /* Font size */
	Uint fontFlags;			 /* Font flags */
#define VG_TEXT_BOLD      0x01		 /* Bold style */
#define VG_TEXT_ITALIC    0x02		 /* Italic style */
#define VG_TEXT_UNDERLINE 0x04		 /* Underlined */
#define VG_TEXT_SCALED    0x08		 /* Try to scale the text */

	char text[VG_TEXT_MAX];		/* Text or format string */
	void *ptrs[VG_TEXT_MAX_PTRS];	/* Polled data */
	int  nPtrs;
} VG_Text;

#define VGTEXT(p) ((VG_Text *)(p))

__BEGIN_DECLS
extern const VG_NodeOps vgTextOps;

void VG_TextPrintf(VG_Text *, const char *, ...);
void VG_TextPrintfP(VG_Text *, const char *, ...);

static __inline__ VG_Text *
VG_TextNew(void *pNode, VG_Point *pt)
{
	VG_Text *vt;

	vt = AG_Malloc(sizeof(VG_Text));
	VG_NodeInit(vt, &vgTextOps);
	vt->p = pt;
	VG_AddRef(vt, pt);
	VG_NodeAttach(pNode, vt);
	return (vt);
}

static __inline__ void
VG_TextAlignment(VG_Text *vt, enum vg_alignment align)
{
	vt->align = align;
}
static __inline__ void
VG_TextAngle(VG_Text *vt, float angle)
{
	vt->angle = angle;
}
static __inline__ void
VG_TextFontFace(VG_Text *vt, const char *face)
{
	AG_Strlcpy(vt->fontFace, face, sizeof(vt->fontFace));
}
static __inline__ void
VG_TextFontSize(VG_Text *vt, int size)
{
	vt->fontSize = size;
}
static __inline__ void
VG_TextFontFlags(VG_Text *vt, Uint flags)
{
	vt->fontFlags = flags;
}
__END_DECLS
