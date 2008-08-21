#!/bin/sh

cat <<EOF
-- auto generated, do not edit
with system;

package agar.core.types is

  -- generic void pointer type
  subtype void_ptr_t is system.address;

  -- standard integer set
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
