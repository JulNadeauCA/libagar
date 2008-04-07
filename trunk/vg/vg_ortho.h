/*	Public domain	*/

enum vg_ortho_mode {
	VG_NO_ORTHO,		/* No orthogonal restriction */
	VG_HORIZ_ORTHO,		/* Horizontal restriction */
	VG_VERT_ORTHO		/* Vertical restriction */
};

struct ag_toolbar;

__BEGIN_DECLS
void		   VG_RestrictOrtho(struct vg_view *, VG_Vector *);
struct ag_toolbar *VG_OrthoRestrictToolbar(void *, struct vg_view *, int);
__END_DECLS
