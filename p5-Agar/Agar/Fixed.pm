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

Please see AG_Fixed(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Fixed>

=head1 METHODS

=over 4

=item B<$widget = Agar::Fixed-E<gt>new($parent, [%options])>

Constructor.

Recognised options include:

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

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Box(3)>, L<Agar::Pane(3)>, L<Agar::Scrollview(3)>,
L<Agar::Window(3)>

=cut
