package Agar::Surface;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Surface - Graphical surface object

=head1 SYNOPSIS

  use Agar;
  use Agar::Surface;
  use Agar::PixelFormat;

  my $sPacked = Agar::Surface->new(10,10,
      Agar::PixelFormat->newStandard(),
      [%options]);
  my $sIndexed = Agar::Surface->newIndexed(10,10, 32);
  my $sEmpty = Agar::Surface->newEmpty();
  my $sImage = Agar::Surface->newFromPNG('foo.png');
  my $sPhoto = Agar::Surface->newFromJPEG('foo.jpg');

=head1 DESCRIPTION

B<Agar::Surface> represents a graphical surface which supports packed-pixel
formats (allowing components to be retrieved by bitmasks), as well as
I<indexed> (or I<palletized>) pixel formats.

Pixel formats are represented by the L<Agar::PixelFormat> object.

=head1 INHERITANCE HIERARCHY

B<Agar::Surface>

=head1 METHODS

=over 4

=item B<$surface = Agar::Surface-E<gt>new($width,$height,$fmt,\%options)>

Create a new surface of the given dimensions in pixels. $width and $height
are the dimensions of the surface in pixels. $fmt is an L<Agar::PixelFormat>
object describing the way pixels are encoded in memory. Available %options
include:

=over 4

=item C<hwSurface>

Advise that the surface should be created in video memory whenever possible.
This optimizes the performance of hardware<->hardware surface operations at
the expense of hardware<->software performance.

=item C<srcColorKey>

Enable colorkeying whenever this surface is used as a source in a blit
operation. Pixels matching the colorkey value associated with the surface
will be treated as transparent.

=item C<srcAlpha>

Enable alpha blending whenever this surface is used as a source in a blit
operation. The alpha component stored in individual pixels will be used as
a weight factor for the source pixel.

=back

=item B<$surface = Agar::Surface-E<gt>newIndexed($width,$height,$bpp)>

Create a new surface using color-index encoding. The size of the palette
is determined by $bpp, which is given in bits per pixel.

=item B<$surface = Agar::Surface-E<gt>newEmpty>

Create a new, empty surface.

=item B<$surface = Agar::Surface-E<gt>newFromBMP($path)>

Create a new surface from the given bitmap file. Returns undef on failure.

=item B<$surface = Agar::Surface-E<gt>newFromPNG($path)>

Create a new surface from the given PNG file. Returns undef on failure.

=item B<$surface = Agar::Surface-E<gt>newFromJPEG($path)>

Create a new surface from the given JPEG file. Returns undef on failure.

=back

=head1 AUTHOR

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::PixelFormat(3)>, L<Agar::Pixmap(3)>

=cut
