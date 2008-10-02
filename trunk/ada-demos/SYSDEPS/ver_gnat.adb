-- Detect GNAT version

with ada.exceptions;
with ada.strings.fixed;
with ada.strings.bounded;
with ada.strings;
with ada.text_io;
with gnat.compiler_version;

procedure ver_gnat is
  package ex renames ada.exceptions;
  package io renames ada.text_io;
  package str renames ada.strings;
  package str_fixed renames ada.strings.fixed;
  package version_strings is new ada.strings.bounded.generic_bounded_length (16);
  package ver is new gnat.compiler_version;

  parse_error : exception;
  subtype version_name_t is version_strings.bounded_string;

  procedure version_parse
    (version_raw   : string;
     version_name  : out version_name_t;
     version_major : out integer;
     version_minor : out integer;
     version_patch : out integer)
  is
    version_tmp : string := version_raw;
    fsf_prefix  : constant string := "GNAT Version: ";
    gpl_prefix  : constant string := "GPL ";

    -- find first non numeric character in string
    procedure first_non_numeric
      (str      : string;
       position : out positive;
       found    : out boolean) is
    begin
      for index in str'range loop
        declare
          char : constant character := str (index);
        begin
          if (char < '0') or (char > '9') then
            position := index;
            found := true;
            return;
          end if;
        end;
      end loop;
      found := false;
    end first_non_numeric;

    -- remove GNAT version prefix if necessary
    procedure remove_prefix
      (version : in out string;
       prefix  : string;
       removed : out boolean) is
    begin
      if version'length >= prefix'length then
        if version (version'first .. prefix'length) = prefix then
          str_fixed.delete (version, version'first, prefix'length);
          removed := true;
          return;
        end if;
      end if;
      removed := false;
    end remove_prefix;

    -- expects strings of the form "N.N.N"
    procedure parse_fsf is
      str_tmp : string := version_tmp;
    begin
      -- consume major
      declare
        dot : constant integer := str_fixed.index (str_tmp, ".");
      begin
        version_major := integer'value (str_tmp (str_tmp'first .. dot - 1));
        str_fixed.delete (str_tmp, str_tmp'first, dot);
      end;

      -- consume minor
      declare
        dot : constant integer := str_fixed.index (str_tmp, ".");
      begin
        version_minor := integer'value (str_tmp (str_tmp'first .. dot - 1));
        str_fixed.delete (str_tmp, str_tmp'first, dot);
      end;
 
      -- consume patch
      declare
        found    : boolean;
        position : positive;
      begin
        first_non_numeric (str_tmp, position, found);
        if found then
          version_patch := integer'value (str_tmp (str_tmp'first .. position - 1));
        end if;
      end;
    exception
      when others => raise parse_error with "error parsing FSF version string";
    end parse_fsf;

    -- expects strings of the form "YYYY (YYYYMMDD)" eg. "2008 (20080521)"
    procedure parse_gpl is
      year : constant string :=
        version_tmp (version_tmp'first .. str_fixed.index (version_tmp, " "));
    begin
      version_major := integer'value (year);
    exception
      when others => raise parse_error with "error parsing GPL version string";
    end parse_gpl;

    removed : boolean;
  begin
    version_name  := version_strings.to_bounded_string ("GNAT");
    version_major := 0;
    version_minor := 0;
    version_patch := 0;

    remove_prefix (version_tmp, fsf_prefix, removed);
    if removed then
      version_name := version_strings.to_bounded_string ("GNAT_FSF");
      parse_fsf;
      return;
    end if;

    remove_prefix (version_tmp, gpl_prefix, removed);
    if removed then
      version_name := version_strings.to_bounded_string ("GNAT_GPL");
      parse_gpl;
      return;
    end if;
  end version_parse;

  function to_string (num : integer) return string is
  begin
    return str_fixed.trim (integer'image (num), str.left);
  end to_string;

  -- data
  ver_input : constant string := ver.version;
  ver_name  : version_name_t;
  ver_major : integer;
  ver_minor : integer;
  ver_patch : integer;
begin

  -- catch parse errors
  begin
    version_parse (ver_input, ver_name, ver_major, ver_minor, ver_patch);
  exception
    when e: parse_error =>
      io.put_line (io.current_error, "error: " & ex.exception_message (e));
  end;

  io.put_line
    ("SYSDEP_ADA_TYPE_" & version_strings.to_string (ver_name) & " " &
     to_string (ver_major) & "." &
     to_string (ver_minor) & "." &
     to_string (ver_patch));
end ver_gnat;
