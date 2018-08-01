------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                           A G A R . E R R O R                            --
--                                 B o d y                                  --
--                                                                          --
-- Copyright (c) 2018, Julien Nadeau Carriere (vedge@hypertriton.com)       --
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
package body Agar.Error is
  function Get_Error return String is
  begin
    return C.To_Ada (CS.Value (AG_GetError));
  end;

  procedure Set_Error (Message : in String) is
    Ch_Message : aliased C.char_array := C.To_C (Message);
  begin
    AG_SetErrorS
      (Message => CS.To_Chars_Ptr (Ch_Message'Unchecked_Access));
  end;

  procedure Fatal_Error (Message : in String) is
    Ch_Message : aliased C.char_array := C.To_C (Message);
  begin
    AG_FatalError
      (Message => CS.To_Chars_Ptr (Ch_Message'Unchecked_Access));
  end;

  --
  -- Proxy procedure to call error callback from C code.
  --

  Error_Callback : Error_Callback_t := null;

  procedure Caller (Message : CS.chars_ptr) with Convention => C;

  procedure Caller (Message : in CS.chars_ptr) is
  begin
    if Error_Callback /= null then
      Error_Callback.all (C.To_Ada (CS.Value (Message)));
    end if;
  end;

  procedure Set_Fatal_Callback
    (Callback : Error_Callback_Not_Null_t) is
  begin
    Error_Callback := Callback;
    AG_SetFatalCallback (Caller'Access);
  end;

end Agar.Error;
