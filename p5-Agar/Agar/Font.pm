package Agar::Font;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Font - encapsulate Agar's conception of a font

=head1 SYNOPSIS

  use Agar;
  use Agar::Font;
  
  Agar::Font->new("Courier", 12)->setDefault();

=head1 DESCRIPTION

This class enables the loading and using of different fonts.

=head1 METHODS

=over 4

=item B<$font = Agar::Font-E<gt>new($path, $pointsize)>

Create a new font object from the given file (TrueType or pixmap) and point
size.

=item B<$font-E<gt>setDefault()>

Cause the font to be used as the default for all text rendering.

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Editable(3)>, L<Agar::Label(3)>, L<Agar::Text(3)>,
L<Agar::Textbox(3)>

=cut
