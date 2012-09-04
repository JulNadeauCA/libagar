/*	Public domain	*/

#define VG_TEXT_MAX       256
#define VG_TEXT_MAX_PTRS  32
#define VG_FONT_FACE_MAX  32
#define VG_FONT_STYLE_MAX 16
#define VG_FONT_SIZE_MIN  4
#define VG_FONT_SIZE_MAX  48

typedef struct vg_text {
	struct vg_node _inherit;
	VG_Point *p1, *p2;		/* Position line */
	enum vg_alignment align;	/* Text alignment around line */

	char fontFace[VG_FONT_FACE_MAX]; /* Font face */
	int  fontSize;			 /* Font size */
	Uint fontFlags;			 /* Font flags */
#define VG_TEXT_BOLD      0x01		 /* Bold style */
#define VG_TEXT_ITALIC    0x02		 /* Italic style */
#define VG_TEXT_UNDERLINE 0x04		 /* Underlined */
#define VG_TEXT_SCALED    0x08		 /* Try to scale the text */

	char text[VG_TEXT_MAX];		/* Text or format string */
	AG_List *args;			/* Text arguments */
	int     *argSizes;		/* Sizes of format strings in text */

	void *vsObj;			/* Object for $(foo) expansion */
} VG_Text;

#define VGTEXT(p) ((VG_Text *)(p))

__BEGIN_DECLS
extern VG_NodeOps vgTextOps;

void VG_TextString(VG_Text *, const char *);
void VG_TextPrintf(VG_Text *, const char *, ...);

static __inline__ VG_Text *
VG_TextNew(void *pNode, VG_Point *p1, VG_Point *p2)
{
	VG_Text *vt;

	vt = (VG_Text *)AG_Malloc(sizeof(VG_Text));
	VG_NodeInit(vt, &vgTextOps);
	vt->p1 = p1;
	vt->p2 = p2;
	VG_AddRef(vt, p1);
	VG_AddRef(vt, p2);
	VG_NodeAttach(pNode, vt);
	return (vt);
}

static __inline__ void
VG_TextAlignment(VG_Text *vt, enum vg_alignment align)
{
	VG_Lock(VGNODE(vt)->vg);
	vt->align = align;
	VG_Unlock(VGNODE(vt)->vg);
}
static __inline__ void
VG_TextFontFace(VG_Text *vt, const char *face)
{
	VG_Lock(VGNODE(vt)->vg);
	AG_Strlcpy(vt->fontFace, face, sizeof(vt->fontFace));
	VG_Unlock(VGNODE(vt)->vg);
}
static __inline__ void
VG_TextFontSize(VG_Text *vt, int size)
{
	VG_Lock(VGNODE(vt)->vg);
	vt->fontSize = size;
	VG_Unlock(VGNODE(vt)->vg);
}
static __inline__ void
VG_TextFontFlags(VG_Text *vt, Uint flags)
{
	VG_Lock(VGNODE(vt)->vg);
	vt->fontFlags = flags;
	VG_Unlock(VGNODE(vt)->vg);
}
static __inline__ void
VG_TextSubstObject(VG_Text *vt, void *obj)
{
	VG_Lock(VGNODE(vt)->vg);
	vt->vsObj = obj;
	VG_Unlock(VGNODE(vt)->vg);
}
__END_DECLS
