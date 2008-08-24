#!/bin/sh

(cat <<EOF
-- auto generated, do not edit

package agar.core.limits is

`./chk-limits`

end agar.core.limits;
EOF
) || exit 112
