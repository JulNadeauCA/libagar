package body agar.gui.unit is

  package cbinds is
    function find (key : cs.chars_ptr) return unit_const_access_t;
    pragma import (c, find, "AG_FindUnit");
  
    function best
      (unit_group : unit_const_access_t;
       n          : c.double) return unit_const_access_t;
    pragma import (c, best, "AG_BestUnit");
  
    function unit_to_base
      (n          : c.double;
       unit_group : unit_const_access_t) return c.double;
    pragma import (c, unit_to_base, "agar_unit2base");
  
    function base_to_unit
      (n          : c.double;
       unit_group : unit_const_access_t) return c.double;
    pragma import (c, base_to_unit, "agar_base2unit");
  
    function unit_to_unit
      (n         : c.double;
       unit_from : unit_const_access_t;
       unit_to   : unit_const_access_t) return c.double;
    pragma import (c, unit_to_unit, "agar_unit2unit");
  end cbinds;

  function find (key : string) return unit_const_access_t is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    return cbinds.find (cs.to_chars_ptr (ca_key'unchecked_access));
  end find;

  function best
    (unit_group : unit_const_access_t;
     n          : long_float) return unit_const_access_t is
  begin
    return cbinds.best (unit_group, c.double (n));
  end best;

  function abbreviation (unit : unit_const_access_t) return string is
  begin
    return cs.value (abbreviation (unit));
  end abbreviation;

  function unit_to_base
    (n          : long_float;
     unit_group : unit_const_access_t) return long_float
  is
    ret : constant c.double := cbinds.unit_to_base (c.double (n), unit_group);
  begin
    return long_float (ret);
  end unit_to_base;
 
  function base_to_unit
    (n          : long_float;
     unit_group : unit_const_access_t) return long_float
  is
    ret : constant c.double := cbinds.base_to_unit (c.double (n), unit_group);
  begin
    return long_float (ret);
  end base_to_unit;
 
  function unit_to_unit
    (n         : long_float;
     unit_from : unit_const_access_t;
     unit_to   : unit_const_access_t) return long_float
  is
    ret : constant c.double := cbinds.unit_to_unit (c.double (n), unit_from, unit_to);
  begin
    return long_float (ret);
  end unit_to_unit;
 
end agar.gui.unit;
