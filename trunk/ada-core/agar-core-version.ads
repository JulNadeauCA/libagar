with agar.core.datasource;
with agar.core.types;

package agar.core.version is

  use type c.unsigned;
  version_name_max : constant c.unsigned := 48;
  version_max      : constant c.unsigned := version_name_max + 8;

  type version_t is record
    major : agar.core.types.uint32_t;
    minor : agar.core.types.uint32_t;
  end record;
  type version_access_t is access all version_t;
  pragma convention (c, version_t);
  pragma convention (c, version_access_t);

  function read_version
    (ds    : agar.core.datasource.datasource_access_t;
     magic : cs.chars_ptr;
     ver   : version_access_t;
     rver  : version_access_t) return c.int;
  pragma import (c, read_version, "AG_ReadVersion");

  procedure write_version
    (ds    : agar.core.datasource.datasource_access_t;
     magic : cs.chars_ptr;
     ver   : version_access_t);
  pragma import (c, write_version, "AG_WriteVersion");

  function read_object_version
    (ds  : agar.core.datasource.datasource_access_t;
     obj : agar.core.types.void_ptr_t;
     ver : version_access_t) return c.int;
  pragma import (c, read_object_version, "AG_ReadObjectVersion");

  procedure write_object_version
    (ds  : agar.core.datasource.datasource_access_t;
     obj : agar.core.types.void_ptr_t);
  pragma import (c, write_object_version, "AG_WriteObjectVersion");

end agar.core.version;
