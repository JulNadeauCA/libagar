package body agar.gui.unit is

  function find (key : string) return unit_const_access_t is
    ca_key : aliased c.char_array := c.to_c (key);
  begin
    return find (cs.to_chars_ptr (ca_key'unchecked_access));
  end find;

  function best
    (unit_group : unit_const_access_t;
     n          : long_float) return unit_const_access_t is
  begin
    return best (unit_group, c.double (n));
  end best;

  function abbreviation (unit : unit_const_access_t) return string is
  begin
    return cs.value (abbreviation (unit));
  end abbreviation;

  function unit_to_base
    (n          : long_float;
     unit_group : unit_const_access_t) return long_float
  is
    ret : constant c.double := unit_to_base (c.double (n), unit_group);
  begin
    return long_float (ret);
  end unit_to_base;
 
  function base_to_unit
    (n          : long_float;
     unit_group : unit_const_access_t) return long_float
  is
    ret : constant c.double := base_to_unit (c.double (n), unit_group);
  begin
    return long_float (ret);
  end base_to_unit;
 
  function unit_to_unit
    (n         : long_float;
     unit_from : unit_const_access_t;
     unit_to   : unit_const_access_t) return long_float
  is
    ret : constant c.double := unit_to_unit (c.double (n), unit_from, unit_to);
  begin
    return long_float (ret);
  end unit_to_unit;
 
end agar.gui.unit;
