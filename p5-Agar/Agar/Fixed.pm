package Agar::Fixed;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Fixed - a container holding widgets in fixed positions

=head1 SYNOPSIS

  use Agar;
  use Agar::Fixed;
  
  Agar::Fixed->new($parent);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_Fixed(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Fixed-E<gt>new($parent, { flags })>

Constructor.

Recognised flags include:

=over 4

=item C<fillBG>

=item C<box>

=item C<frame>

=item C<noUpdate>

Z<>

=back

=item B<$widget-E<gt>size($child,$w,$h)>

=item B<$widget-E<gt>move($child,$x,$y)>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

=head1 MAINTAINER

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Fixed(3)>

=cut
