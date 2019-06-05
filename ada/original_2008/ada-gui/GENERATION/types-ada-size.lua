#!/usr/bin/env lua

local io = require ("io")
local string = require ("string")

local argv = arg
local argc = table.maxn (argv)
local progname = "types-ada-size"

local generic_packages = {}
local names_ordered = {}
local names_full = {}
local names_strings = {}
local names_generic = {}
local names_pkg = {}
local names_type = {}

local file_map = nil
local file_gen = nil
local fd_map = nil
local fd_gen = nil

local function fatal (e, s)
  io.stderr:write (progname .. ": fatal: " .. s .. "\n")
  os.exit (e)
end

if argc > 0 then
  file_map = argv[1]
  file_gen = argv[2] 
else
  io.stderr:write (progname .. ": usage: typemap generics\n")
  os.exit (111)
end

fd_map, msg = io.open (file_map)
if not fd_map then fatal (112, "open: " .. msg) end
fd_gen, msg = io.open (file_gen)
if not fd_gen then fatal (112, "open: " .. msg) end

io.write ([[
  -- generic types
  type generic_t is new integer;
  type generic_access_t is access all generic_t;

]])

io.write ("  -- package instantiations\n")
for line in fd_gen:lines () do
  local pkg_name = line
  local gen_name = "gen_" .. string.gsub (pkg_name, "%.", "_")
  io.write ("  package " .. gen_name .. " is new " .. pkg_name ..  "\n")
  io.write ("    (child_type => generic_t, child_access_type => generic_access_t);\n")
  generic_packages[pkg_name] = true
  names_generic [pkg_name] = gen_name
end

io.write ("\n")
io.write ("  -- type names\n")

for line in fd_map:lines () do
  local full_path = string.match (line, "^(.*):")
  local base_type = string.match (full_path, "%.([^%.]*)$")
  local pkg_name  = string.sub (full_path, 1, #full_path - (#base_type + 1))
  local pkg_string = string.gsub (pkg_name, "%.", "_")
  local full_string = string.gsub (full_path, "%.", "_")

  if not names_full[full_path] then
    table.insert (names_ordered, full_path)
    names_full    [full_path] = true
    names_strings [full_path] = full_string
    names_pkg     [full_path] = pkg_name
    names_type    [full_path] = base_type
    io.write ("  "..full_string.." : aliased string := \""..full_path.."\";\n")
  end
end

io.write ([[

  type type_t is record
    name : access string;
    size : natural;
  end record;
  type type_lookup_t is array (natural range <>) of type_t;

  types : aliased constant type_lookup_t := (
]])

for index = 1, table.maxn (names_ordered) do
  local full_path = names_ordered[index]
  local name_pkg = names_pkg[full_path]
  local name_string = names_strings[full_path]

  if generic_packages[name_pkg] then
    local gen_name = names_generic[name_pkg].."."..names_type[full_path]
    io.write ("    ("..name_string.."'access, "..gen_name.."'size)")
  else
    io.write ("    ("..name_string.."'access, "..full_path.."'size)")
  end

  if not (index == table.maxn (names_ordered)) then
    io.write (",\n")
  else
    io.write ("\n")
  end
end
io.write ("  );\n\n")
