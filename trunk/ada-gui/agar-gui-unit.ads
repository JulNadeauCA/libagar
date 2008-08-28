with interfaces.c.strings;

package agar.gui.unit is
  package cs renames interfaces.c.strings;

  type unit_t is record
    key     : cs.chars_ptr;
    abbr    : cs.chars_ptr;
    name    : cs.chars_ptr;
    divider : c.double;
    func    : access function (x : c.double; y : c.int) return c.double;
  end record;
  type unit_access_t is access all unit_t;
  type unit_const_access_t is access constant unit_t;
  pragma convention (c, unit_t);
  pragma convention (c, unit_access_t);
  pragma convention (c, unit_const_access_t);

  function find (key : string) return unit_const_access_t;
  pragma inline (find);

  function best
    (unit_group : unit_const_access_t;
     n          : long_float) return unit_const_access_t;
  pragma inline (best);
 
  -- missing: AG_UnitFormat - docs/header type mismatch

  function abbreviation (unit : unit_const_access_t) return cs.chars_ptr;
  pragma import (c, abbreviation, "agar_unitabbr");

  function abbreviation (unit : unit_const_access_t) return string;
  pragma inline (abbreviation);

  function unit_to_base
    (n          : long_float;
     unit_group : unit_const_access_t) return long_float;
  pragma inline (unit_to_base);
 
  function base_to_unit
    (n          : long_float;
     unit_group : unit_const_access_t) return long_float;
  pragma inline (base_to_unit);
 
  function unit_to_unit
    (n         : long_float;
     unit_from : unit_const_access_t;
     unit_to   : unit_const_access_t) return long_float;
  pragma inline (unit_to_unit);
 
end agar.gui.unit;
