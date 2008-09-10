package body agar.core is

  use type c.int;
  use type cs.chars_ptr;

  package cbinds is
    function init
      (progname : cs.chars_ptr;
       flags    : init_flags_t := 0) return c.int;
    pragma import (c, init, "AG_InitCore");
  end cbinds;

  -- prevent memory leak by checking if progname is set
  ag_progname : cs.chars_ptr;
  pragma import (c, ag_progname, "agProgName");

  function init
    (progname : string;
     flags    : init_flags_t := 0) return boolean is
  begin
    if ag_progname /= cs.null_ptr then
      cs.free (ag_progname);
    end if;
    return cbinds.init
      (progname => cs.new_string (progname),
       flags    => flags) = 0;
  end init;

  procedure get_version (version : out version_t) is
    tmp_ver : aliased version_t;
  begin
    get_version (tmp_ver'unchecked_access);
    version := tmp_ver;
  end get_version;

  -- Register an error callback so that AG_FatalError raises
  -- an exception instead of aborting.

  procedure register_error_callback is
    procedure exception_raiser (err_str : cs.chars_ptr) is
    begin
      raise agar_error with cs.value (err_str);
    end exception_raiser;

    procedure set_callback
      (callback : access procedure (str : cs.chars_ptr));
    pragma import (c, set_callback, "AG_SetFatalCallback");
  begin
    set_callback (exception_raiser'access);
  end register_error_callback;

begin
  register_error_callback;  
end agar.core;
