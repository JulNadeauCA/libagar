# Public domain

use strict;
use Test;

BEGIN { plan tests => 3 }

use Agar;
ok(1);
Agar::Version();
ok(2);
Agar::SetError('OK');
ok(Agar::GetError(), 'OK');

