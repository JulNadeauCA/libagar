------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                           A G A R . T Y P E S                            --
--                                 S p e c                                  --
------------------------------------------------------------------------------
with Interfaces;

--
-- Primitive data types dependent on the Agar memory model.
--

package Agar.Types is

  ------------------
  -- Memory model --
  ------------------
  AG_MODEL  : constant Natural := $AG_MODEL;
  AG_SMALL  : constant Natural := $AG_SMALL;  --  8/16-bit CPUs, 12-bit color
  AG_MEDIUM : constant Natural := $AG_MEDIUM; -- 32/64-bit CPUs, 24-bit color
  AG_LARGE  : constant Natural := $AG_LARGE;  -- 32/64-bit CPUs, 48-bit color

  ---------------------------
  -- Native character type --
  ---------------------------
#if AG_UNICODE
  subtype Char is Interfaces.Unsigned_32;
#else
  subtype Char is Interfaces.Unsigned_8;
#end if;
  AG_CHAR_MAX : constant Natural := $AG_CHAR_MAX;
  
  -----------------------
  -- Sizes and offsets --
  -----------------------
#if AG_MODEL = AG_SMALL
 subtype Size is Interfaces.Unsigned_16;
 type Offset is range -(2 **15) .. +(2 **15 - 1) with Convention => C;
 for Offset'Size use 16;
#elsif AG_MODEL = AG_MEDIUM
 subtype Size is Interfaces.Unsigned_32;
 type Offset is range -(2 **31) .. +(2 **31 - 1) with Convention => C;
 for Offset'Size use 32;
#elsif AG_MODEL = AG_LARGE
 subtype Size is Interfaces.Unsigned_64;
 type Offset is range -(2 **63) .. +(2 **63 - 1) with Convention => C;
 for Offset'Size use 64;
#end if;

end Agar.Types;
