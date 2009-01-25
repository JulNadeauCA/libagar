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
  
  Agar::Font->new("Bitstream Vera.ttf", 12)->setDefault();

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

=head1 MAINTAINER

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar>, L<Agar::Text>

=cut
