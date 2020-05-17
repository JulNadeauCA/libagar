package Agar::PixelFormat;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::PixelFormat - Pixel encoding information

=head1 SYNOPSIS

  use Agar;
  use Agar::PixelFormat;

  my $pfRGB = Agar::PixelFormat->newRGB(32,
      0xff000000, 0x00ff0000, 0x0000ff00);
  my $pfRGBA = Agar::PixelFormat->newRGBA(32,
      0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
  my $pfIndexed = Agar::PixelFormat->newIndexed(8);
  my $pfStandard = Agar::PixelFormat->newStandard();
  my $redPixel = $pf->mapRGB(255, 0, 0);
  my $transparentPixel = $pf->mapRGBA(255, 0, 0, 127);
  printf "Hex = %s", $pf->getRGB($pixel);

=head1 DESCRIPTION

B<Agar::PixelFormat> encodes and decodes individual pixels according to a
memory representation of a specified depth (in bits per pixel).
In the case of I<packed pixel> formats, a set of masks define how individual
8-bit red, green, blue and alpha components can be retrieved from the memory
representation of a pixel. For I<indexed> formats, pixels are represented as
indices into a color palette contained in the B<Agar::PixelFormat> object.

=head1 METHODS

=over 4

=item B<$format = Agar::PixelFormat-E<gt>newRGB($depth,$Rmask,$Gmask,$Bmask)>

Create a new format object representing a packed-pixel format of the
specified depth (given in bits per pixel). $[RGB]mask specify the masks
used to retrieve the individual components of a pixel from memory.

=item B<$format = Agar::PixelFormat-E<gt>newRGBA($depth,$Rmask,$Gmask,$Bmask, $Amask)>

Same as B<newRGB>, except that the pixel packing includes an alpha
component.

=item B<$format = Agar::PixelFormat-E<gt>newIndexed($depth)>

Create a new format object representing a color-index format. $depth is
given in bits per pixel and the size of the palette is determined by it.

=item B<$format = Agar::PixelFormat-E<gt>newStandard>

Duplicate the standard, general-purpose Agar surface format which is
determined on initialization. Usually this format is packed-pixel with an
alpha channel.

=item B<$pixel = $pf-E<gt>mapRGB($red,$green,$blue)>

Return a 32-bit value containing the memory representation of a pixel of
the given color. If I<$pf> specifies an alpha-channel, it is set to 255
(opaque).

=item B<$pixel = $pf-E<gt>mapRGBA($red,$green,$blue,$alpha)>

Return a 32-bit value containing the memory representation of a pixel of
the given color and alpha value.

=item B<@components = $pf-E<gt>getRGB($pixel)>

Given a 32-bit value containing the memory representation of a pixel,
return a 3-element array containing the component values. In scalar
context, the hexadecimal representation of the color (#RRGGBB) is
returned.

=item B<@components = $pf-E<gt>getRGBA($pixel)>

Given a 32-bit value containing the memory representation of a pixel,
return a 4-element array containing the component values. In scalar
context, the hexadecimal representation of the color (#RRGGBBAA)
is returned.

=back

=head1 AUTHOR

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Surface(3)>

=cut
