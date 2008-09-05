package agar.core.file is

  type type_t is (
    FILE_REGULAR,
    FILE_DIRECTORY,
    FILE_DEVICE,
    FILE_FIFO,
    FILE_SYMLINK,
    FILE_SOCKET
  );
  for type_t use (
    FILE_REGULAR   => 0,
    FILE_DIRECTORY => 1,
    FILE_DEVICE    => 2,
    FILE_FIFO      => 3,
    FILE_SYMLINK   => 4,
    FILE_SOCKET    => 5
  );
  for type_t'size use c.unsigned'size;
  pragma convention (c, type_t);

  subtype perms_t is c.unsigned;
  FILE_READABLE   : constant perms_t := 16#01#;
  FILE_WRITEABLE  : constant perms_t := 16#02#;
  FILE_EXECUTABLE : constant perms_t := 16#04#;

  subtype flags_t is c.unsigned;
  FILE_SUID       : constant flags_t := 16#001#;
  FILE_SGID       : constant flags_t := 16#002#;
  FILE_ARCHIVE    : constant flags_t := 16#004#;
  FILE_COMPRESSED : constant flags_t := 16#008#;
  FILE_ENCRYPTED  : constant flags_t := 16#010#;
  FILE_HIDDEN     : constant flags_t := 16#020#;
  FILE_REPARSE_PT : constant flags_t := 16#040#;
  FILE_SPARSE     : constant flags_t := 16#080#;
  FILE_TEMPORARY  : constant flags_t := 16#100#;
  FILE_SYSTEM     : constant flags_t := 16#200#;

  type info_t is record
    file_type : type_t;
    perms     : perms_t;
    flags     : flags_t;
  end record;
  type info_access_t is access all info_t;
  pragma convention (c, info_t);
  pragma convention (c, info_access_t);

  -- API

  function get_info
    (name : string;
     info : info_access_t) return boolean;
  pragma inline (get_info);

  function get_system_temp_dir return string;
  pragma inline (get_system_temp_dir);

  function exists (name : string) return boolean;
  pragma inline (exists);

  function delete (name : string) return boolean;
  pragma inline (delete);

end agar.core.file;
