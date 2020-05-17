------------------------------------------------------------------------------
--                             AGAR GUI LIBRARY                             --
--                         A G A R . I N I T _ G U I                        --
--                                 B o d y                                  --
--                                                                          --
-- Copyright (c) 2018-2019 Julien Nadeau Carriere (vedge@csoft.net)         --
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

package body Agar.Init_GUI is

  function Init_Graphics (Driver : in String) return Boolean
  is
    Ch_Driver : aliased C.char_array := C.To_C(Driver);
  begin
    return 0 = AG_InitGraphics
      (Driver => CS.To_Chars_Ptr(Ch_Driver'Unchecked_Access));
  end;

  function Init_GUI return Boolean is
  begin
    return 0 = AG_InitGUI
      (Flags => 0);
  end;

end Agar.Init_GUI;
