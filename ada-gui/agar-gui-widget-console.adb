package body agar.gui.widget.console is

  package cbinds is
    procedure set_padding
      (console : console_access_t;
       padding : c.int);
    pragma import (c, set_padding, "AG_ConsoleSetPadding");

    function message
      (console : console_access_t;
       fmt     : cs.chars_ptr;
       text    : cs.chars_ptr) return line_access_t;
    pragma import (c, message, "AG_ConsoleMsg");

    function append_line
      (console : console_access_t;
       text    : cs.chars_ptr) return line_access_t;
    pragma import (c, append_line, "AG_ConsoleAppendLine");
  end cbinds;

  procedure set_padding
    (console : console_access_t;
     padding : natural) is
  begin
    cbinds.set_padding
      (console => console,
       padding => c.int (padding));
  end set_padding;

  function message
    (console : console_access_t;
     text    : string) return line_access_t
  is
    ca_fmt  : aliased c.char_array := c.to_c ("%s");
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    return cbinds.message
      (console => console,
       fmt     => cs.to_chars_ptr (ca_fmt'unchecked_access),
       text    => cs.to_chars_ptr (ca_text'unchecked_access));
  end message;

  function append_line
    (console : console_access_t;
     text    : string) return line_access_t
  is
    ca_text : aliased c.char_array := c.to_c (text);
  begin
    return cbinds.append_line
      (console => console,
       text    => cs.to_chars_ptr (ca_text'unchecked_access));
  end append_line;

end agar.gui.widget.console;
