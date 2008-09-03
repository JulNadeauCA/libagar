package agar.gui.widget.fixed is

  use type c.unsigned;

  subtype flags_t is c.unsigned;
  FIXED_HFILL     : constant flags_t := 16#01#;
  FIXED_VFILL     : constant flags_t := 16#02#;
  FIXED_NO_UPDATE : constant flags_t := 16#04#;
  FIXED_FILLBG    : constant flags_t := 16#08#;
  FIXED_BOX       : constant flags_t := 16#10#;
  FIXED_INVBOX    : constant flags_t := 16#20#;
  FIXED_FRAME     : constant flags_t := 16#40#;
  FIXED_EXPAND    : constant flags_t := FIXED_HFILL or FIXED_VFILL;

  type fixed_t is record
    widget     : widget_t;
    flags      : flags_t;
    width_pre  : c.int;
    height_pre : c.int;
  end record;
  type fixed_access_t is access all fixed_t;
  pragma convention (c, fixed_t);
  pragma convention (c, fixed_access_t);    

end agar.gui.widget.fixed;
