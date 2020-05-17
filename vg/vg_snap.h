/*	Public domain	*/

enum vg_snap_mode {
	VG_FREE_POSITIONING,	/* No positional restriction */
	VG_GRID,		/* Snap to grid */
	VG_ENDPOINT,		/* Snap to endpoints */
	VG_ENDPOINT_DISTANCE,	/* Snap to given distance from endpoint */
	VG_CLOSEST_POINT,	/* Snap to closest point on entity */
	VG_CENTER_POINT,	/* Snap to center points */
	VG_MIDDLE_POINT,	/* Snap to middle points */
	VG_INTERSECTIONS_AUTO,	/* Snap to intersections automatically */
	VG_INTERSECTIONS_MANUAL	/* Snap to intersections manually */
};

struct ag_menu;
struct ag_menu_item;

__BEGIN_DECLS
struct ag_toolbar *_Nonnull VG_SnapToolbar(void *_Nullable,
                                           struct vg_view *_Nonnull, int);
void		            VG_SnapMenu(struct ag_menu_item *_Nonnull,
                                        struct vg_view *_Nonnull);
__END_DECLS
