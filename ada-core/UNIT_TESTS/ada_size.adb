with ada.text_io;
with ada.command_line;

with agar.core.datasource;
with agar.core.dso;
with agar.core.event;
with agar.core.object;
with agar.core.threads;
with agar.core.timeout;
with agar.core.types;
with agar.core.version;
with agar.core;
with agar;

procedure ada_size is
  package io renames ada.text_io;
  package cmdline renames ada.command_line;

  -- auto generated - do not edit
  agar_core_datasource_byte_order_t : aliased string := "agar.core.datasource.byte_order_t";
  agar_core_datasource_datasource_t : aliased string := "agar.core.datasource.datasource_t";
  agar_core_datasource_io_status_t : aliased string := "agar.core.datasource.io_status_t";
  agar_core_datasource_seek_mode_t : aliased string := "agar.core.datasource.seek_mode_t";
  agar_core_dso_dso_access_t : aliased string := "agar.core.dso.dso_access_t";
  agar_core_dso_dso_t : aliased string := "agar.core.dso.dso_t";
  agar_core_dso_sym_access_t : aliased string := "agar.core.dso.sym_access_t";
  agar_core_dso_sym_t : aliased string := "agar.core.dso.sym_t";
  agar_core_event_arg_t : aliased string := "agar.core.event.arg_t";
  agar_core_event_arg_type_t : aliased string := "agar.core.event.arg_type_t";
  agar_core_event_event_t : aliased string := "agar.core.event.event_t";
  agar_core_object_checksum_alg_t : aliased string := "agar.core.object.checksum_alg_t";
  agar_core_object_class_t : aliased string := "agar.core.object.class_t";
  agar_core_object_dep_t : aliased string := "agar.core.object.dep_t";
  agar_core_object_object_t : aliased string := "agar.core.object.object_t";
  agar_core_object_prop_class_t : aliased string := "agar.core.object.prop_class_t";
  agar_core_object_prop_type_t : aliased string := "agar.core.object.prop_type_t";
  agar_core_threads_cond_t : aliased string := "agar.core.threads.cond_t";
  agar_core_threads_key_t : aliased string := "agar.core.threads.key_t";
  agar_core_threads_mutex_attr_t : aliased string := "agar.core.threads.mutex_attr_t";
  agar_core_threads_mutex_t : aliased string := "agar.core.threads.mutex_t";
  agar_core_threads_thread_t : aliased string := "agar.core.threads.thread_t";
  agar_core_timeout_timeout_t : aliased string := "agar.core.timeout.timeout_t";
  agar_core_types_int16_t : aliased string := "agar.core.types.int16_t";
  agar_core_types_int32_t : aliased string := "agar.core.types.int32_t";
  agar_core_types_int64_t : aliased string := "agar.core.types.int64_t";
  agar_core_types_int8_t : aliased string := "agar.core.types.int8_t";
  agar_core_types_uint16_t : aliased string := "agar.core.types.uint16_t";
  agar_core_types_uint32_t : aliased string := "agar.core.types.uint32_t";
  agar_core_types_uint64_t : aliased string := "agar.core.types.uint64_t";
  agar_core_types_uint8_t : aliased string := "agar.core.types.uint8_t";
  agar_core_version_version_t : aliased string := "agar.core.version.version_t";
  agar_core_version_t : aliased string := "agar.core.version_t";

  type type_t is record
    name : access string;
    size : natural;
  end record;
  type type_lookup_t is array (natural range <>) of type_t;

  types : aliased constant type_lookup_t := (
    (agar_core_datasource_byte_order_t'access, agar.core.datasource.byte_order_t'size),
    (agar_core_datasource_datasource_t'access, agar.core.datasource.datasource_t'size),
    (agar_core_datasource_io_status_t'access, agar.core.datasource.io_status_t'size),
    (agar_core_datasource_seek_mode_t'access, agar.core.datasource.seek_mode_t'size),
    (agar_core_dso_dso_access_t'access, agar.core.dso.dso_access_t'size),
    (agar_core_dso_dso_t'access, agar.core.dso.dso_t'size),
    (agar_core_dso_sym_access_t'access, agar.core.dso.sym_access_t'size),
    (agar_core_dso_sym_t'access, agar.core.dso.sym_t'size),
    (agar_core_event_arg_t'access, agar.core.event.arg_t'size),
    (agar_core_event_arg_type_t'access, agar.core.event.arg_type_t'size),
    (agar_core_event_event_t'access, agar.core.event.event_t'size),
    (agar_core_object_checksum_alg_t'access, agar.core.object.checksum_alg_t'size),
    (agar_core_object_class_t'access, agar.core.object.class_t'size),
    (agar_core_object_dep_t'access, agar.core.object.dep_t'size),
    (agar_core_object_object_t'access, agar.core.object.object_t'size),
    (agar_core_object_prop_class_t'access, agar.core.object.prop_class_t'size),
    (agar_core_object_prop_type_t'access, agar.core.object.prop_type_t'size),
    (agar_core_threads_cond_t'access, agar.core.threads.cond_t'size),
    (agar_core_threads_key_t'access, agar.core.threads.key_t'size),
    (agar_core_threads_mutex_attr_t'access, agar.core.threads.mutex_attr_t'size),
    (agar_core_threads_mutex_t'access, agar.core.threads.mutex_t'size),
    (agar_core_threads_thread_t'access, agar.core.threads.thread_t'size),
    (agar_core_timeout_timeout_t'access, agar.core.timeout.timeout_t'size),
    (agar_core_types_int16_t'access, agar.core.types.int16_t'size),
    (agar_core_types_int32_t'access, agar.core.types.int32_t'size),
    (agar_core_types_int64_t'access, agar.core.types.int64_t'size),
    (agar_core_types_int8_t'access, agar.core.types.int8_t'size),
    (agar_core_types_uint16_t'access, agar.core.types.uint16_t'size),
    (agar_core_types_uint32_t'access, agar.core.types.uint32_t'size),
    (agar_core_types_uint64_t'access, agar.core.types.uint64_t'size),
    (agar_core_types_uint8_t'access, agar.core.types.uint8_t'size),
    (agar_core_version_version_t'access, agar.core.version.version_t'size),
    (agar_core_version_t'access, agar.core.version_t'size)
  );

  procedure find (name : string) is
  begin
    for index in types'range loop
      if types (index).name.all = name then
        io.put_line (natural'image (types (index).size));
        return;
      end if;
    end loop;
    raise program_error with "fatal: unknown ada type";
  end find;

begin
  if cmdline.argument_count /= 1 then
    raise program_error with "fatal: incorrect number of args";
  end if;
  find (cmdline.argument (1));
end ada_size;
