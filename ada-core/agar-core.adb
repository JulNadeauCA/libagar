package body agar.core is

  use type c.int;

  function init
    (progname : string;
     flags    : init_flags_t := 0) return boolean
  is
    ca_progname : aliased c.char_array := c.to_c (progname);
  begin
    return init
      (progname => cs.to_chars_ptr (ca_progname'unchecked_access),
       flags    => flags) = 0;
  end init;

  procedure get_version (version : out version_t) is
    tmp_ver : aliased version_t;
  begin
    get_version (tmp_ver'unchecked_access);
    version := tmp_ver;
  end get_version;

end agar.core;
