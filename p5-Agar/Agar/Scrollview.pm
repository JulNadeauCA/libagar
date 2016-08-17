package Agar::Scrollview;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Scrollview - a scrollable Box widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Scrollview;
  
  Agar::Scrollview->new($parent);

=head1 DESCRIPTION

Please see AG_Scrollview(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Scrollview>

=head1 METHODS

=over 4

=item B<$widget = Agar::Scrollview-E<gt>new($parent, [%options])>

Constructor.

Recognised options include:

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

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009-2016 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Box(3)>, L<Agar::Fixed(3)>, L<Agar::Pane(3)>,
L<Agar::Scrollbar(3)>, L<Agar::Window(3)>

=cut
