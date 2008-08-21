#!/bin/sh

(cat <<EOF
-- auto generated, do not edit
EOF
) || exit 112

cat agar-core-object.ads.txt || exit 112

#----------------------------------------------------------------------
# object_t base

(cat << EOF

  -- Object instance data.
  type object_t is record
    name         : object_name_t;
    archive_path : cs.chars_ptr;
    save_prefix  : cs.chars_ptr;
    class        : class_access_t;
    flags        : c.unsigned;
    nevents      : c.unsigned;
    events       : event_queue.head_t;
    props        : prop_queue.head_t;
    timeout      : timeout_queue.head_t;
    deps         : dep_queue.head_t;
    children     : obj_queue.head_t;
    cobjs        : obj_queue.entry_t;
    tobjs        : obj_queue.entry_t;
    parent       : object_access_t;
    root         : object_access_t;
EOF
) || exit 112

#----------------------------------------------------------------------
# threads

threads=`./chk-threads`
if [ $? -ne 0 ]
then
  echo 'fatal: could not determine if THREADS is defined' 1>&2
  exit 112
fi

if [ "${threads}" = "threads" ]
then
  (cat <<EOF
  -- THREADS defined
  lock           : agar.core.threads.mutex_t;
EOF
) || exit 112
fi

#----------------------------------------------------------------------
# lock debug

lockdebug=`./chk-lockdebug`
if [ $? -ne 0 ]
then
  echo 'fatal: could not determine if LOCKDEBUG is defined' 1>&2
  exit 112
fi

if [ "${lockdebug}" = "lockdebug" ]
then
  (cat <<EOF
  -- LOCKDEBUG defined
  lockinfo       : access cs.chars_ptr;
  nlockinfo      : c.unsigned;
EOF
) || exit 112
fi

#----------------------------------------------------------------------
# debug

debug=`./chk-debug`
if [ $? -ne 0 ]
then
  echo 'fatal: could not determine if DEBUG is defined' 1>&2
  exit 112
fi

if [ "${debug}" = "debug" ]
then
  (cat <<EOF
  -- DEBUG defined
  debug_fn       : access procedure
    (ptr1 : agar.core.types.void_ptr_t;
     ptr2 : agar.core.types.void_ptr_t;
     str  : cs.chars_ptr);
  debug_ptr      : agar.core.types.void_ptr_t;
EOF
) || exit 112
fi

#----------------------------------------------------------------------

(cat <<EOF
  end record;
  pragma convention (c, object_t);

end agar.core.object;
EOF
) || exit 112
