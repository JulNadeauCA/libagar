with ada.exceptions;
with ada.text_io;
with agar.core.error;

procedure ada_except is
  package ex renames ada.exceptions;
  package io renames ada.text_io;
begin
  agar.core.error.fatal ("caught agar_error exception");
exception
  when e : agar.agar_error =>
    io.put_line ("pass: " & ex.exception_message (e));
end ada_except;

