------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                          A G A R . C O N F I G                           --
--                                 S p e c                                  --
--                                                                          --
-- Copyright (c) 2018-2020, Julien Nadeau Carriere (vedge@csoft.net)        --
-- Copyright (c) 2010, coreland (mark@coreland.ath.cx)                      --
--                                                                          --
-- Permission to use, copy, modify, and/or distribute this software for any --
-- purpose with or without fee is hereby granted, provided that the above   --
-- copyright notice and this permission notice appear in all copies.        --
--                                                                          --
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES --
-- WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         --
-- MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  --
-- ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   --
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    --
-- ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  --
-- OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           --
------------------------------------------------------------------------------
with Interfaces.C;
with Interfaces.C.Strings;

--
-- Interface to Agar "application preferences" object. For a list of the
-- built-in settings, see AG_Config(3), "CONFIGURATION PARAMETERS". Extra,
-- application-specific settings can be defined using Agar.Variable.
--

package Agar.Config is
  function Load return Boolean;
  -- Load previously saved configuration settings from the data directory.

  function Save return Boolean;
  -- Save configuration settings to the application data directory.

  private
  
  package C renames Interfaces.C;
  package CS renames Interfaces.C.Strings;

  function AG_ConfigFind
    (Name      : in CS.chars_ptr;
     Dest_Path : in CS.chars_ptr;
     Dest_Len  : in C.size_t) return C.int
    with Import, Convention => C, Link_Name => "AG_ConfigFind";

  function AG_ConfigLoad return C.int
    with Import, Convention => C, Link_Name => "AG_ConfigLoad";

  function AG_ConfigSave return C.int
    with Import, Convention => C, Link_Name => "AG_ConfigSave";

end Agar.Config;
