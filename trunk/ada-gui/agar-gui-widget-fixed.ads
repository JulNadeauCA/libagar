generic
  type child_type is limited private;
  type child_access_type is access child_type;

package agar.gui.widget.fixed is

  use type c.unsigned;

  type flags_t is new c.unsigned;
  FIXED_HFILL     : constant flags_t := 16#01#;
  FIXED_VFILL     : constant flags_t := 16#02#;
  FIXED_NO_UPDATE : constant flags_t := 16#04#;
  FIXED_FILLBG    : constant flags_t := 16#08#;
  FIXED_BOX       : constant flags_t := 16#10#;
  FIXED_INVBOX    : constant flags_t := 16#20#;
  FIXED_FRAME     : constant flags_t := 16#40#;
  FIXED_EXPAND    : constant flags_t := FIXED_HFILL or FIXED_VFILL;

  type fixed_t is record
    widget     : aliased widget_t;
    flags      : flags_t;
    width_pre  : c.int;
    height_pre : c.int;
  end record;
  type fixed_access_t is access all fixed_t;
  pragma convention (c, fixed_t);
  pragma convention (c, fixed_access_t);    

  -- name uniformity
  subtype child_t is child_type;
  subtype child_access_t is child_access_type;

  -- API
  procedure put
    (fixed : fixed_access_t;
     child : child_access_t;
     x     : natural;
     y     : natural);
  pragma inline (put);

  procedure delete
    (fixed : fixed_access_t;
     child : child_access_t);
  pragma import (c, delete, "AG_FixedDel");

  procedure size
    (fixed  : fixed_access_t;
     child  : child_access_t;
     width  : positive;
     height : positive);
  pragma inline (size);

  procedure move
    (fixed : fixed_access_t;
     child : child_access_t;
     x     : natural;
     y     : natural);
  pragma inline (move);
 
  function widget (fixed : fixed_access_t) return widget_access_t;
  pragma inline (widget);

end agar.gui.widget.fixed;
