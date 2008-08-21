package body agar.core.object is

  use type c.int;

  function root (object : object_access_t) return object_access_t is
  begin
    return object.root;
  end root;

  function parent (object : object_access_t) return object_access_t is
  begin
    return object.parent;
  end parent;

  function save (object : object_access_t) return c.int is
  begin
    return save_to_file (object, cs.null_ptr);
  end save;

  function save (object : object_access_t) return boolean is
  begin
    return save (object) = 0;
  end save;

  function save_all (object : object_access_t) return boolean is
  begin
    return save_all (object) = 0;
  end save_all;

  function load (object : object_access_t) return c.int is
  begin
    return load_from_file (object, cs.null_ptr);
  end load;

  function load_data
    (object : object_access_t;
     num    : access c.int) return c.int is
  begin
    return load_data_from_file (object, num, cs.null_ptr);
  end load_data;

  function load_generic (object : object_access_t) return c.int is
  begin
    return load_generic_from_file (object, cs.null_ptr);
  end load_generic;

  function allocate
    (parent : agar.core.types.void_ptr_t;
     name   : string;
     class  : class_access_t) return object_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return allocate
      (parent => parent,
       name   => cs.to_chars_ptr (ca_name'unchecked_access),
       class  => class);
  end allocate;

  function attach_to_named
    (vfsroot : object_access_t;
     path    : string;
     child   : object_access_t) return boolean
  is
    ca_path : aliased c.char_array := c.to_c (path);
  begin
    return attach_to_named
      (vfsroot => vfsroot,
       path    => cs.to_chars_ptr (ca_path'unchecked_access),
       child   => child) = 0;
  end attach_to_named;

  function save_to_file
    (object : object_access_t;
     name   : string) return boolean
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return save_to_file
      (object => object,
       name   => cs.to_chars_ptr (ca_name'unchecked_access)) = 0;
  end save_to_file;

  function load_from_file
    (object : object_access_t;
     name   : string) return boolean
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return load_from_file
      (object => object,
       name   => cs.to_chars_ptr (ca_name'unchecked_access)) = 0;
  end load_from_file;
 
  function load_generic_from_file
    (object : object_access_t;
     name   : string) return boolean
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return load_generic_from_file
      (object => object,
       name   => cs.to_chars_ptr (ca_name'unchecked_access)) = 0;
  end load_generic_from_file;

  function serialize
    (object : object_access_t;
     data   : agar.core.datasource.datasource_access_t) return boolean is
  begin
    return serialize
      (object => object,
       data   => data) = 0;
  end serialize;

  function unserialize
    (object : object_access_t;
     data   : agar.core.datasource.datasource_access_t) return boolean is
  begin
    return unserialize
      (object => object,
       data   => data) = 0;
  end unserialize;

  function page_in (object : object_access_t) return boolean is
  begin
    return page_in (object) = 0;
  end page_in;

  function page_out (object : object_access_t) return boolean is
  begin
    return page_out (object) = 0;
  end page_out;

  procedure set_save_prefix
    (object : object_access_t;
     prefix : string)
  is
    ca_prefix : aliased c.char_array := c.to_c (prefix);
  begin
    set_save_prefix
      (object => object,
       prefix => cs.to_chars_ptr (ca_prefix'unchecked_access));
  end set_save_prefix;

  function is_class_general
    (object  : object_access_t;
     pattern : string) return boolean
  is
    ca_pattern : aliased c.char_array := c.to_c (pattern);
  begin
    return is_class_general
      (object  => object,
       pattern => cs.to_chars_ptr (ca_pattern'unchecked_access)) = 1;
  end is_class_general;
 
  procedure set_archive_path
    (object : object_access_t;
     path   : string)
  is
    ca_path : aliased c.char_array := c.to_c (path);
  begin
    set_archive_path
      (object => object,
       path   => cs.to_chars_ptr (ca_path'unchecked_access));
  end set_archive_path;

  procedure set_name
    (object : object_access_t;
     name   : string)
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    set_name
      (object => object,
       name   => cs.to_chars_ptr (ca_name'unchecked_access));
  end set_name;
 
  function in_use (object : object_access_t) return boolean is
  begin
    return in_use (object) = 1;
  end in_use;

  function find
    (vfsroot : object_access_t;
     name    : string) return object_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return find
      (vfsroot => vfsroot,
       name    => cs.to_chars_ptr (ca_name'unchecked_access));
  end find;

end agar.core.object;
