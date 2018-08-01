------------------------------------------------------------------------------
--                            AGAR CORE LIBRARY                             --
--                       A G A R . C O R E . I N I T                        --
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
package body Agar.Init is

  procedure Get_Version
    (Major   : out Natural;
     Minor   : out Natural;
     Patch   : out Natural)
  is
    Version : aliased Agar_Version_t;
  begin
    AG_GetVersion (Version'Unchecked_Access);

    Major   := Natural (Version.Major);
    Minor   := Natural (Version.Minor);
    Patch   := Natural (Version.Patch);
  end Get_Version;
  
  function Init_Core
    (Program_Name     : in String;
     Verbose          : in Boolean := False;
     Create_Directory : in Boolean := False;
     Software_Timers  : in Boolean := False) return Boolean
  is
    Ch_Name : aliased C.char_array := C.To_C (Program_Name);
    C_Flags : C.unsigned := 0;
  begin
    if Verbose          then C_Flags := C_Flags or AG_VERBOSE;        end if;
    if Create_Directory then C_Flags := C_Flags or AG_CREATE_DATADIR; end if;
    if Software_Timers  then C_Flags := C_Flags or AG_SOFT_TIMERS;    end if;

    return 0 = AG_InitCore
      (Progname => CS.To_Chars_Ptr (Ch_Name'Unchecked_Access),
       Flags    => C_Flags);
  end;

  function Init_Core
    (Verbose          : in Boolean := False;
     Create_Directory : in Boolean := False;
     Software_Timers  : in Boolean := False) return Boolean
  is
    C_Flags : C.unsigned := 0;
  begin
    if Verbose          then C_Flags := C_Flags or AG_VERBOSE;        end if;
    if Create_Directory then C_Flags := C_Flags or AG_CREATE_DATADIR; end if;
    if Software_Timers  then C_Flags := C_Flags or AG_SOFT_TIMERS;    end if;

    return 0 = AG_InitCore
      (Progname => CS.Null_Ptr,
       Flags    => C_Flags);
  end;

  --
  -- Proxy procedure to call 'Exit_Callback' from C.
  --

  procedure At_Exit_Caller with Convention => C;

  Exit_Callback : Exit_Procedure_t := null;

  procedure At_Exit_Caller is
  begin
    if Exit_Callback /= null then
      Exit_Callback.all;
    end if;
  end At_Exit_Caller;

  procedure At_Exit
    (Callback : Exit_Procedure_Not_Null_t) is
  begin
    Exit_Callback := Callback;
    AG_AtExitFunc (At_Exit_Caller'Access);
  end At_Exit;

end Agar.Init;
