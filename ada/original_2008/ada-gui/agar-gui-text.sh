#!/bin/sh

(cat <<EOF
-- auto generated, do not edit
EOF
) || exit 112

cat agar-gui-text.ads.txt || exit 112

#----------------------------------------------------------------------
# widget_t base

(cat << EOF
  -- openGL array types
  type texcoord_t is array (1 .. 4) of c.c_float;

  -- glyph type
  type glyph_t is record
    fontname : font_name_t;
    fontsize : c.int;
    color    : agar.core.types.uint32_t;
    ch       : agar.core.types.uint32_t;
    nrefs    : agar.core.types.uint32_t;
    last_ref : agar.core.types.uint32_t;
    surface  : agar.gui.surface.surface_access_t;
    advance  : c.int;
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
    texture  : c.unsigned;
    texcoord : texcoord_t;
EOF
) || exit 112
fi

#----------------------------------------------------------------------

(cat <<EOF
    glyphs   : glyph_slist.entry_t;
  end record;
  pragma convention (c, glyph_t);

end agar.gui.text;
EOF
) || exit 112
