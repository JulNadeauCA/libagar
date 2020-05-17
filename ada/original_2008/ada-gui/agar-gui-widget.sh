#!/bin/sh

(cat <<EOF
-- auto generated, do not edit
EOF
) || exit 112

cat agar-gui-widget.ads.txt || exit 112

#----------------------------------------------------------------------
# widget_t base

(cat << EOF
private

  -- openGL array types
  type clip_plane_state_t is array (1 .. 4) of aliased c.int;
  pragma convention (c, clip_plane_state_t);
  type clip_save_gl_row_t is array (1 .. 4) of aliased c.double;
  pragma convention (c, clip_save_gl_row_t);
  type clip_save_gl_t is array (1 .. 4) of aliased clip_save_gl_row_t;
  pragma convention (c, clip_save_gl_t);

  -- widget type
  type widget_private_t is record
    clip_save        : agar.gui.rect.rect_t;
    style            : agar.core.types.void_ptr_t; -- XXX: style_access_t
    surfaces         : access agar.gui.surface.surface_access_t;
    surface_flags    : access c.unsigned;
    nsurfaces        : c.unsigned;
EOF
) || exit 112

#----------------------------------------------------------------------
# opengl

opengl=`./chk-opengl`
if [ $? -ne 0 ]
then
  echo 'fatal: could not determine if HAVE_OPENGL is defined' 1>&2
  exit 112
fi

if [ "${opengl}" = "opengl" ]
then
  (cat <<EOF
    textures         : access c.unsigned;
    texcoords        : access c.c_float;
    texture_gc       : access c.unsigned;
    ntexture_gc      : c.unsigned;
    clip_plane_state : clip_plane_state_t;
    clip_save_gl     : clip_save_gl_t;
EOF
) || exit 112
fi

#----------------------------------------------------------------------

(cat <<EOF
    bindings_lock    : agar.core.threads.mutex_t;
    bindings         : binding_slist.head_t;
    menus            : menu_slist.head_t;
  end record;
  pragma convention (c, widget_private_t);

end agar.gui.widget;
EOF
) || exit 112
