with Ada.Text_IO;

package body myatexit is
  procedure atexit is
  begin
    Ada.Text_IO.Put_Line("myatexit callback...");
  end atexit;
end myatexit;
