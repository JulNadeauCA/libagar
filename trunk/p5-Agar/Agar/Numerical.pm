package Agar::Numerical;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Numerical - a spin-button widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Numerical;
  
  Agar::Numerical->new($parent);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_Numerical(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Numerical-E<gt>new($parent)>

Constructor.

=item B<$widget-E<gt>setWriteable($on)>

=item B<$widget-E<gt>setRangeInt($min,$max)>

=item B<$widget-E<gt>setRangeFloat($min,$max)>

=item B<$widget-E<gt>setRangeDouble($min,$max)>

=item B<$widget-E<gt>setValue($value)>

=item B<$widget-E<gt>setIncrement($step)>

=item B<$widget-E<gt>setPrecision($format,$prec)>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009-2016 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Numerical(3)>

=cut
