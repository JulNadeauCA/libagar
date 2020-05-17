package Agar::Pixmap;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Pixmap - a widget for displaying an Agar::Surface

=head1 SYNOPSIS

  use Agar;
  use Agar::Pixmap;
  
  Agar::Pixmap->new($parent);

=head1 DESCRIPTION

Please see AG_Pixmap(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Pixmap>

=head1 METHODS

=over 4

=item B<$widget = Agar::Pixmap-E<gt>new($parent,$surface)>

=item B<$widget = Agar::Pixmap-E<gt>newScaled($parent,$surface,$w,$h)>

Constructors.

=item B<$widget-E<gt>setSourceCoords($x,$y)>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::PixelFormat(3)>, L<Agar::Surface(3)>

=cut
