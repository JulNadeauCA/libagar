package Agar::Button;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Button - a clickable button widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Button;
  
  Agar::Button->new($parent);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_Button(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Button-E<gt>new($parent, { flags })>

Constructor.

Recognised flags include:

=over 4

=item C<sticky>

=item C<mouseOver>

=item C<repeat>

=item C<invertState>

Z<>

=back

=item B<$widget-E<gt>setPadding($left,$right,$top,$bottom)>

=item B<$widget-E<gt>justify($mode)>

=item B<$widget-E<gt>vAlign($mode)>

=item B<$widget-E<gt>setRepeatMode($on)>

=item B<$widget-E<gt>surface($surface)>

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

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Button(3)>

=cut
