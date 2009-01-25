# Public domain

use strict;
use Test;

BEGIN { plan tests => 4 }

use Agar;
ok(1);
Agar::Version();
ok(2);
Agar::InitCore('test');
ok(3);
Agar::SetError('OK');
ok(Agar::GetError(), 'OK');

