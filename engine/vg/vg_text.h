/*	$Csoft: vg_text.h,v 1.3 2004/05/05 16:45:35 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_TEXT_H_
#define _AGAR_VG_TEXT_H_
#include "begin_code.h"

#define VG_TEXT_MAX		256
#define VG_FONT_FACE_MAX	32
#define VG_FONT_STYLE_MAX	16
#define VG_FONT_SIZE_MIN	4
#define VG_FONT_SIZE_MAX	48
		
struct vg_text_args {
	SDL_Surface *su;		/* Text surface */
	char text[VG_TEXT_MAX];		/* Text buffer */
	double angle;			/* Angle of rotation (deg) */
	enum vg_alignment align;	/* Alignment of text */
	char style[VG_FONT_STYLE_MAX];	/* Text style */
	char face[VG_FONT_FACE_MAX];	/* Font face name */
	int size;			/* Font size */
	int flags;
#define VG_FONT_BOLD		0x01		/* Bold style */
#define VG_FONT_ITALIC		0x02		/* Italic style */
#define VG_FONT_UNDERLINE	0x04		/* Underlined */
#define VG_FONT_SCALED		0x08		/* Try to scale the text */
};

struct vg_text_style {
	const char name[VG_FONT_STYLE_MAX];		/* Name of style */
	const char face[VG_FONT_FACE_MAX];		/* Font face */
	int size;					/* Font size */
	int flags;					/* Font style */
	Uint32 color;					/* Font color */
	TAILQ_ENTRY(vg_text_style) txtstyles;
};

__BEGIN_DECLS
void	vg_text_init(struct vg *, struct vg_element *);
void	vg_text_destroy(struct vg *, struct vg_element *);
void	vg_text_align(struct vg *, enum vg_alignment);
void	vg_text_angle(struct vg *, double);
int	vg_text_font(struct vg *, const char *, int, int);
int	vg_text_style(struct vg *, const char *);
void	vg_printf(struct vg *, const char *, ...);
void	vg_draw_text(struct vg *, struct vg_element *);
void	vg_text_bbox(struct vg *, struct vg_element *, struct vg_rect *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_TEXT_H_ */
