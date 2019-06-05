package body agar.gui.widget.console is

  package cbinds is
    procedure set_padding
      (console : console_access_t;
       padding : c.int);
    pragma import (c, set_padding, "AG_ConsoleSetPadding");

    function message
      (console : console_access_t;
       text    : cs.chars_ptr) return line_access_t;
    pragma import (c, message, "AG_ConsoleMsgS");

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

  function widget (console : console_access_t) return widget_access_t is
  begin
    return console.widget'access;
  end widget;

end agar.gui.widget.console;
