#!/bin/sh

cat <<EOF
-- auto generated, do not edit
with system;

package agar.core.types is

  -- generic void pointer type
  subtype void_ptr_t is system.address;
  null_ptr : constant void_ptr_t := system.null_address;

  -- standard integer set
  type integer_t is new c.int;
  type integer_access_t is access all integer_t;
  pragma convention (c, integer_access_t);

  type unsigned_t is new c.unsigned;
  type unsigned_access_t is access all unsigned_t;
  pragma convention (c, unsigned_access_t);

  type float_t is new c.c_float;
  type float_access_t is access all float_t;
  pragma convention (c, float_access_t);

  type double_t is new c.double;
  type double_access_t is access all double_t;
  pragma convention (c, double_access_t);

  type boolean_t is (false, true);
   for boolean_t use (false => 0, true => 1);
   for boolean_t'size use c.unsigned'size;
  pragma convention (c, boolean_t);

  type boolean_access_t is access all boolean_t;
  pragma convention (c, boolean_access_t);

  type int8_t is range -16#7f# .. 16#7f#;
  type int16_t is range -16#7fff# .. 16#7fff#;
  type int32_t is range -16#7fffffff# .. 16#7fffffff#;
  type int64_t is range -16#7fffffffffffffff# .. 16#7fffffffffffffff#;

  for int8_t'size use 8;
  for int16_t'size use 16;
  for int32_t'size use 32;
  for int64_t'size use 64;

  pragma convention (c, int8_t);
  pragma convention (c, int16_t);
  pragma convention (c, int32_t);
  pragma convention (c, int64_t);

  type int8_ptr_t is access all int8_t;
  type int16_ptr_t is access all int16_t;
  type int32_ptr_t is access all int32_t;
  type int64_ptr_t is access all int64_t;

  pragma convention (c, int8_ptr_t);
  pragma convention (c, int16_ptr_t);
  pragma convention (c, int32_ptr_t);
  pragma convention (c, int64_ptr_t);

  type uint8_t is mod 2 ** 8;
  type uint16_t is mod 2 ** 16;
  type uint32_t is mod 2 ** 32;
  type uint64_t is mod 2 ** 64;

  for uint8_t'size use 8;
  for uint16_t'size use 16;
  for uint32_t'size use 32;
  for uint64_t'size use 64;

  pragma convention (c, uint8_t);
  pragma convention (c, uint16_t);
  pragma convention (c, uint32_t);
  pragma convention (c, uint64_t);

  type uint8_ptr_t is access all uint8_t;
  type uint16_ptr_t is access all uint16_t;
  type uint32_ptr_t is access all uint32_t;
  type uint64_ptr_t is access all uint64_t;

  pragma convention (c, uint8_ptr_t);
  pragma convention (c, uint16_ptr_t);
  pragma convention (c, uint32_ptr_t);
  pragma convention (c, uint64_ptr_t);

EOF

#----------------------------------------------------------------------
# off_t

off_t=`./chk-off_t`
if [ $? -ne 0 ]
then
  echo 'fatal: could not get size of off_t' 1>&2
  exit 112
fi

off_t_size=`echo ${off_t} | awk '{print $1}'`
off_t_min=`echo ${off_t} | awk '{print $2}'`
off_t_max=`echo ${off_t} | awk '{print $3}'`

cat <<EOF
  -- system file offset type
  type off_t is range ${off_t_min} .. ${off_t_max};
   for off_t'size use ${off_t_size};
  pragma convention (c, off_t);

EOF

cat <<EOF
end agar.core.types;
EOF
