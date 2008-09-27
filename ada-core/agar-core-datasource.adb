package body agar.core.datasource is

  package cbinds is
    function open
      (path : cs.chars_ptr;
       mode : cs.chars_ptr) return datasource_access_t;
    pragma import (c, open, "AG_OpenFile");

    procedure set_error_callback
      (data_source : datasource_access_t;
       callback    : agar.core.event_types.callback_t;
       fmt         : agar.core.types.void_ptr_t);
    pragma import (c, set_error_callback, "AG_DataSourceSetErrorFn");

    procedure error
      (data_source : datasource_access_t;
       fmt         : cs.chars_ptr;
       message     : cs.chars_ptr);
    pragma import (c, error, "AG_DataSourceError");
  end cbinds;

  function open
    (path : string;
     mode : string) return datasource_access_t
  is
    ca_path : aliased c.char_array := c.to_c (path);
    ca_mode : aliased c.char_array := c.to_c (mode);
  begin
    return cbinds.open
      (path => cs.to_chars_ptr (ca_path'unchecked_access),
       mode => cs.to_chars_ptr (ca_mode'unchecked_access));
  end open;

  procedure set_error_callback
    (data_source : datasource_access_t;
     callback    : agar.core.event_types.callback_t) is
  begin
    cbinds.set_error_callback
      (data_source => data_source,
       callback    => callback,
       fmt         => agar.core.types.null_ptr);
  end set_error_callback;
    
  procedure error
    (data_source : datasource_access_t;
     message     : string)
  is
    ca_fmt : aliased c.char_array := c.to_c ("%s");
    ca_str : aliased c.char_array := c.to_c (message);
  begin
    cbinds.error
      (data_source => data_source,
       fmt         => cs.to_chars_ptr (ca_fmt'unchecked_access),
       message     => cs.to_chars_ptr (ca_str'unchecked_access));
  end error;

end agar.core.datasource;
