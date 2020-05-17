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

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Box(3)>, L<Agar::Fixed(3)>, L<Agar::Pane(3)>,
L<Agar::Scrollbar(3)>, L<Agar::Window(3)>

=cut
