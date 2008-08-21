with agar.core.threads;
with agar.core.types;

package agar.core.datasource is

  type byte_order_t is (
    BYTEORDER_BE,
    BYTEORDER_LE
  );
  for byte_order_t use (
    BYTEORDER_BE => 0,
    BYTEORDER_LE => 1
  );
  for byte_order_t'size use c.unsigned'size;
  pragma convention (c, byte_order_t);

  type seek_mode_t is (
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
  );
  for seek_mode_t use (
    SEEK_SET => 0,
    SEEK_CUR => 1,
    SEEK_END => 2
  );
  for seek_mode_t'size use c.unsigned'size;
  pragma convention (c, seek_mode_t);

  type io_status_t is (
    IO_SUCCESS,
    IO_EOF,
    IO_ERROR,
    IO_UNAVAIL
  );
  for io_status_t use (
    IO_SUCCESS => 0,
    IO_EOF     => 1,
    IO_ERROR   => 2,
    IO_UNAVAIL => 3
  );
  for io_status_t'size use c.unsigned'size;
  pragma convention (c, io_status_t);

  type datasource_t;
  type datasource_access_t is access all datasource_t;

  type read_callback_t is access function
    (data_source : datasource_access_t;
     buf         : agar.core.types.void_ptr_t;
     size        : c.size_t;
     nmemb       : c.size_t;
     rv          : access c.size_t) return io_status_t;
  pragma convention (c, read_callback_t);

  type read_at_callback_t is access function
    (data_source : datasource_access_t;
     buf         : agar.core.types.void_ptr_t;
     size        : c.size_t;
     nmemb       : c.size_t;
     pos         : agar.core.types.off_t;
     rv          : access c.size_t) return io_status_t;  
  pragma convention (c, read_at_callback_t);

  type write_callback_t is access function
    (data_source : datasource_access_t;
     buf         : agar.core.types.void_ptr_t;
     size        : c.size_t;
     nmemb       : c.size_t;
     rv          : access c.size_t) return io_status_t;
  pragma convention (c, write_callback_t);

  type write_at_callback_t is access function
    (data_source : datasource_access_t;
     buf         : agar.core.types.void_ptr_t;
     size        : c.size_t;
     nmemb       : c.size_t;
     pos         : agar.core.types.off_t;
     rv          : access c.size_t) return io_status_t;  
  pragma convention (c, write_at_callback_t);

  type tell_callback_t is access function (data_source : datasource_access_t)
    return agar.core.types.off_t;
  pragma convention (c, tell_callback_t);

  type seek_callback_t is access function
    (data_source : datasource_access_t;
     offs        : agar.core.types.off_t;
     seek_mode   : seek_mode_t) return c.int;
  pragma convention (c, seek_callback_t);

  type close_callback_t is access procedure (data_source : datasource_access_t);
  pragma convention (c, close_callback_t);

  type datasource_t is record
    lock        : agar.core.threads.mutex_t;
    byte_order  : byte_order_t;
    write_last  : c.size_t;
    read_last   : c.size_t;
    write_total : c.size_t;
    read_total  : c.size_t;
    read        : read_callback_t;
    read_at     : read_at_callback_t;
    write       : write_callback_t;
    write_at    : write_at_callback_t;
    tell        : tell_callback_t;
    seek        : seek_callback_t;
    close       : close_callback_t;
  end record;
  pragma convention (c, datasource_t);
  pragma convention (c, datasource_access_t);

  -- API
  function open
    (path : cs.chars_ptr;
     mode : cs.chars_ptr) return datasource_access_t;
  pragma import (c, open, "AG_OpenFile");

  function open
    (path : string;
     mode : string) return datasource_access_t;
  pragma inline (open);
 
  -- missing: AG_OpenFileHandle (need stdio.h binding)

  function open_core
    (ptr  : agar.core.types.void_ptr_t;
     size : c.size_t) return datasource_access_t;
  pragma import (c, open_core, "AG_OpenCore");

  function open_const_core
    (ptr  : agar.core.types.void_ptr_t;
     size : c.size_t) return datasource_access_t;
  pragma import (c, open_const_core, "AG_OpenConstCore");

  procedure close (data_source : datasource_access_t);
  pragma import (c, close, "AG_CloseFile");

  -- missing: AG_CloseFileHandle - macro

  procedure close_core (data_source : datasource_access_t);
  pragma import (c, close_core, "AG_CloseCore");

  -- missing: AG_CloseConstCore - macro

  procedure close_data_source (data_source : datasource_access_t);
  pragma import (c, close_data_source, "AG_CloseDataSource");

  function read
    (data_source : datasource_access_t;
     buf         : agar.core.types.void_ptr_t;
     size        : c.size_t;
     nmemb       : c.size_t;
     rv          : access c.size_t) return io_status_t;
  pragma import (c, read, "AG_Read");

  function read_at
    (data_source : datasource_access_t;
     buf         : agar.core.types.void_ptr_t;
     size        : c.size_t;
     nmemb       : c.size_t;
     pos         : agar.core.types.off_t;
     rv          : access c.size_t) return io_status_t;  
  pragma import (c, read_at, "AG_ReadAt");

  function write
    (data_source : datasource_access_t;
     buf         : agar.core.types.void_ptr_t;
     size        : c.size_t;
     nmemb       : c.size_t;
     rv          : access c.size_t) return io_status_t;
  pragma import (c, write, "AG_Write");

  function write_at
    (data_source : datasource_access_t;
     buf         : agar.core.types.void_ptr_t;
     size        : c.size_t;
     nmemb       : c.size_t;
     pos         : agar.core.types.off_t;
     rv          : access c.size_t) return io_status_t;  
  pragma import (c, write_at, "AG_WriteAt");
 
  function tell (data_source : datasource_access_t) return agar.core.types.off_t;
  pragma import (c, tell, "AG_Tell");

  function seek (data_source : datasource_access_t) return c.int;
  pragma import (c, seek, "AG_Seek");

  -- missing: AG_LockDataSource - macro
  -- missing: AG_UnlockDataSource - macro

  procedure set_byte_order
    (data_source : datasource_access_t;
     order       : byte_order_t);
  pragma import (c, set_byte_order, "AG_SetByteOrder");

  procedure init (data_source : datasource_access_t);
  pragma import (c, init, "AG_DataSourceInit");

  procedure destroy (data_source : datasource_access_t);
  pragma import (c, destroy, "AG_DataSourceDestroy");

end agar.core.datasource;
