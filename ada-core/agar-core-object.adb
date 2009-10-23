package body agar.core.object is

  use type c.int;

  package cbinds is
    function allocate
      (parent : agar.core.types.void_ptr_t;
       name   : cs.chars_ptr;
       class  : class_access_t) return object_access_t;
    pragma import (c, allocate, "AG_ObjectNew");

    function attach_to_named
      (vfsroot : object_access_t;
       path    : cs.chars_ptr;
       child   : object_access_t) return c.int;
    pragma import (c, attach_to_named, "AG_ObjectAttachToNamed");

    function find
      (vfsroot : object_access_t;
       name    : cs.chars_ptr) return object_access_t;
    pragma import (c, find, "AG_ObjectFind");

    function save_to_file
      (object : object_access_t;
       name   : cs.chars_ptr) return c.int;
    pragma import (c, save_to_file, "AG_ObjectSaveToFile");

    function load_from_file
      (object : object_access_t;
       name   : cs.chars_ptr) return c.int;
    pragma import (c, load_from_file, "AG_ObjectLoadFromFile");

    function load_generic_from_file
      (object : object_access_t;
       name   : cs.chars_ptr) return c.int;
    pragma import (c, load_generic_from_file, "AG_ObjectLoadGenericFromFile");

    procedure register_namespace
      (name   : cs.chars_ptr;
       prefix : cs.chars_ptr;
       url    : cs.chars_ptr);
    pragma import (c, register_namespace, "AG_RegisterNamespace");

    procedure unregister_namespace (name : cs.chars_ptr);
    pragma import (c, unregister_namespace, "AG_UnregisterNamespace");

    function lookup_class (spec : cs.chars_ptr) return class_access_t;
    pragma import (c, lookup_class, "AG_LookupClass");

    function load_class (spec : cs.chars_ptr) return class_access_t;
    pragma import (c, load_class, "AG_LoadClass");

    procedure register_module_directory (path : cs.chars_ptr);
    pragma import (c, register_module_directory, "AG_RegisterModuleDirectory");

    procedure unregister_module_directory (path : cs.chars_ptr);
    pragma import (c, unregister_module_directory, "AG_UnregisterModuleDirectory");

    function of_class
      (object  : object_access_t;
       pattern : cs.chars_ptr) return c.int;
    pragma import (c, of_class, "agar_object_of_class");

    procedure set_name
      (object : object_access_t;
       name   : cs.chars_ptr);
    pragma import (c, set_name, "AG_ObjectSetNameS");

  end cbinds;

  --

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
    return cbinds.save_to_file (object, cs.null_ptr);
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
    return cbinds.load_from_file (object, cs.null_ptr);
  end load;

  function load_data
    (object : object_access_t;
     num    : access c.int) return c.int is
  begin
    return load_data_from_file (object, num, cs.null_ptr);
  end load_data;

  function load_generic (object : object_access_t) return c.int is
  begin
    return cbinds.load_generic_from_file (object, cs.null_ptr);
  end load_generic;

  function allocate
    (parent : agar.core.types.void_ptr_t;
     name   : string;
     class  : class_access_t) return object_access_t
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return cbinds.allocate
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
    return cbinds.attach_to_named
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
    return cbinds.save_to_file
      (object => object,
       name   => cs.to_chars_ptr (ca_name'unchecked_access)) = 0;
  end save_to_file;

  function load_from_file
    (object : object_access_t;
     name   : string) return boolean
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return cbinds.load_from_file
      (object => object,
       name   => cs.to_chars_ptr (ca_name'unchecked_access)) = 0;
  end load_from_file;

  function load_generic_from_file
    (object : object_access_t;
     name   : string) return boolean
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    return cbinds.load_generic_from_file
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

  procedure set_name
    (object : object_access_t;
     name   : string)
  is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    cbinds.set_name
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
    return cbinds.find
      (vfsroot => vfsroot,
       name    => cs.to_chars_ptr (ca_name'unchecked_access));
  end find;

  procedure register_namespace
    (name   : string;
     prefix : string;
     url    : string)
  is
    ca_name : aliased c.char_array := c.to_c (name);
    ca_pref : aliased c.char_array := c.to_c (prefix);
    ca_url  : aliased c.char_array := c.to_c (url);
  begin
    cbinds.register_namespace
      (name   => cs.to_chars_ptr (ca_name'unchecked_access),
       prefix => cs.to_chars_ptr (ca_pref'unchecked_access),
       url    => cs.to_chars_ptr (ca_url'unchecked_access));
  end register_namespace;

  procedure unregister_namespace (name : string) is
    ca_name : aliased c.char_array := c.to_c (name);
  begin
    cbinds.unregister_namespace (cs.to_chars_ptr (ca_name'unchecked_access));
  end unregister_namespace;

  function lookup_class (spec : string) return class_access_t is
    ca_spec : aliased c.char_array := c.to_c (spec);
  begin
    return cbinds.lookup_class (cs.to_chars_ptr (ca_spec'unchecked_access));
  end lookup_class;

  function load_class (spec : string) return class_access_t is
    ca_spec : aliased c.char_array := c.to_c (spec);
  begin
    return cbinds.load_class (cs.to_chars_ptr (ca_spec'unchecked_access));
  end load_class;

  procedure register_module_directory (path : string) is
    ca_path : aliased c.char_array := c.to_c (path);
  begin
    cbinds.register_module_directory (cs.to_chars_ptr (ca_path'unchecked_access));
  end register_module_directory;

  procedure unregister_module_directory (path : string) is
    ca_path : aliased c.char_array := c.to_c (path);
  begin
    cbinds.unregister_module_directory (cs.to_chars_ptr (ca_path'unchecked_access));
  end unregister_module_directory;

  function of_class
    (object  : object_access_t;
     pattern : string) return boolean
  is
    ca_pattern : aliased c.char_array := c.to_c (pattern);
  begin
    return cbinds.of_class
      (object  => object,
       pattern => cs.to_chars_ptr (ca_pattern'unchecked_access)) = 0;
  end of_class;

  -- prop API
  package body prop is

    package cbinds is
      function load
        (object      : object_access_t;
         data_source : agar.core.datasource.datasource_access_t) return c.int;
      pragma import (c, load, "AG_PropLoad");
  
      function save
        (object      : object_access_t;
         data_source : agar.core.datasource.datasource_access_t) return c.int;
      pragma import (c, save, "AG_PropSave");
  
      function set_boolean
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : c.int) return prop_access_t;
      pragma import (c, set_boolean, "agar_object_prop_set_boolean");
  
      function set_float
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : c.c_float) return prop_access_t;
      pragma import (c, set_float, "agar_object_prop_set_float");
  
      function set_int
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : c.int) return prop_access_t;
      pragma import (c, set_int, "agar_object_prop_set_int");
  
      function set_int16
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : agar.core.types.int16_t) return prop_access_t;
      pragma import (c, set_int16, "agar_object_prop_set_int16");
  
      function set_int32
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : agar.core.types.int32_t) return prop_access_t;
      pragma import (c, set_int32, "agar_object_prop_set_int32");
  
      function set_int64
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : agar.core.types.int64_t) return prop_access_t;
      pragma import (c, set_int64, "agar_object_prop_set_int64");
  
      function set_long_float
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : c.double) return prop_access_t;
      pragma import (c, set_long_float, "agar_object_prop_set_long_float");
  
      function set_int8
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : agar.core.types.int8_t) return prop_access_t;
      pragma import (c, set_int8, "agar_object_prop_set_int8");
  
      function set_long_long_float
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : c.long_double) return prop_access_t;
      pragma import (c, set_long_long_float, "agar_object_prop_set_long_long_float");
  
      function set_pointer
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : agar.core.types.void_ptr_t) return prop_access_t;
      pragma import (c, set_pointer, "agar_object_prop_set_pointer");
  
      function set_uint16
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : agar.core.types.uint16_t) return prop_access_t;
      pragma import (c, set_uint16, "agar_object_prop_set_uint16");
  
      function set_uint32
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : agar.core.types.uint32_t) return prop_access_t;
      pragma import (c, set_uint32, "agar_object_prop_set_uint32");
  
      function set_uint64
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : agar.core.types.uint64_t) return prop_access_t;
      pragma import (c, set_uint64, "agar_object_prop_set_uint64");
  
      function set_uint8
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : agar.core.types.uint8_t) return prop_access_t;
      pragma import (c, set_uint8, "agar_object_prop_set_uint8");
  
      function set_unsigned_int
        (object : object_access_t;
         key    : cs.chars_ptr;
         value  : c.unsigned) return prop_access_t;
      pragma import (c, set_unsigned_int, "agar_object_prop_set_unsigned_int");
  
      function get_boolean
        (object : object_access_t;
         key    : cs.chars_ptr) return c.int;
      pragma import (c, get_boolean, "agar_object_prop_get_boolean");
  
      function get_float
        (object : object_access_t;
         key    : cs.chars_ptr) return c.c_float;
      pragma import (c, get_float, "agar_object_prop_get_float");
  
      function get_int
        (object : object_access_t;
         key    : cs.chars_ptr) return c.int;
      pragma import (c, get_int, "agar_object_prop_get_int");
  
      function get_int16
        (object : object_access_t;
         key    : cs.chars_ptr) return agar.core.types.int16_t;
      pragma import (c, get_int16, "agar_object_prop_get_int16");
  
      function get_int32
        (object : object_access_t;
         key    : cs.chars_ptr) return agar.core.types.int32_t;
      pragma import (c, get_int32, "agar_object_prop_get_int32");
  
      function get_int64
        (object : object_access_t;
         key    : cs.chars_ptr) return agar.core.types.int64_t;
      pragma import (c, get_int64, "agar_object_prop_get_int64");
  
      function get_int8
        (object : object_access_t;
         key    : cs.chars_ptr) return agar.core.types.int8_t;
      pragma import (c, get_int8, "agar_object_prop_get_int8");
  
      function get_long_float
        (object : object_access_t;
         key    : cs.chars_ptr) return c.double;
      pragma import (c, get_long_float, "agar_object_prop_get_long_float");
  
      function get_long_long_float
        (object : object_access_t;
         key    : cs.chars_ptr) return c.long_double;
      pragma import (c, get_long_long_float, "agar_object_prop_get_long_long_float");
  
      function get_pointer
        (object : object_access_t;
         key    : cs.chars_ptr) return agar.core.types.void_ptr_t;
      pragma import (c, get_pointer, "agar_object_prop_get_pointer");
  
      function get_uint16
        (object : object_access_t;
         key    : cs.chars_ptr) return agar.core.types.uint16_t;
      pragma import (c, get_uint16, "agar_object_prop_get_uint16");
  
      function get_uint32
        (object : object_access_t;
         key    : cs.chars_ptr) return agar.core.types.uint32_t;
      pragma import (c, get_uint32, "agar_object_prop_get_uint32");
  
      function get_uint64
        (object : object_access_t;
         key    : cs.chars_ptr) return agar.core.types.uint64_t;
      pragma import (c, get_uint64, "agar_object_prop_get_uint64");
  
      function get_uint8
        (object : object_access_t;
         key    : cs.chars_ptr) return agar.core.types.uint8_t;
      pragma import (c, get_uint8, "agar_object_prop_get_uint8");
  
      function get_unsigned_int
        (object : object_access_t;
         key    : cs.chars_ptr) return c.unsigned;
      pragma import (c, get_unsigned_int, "agar_object_prop_get_unsigned_int");
    end cbinds;

    function load
      (object      : object_access_t;
       data_source : agar.core.datasource.datasource_access_t) return boolean is
    begin
      return cbinds.load (object, data_source) = 0;
    end load;

    function save
      (object      : object_access_t;
       data_source : agar.core.datasource.datasource_access_t) return boolean is
    begin
      return cbinds.save (object, data_source) = 0;
    end save;

    function set_boolean
      (object : object_access_t;
       key    : string;
       value  : boolean) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
      c_int : c.int := 0;
    begin
      if value then c_int := 1; end if;
      return cbinds.set_boolean
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => c_int);
    end set_boolean;

    function set_float
      (object : object_access_t;
       key    : string;
       value  : float) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_float
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => c.c_float (value));
    end set_float;

    function set_integer
      (object : object_access_t;
       key    : string;
       value  : integer) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_int
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => c.int (value));
    end set_integer;

    function set_int16
      (object : object_access_t;
       key    : string;
       value  : agar.core.types.int16_t) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_int16
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => value);
    end set_int16;

    function set_int32
      (object : object_access_t;
       key    : string;
       value  : agar.core.types.int32_t) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_int32
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => value);
    end set_int32;

    function set_int64
      (object : object_access_t;
       key    : string;
       value  : agar.core.types.int64_t) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_int64
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => value);
    end set_int64;

    function set_int8
      (object : object_access_t;
       key    : string;
       value  : agar.core.types.int8_t) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_int8
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => value);
    end set_int8;

    function set_long_float
      (object : object_access_t;
       key    : string;
       value  : long_float) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_long_float
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => c.double (value));
    end set_long_float;

    function set_long_long_float
      (object : object_access_t;
       key    : string;
       value  : long_long_float) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_long_long_float
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => c.long_double (value));
    end set_long_long_float;

    function set_pointer
      (object : object_access_t;
       key    : string;
       value  : agar.core.types.void_ptr_t) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_pointer
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => value);
    end set_pointer;

    function set_uint16
      (object : object_access_t;
       key    : string;
       value  : agar.core.types.uint16_t) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_uint16
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => value);
    end set_uint16;

    function set_uint32
      (object : object_access_t;
       key    : string;
       value  : agar.core.types.uint32_t) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_uint32
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => value);
    end set_uint32;

    function set_uint64
      (object : object_access_t;
       key    : string;
       value  : agar.core.types.uint64_t) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_uint64
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => value);
    end set_uint64;

    function set_uint8
      (object : object_access_t;
       key    : string;
       value  : agar.core.types.uint8_t) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_uint8
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => value);
    end set_uint8;

    function set_natural
      (object : object_access_t;
       key    : string;
       value  : natural) return prop_access_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.set_unsigned_int
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access),
         value  => c.unsigned (value));
    end set_natural;

    -- get

    function get_boolean
      (object : object_access_t;
       key    : string) return boolean
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.get_boolean
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access)) = 1;
    end get_boolean;

    function get_float
      (object : object_access_t;
       key    : string) return float
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return float (cbinds.get_float
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access)));
    end get_float;

    function get_integer
      (object : object_access_t;
       key    : string) return integer
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return integer (cbinds.get_int
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access)));
    end get_integer;

    function get_int16
      (object : object_access_t;
       key    : string) return agar.core.types.int16_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.get_int16
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access));
    end get_int16;

    function get_int32
      (object : object_access_t;
       key    : string) return agar.core.types.int32_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.get_int32
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access));
    end get_int32;

    function get_int64
      (object : object_access_t;
       key    : string) return agar.core.types.int64_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.get_int64
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access));
    end get_int64;

    function get_int8
      (object : object_access_t;
       key    : string) return agar.core.types.int8_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.get_int8
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access));
    end get_int8;

    function get_long_float
      (object : object_access_t;
       key    : string) return long_float
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return long_float (cbinds.get_long_float
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access)));
    end get_long_float;

    function get_long_long_float
      (object : object_access_t;
       key    : string) return long_long_float
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return long_long_float (cbinds.get_long_long_float
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access)));
    end get_long_long_float;

    function get_pointer
      (object : object_access_t;
       key    : string) return agar.core.types.void_ptr_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.get_pointer
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access));
    end get_pointer;

    function get_uint16
      (object : object_access_t;
       key    : string) return agar.core.types.uint16_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.get_uint16
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access));
    end get_uint16;

    function get_uint32
      (object : object_access_t;
       key    : string) return agar.core.types.uint32_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.get_uint32
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access));
    end get_uint32;

    function get_uint64
      (object : object_access_t;
       key    : string) return agar.core.types.uint64_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.get_uint64
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access));
    end get_uint64;

    function get_uint8
      (object : object_access_t;
       key    : string) return agar.core.types.uint8_t
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return cbinds.get_uint8
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access));
    end get_uint8;

    function get_natural
      (object : object_access_t;
       key    : string) return natural
    is
      ca_key : aliased c.char_array := c.to_c (key);
    begin
      return natural (cbinds.get_unsigned_int
        (object => object,
         key    => cs.to_chars_ptr (ca_key'unchecked_access)));
    end get_natural;

  end prop;

end agar.core.object;