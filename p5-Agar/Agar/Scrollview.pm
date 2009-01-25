package Agar::Scrollview;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Scrollview - a Box widget with Scrollbars

=head1 SYNOPSIS

  use Agar;
  use Agar::Scrollview;
  
  Agar::Scrollview->new($parent);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_Scrollview(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Scrollview-E<gt>new($parent, { flags })>

Constructor.

Recognised flags include:

=over 4

=item C<noPanX>

=item C<noPanY>

=item C<noPanXY>

=item C<byMouse>

=item C<frame>

Z<>

=back

=item B<$widget-E<gt>sizeHint($w,$h)>

=item B<$widget-E<gt>setIncrement($pixels)>

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

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Scrollview(3)>

=cut
