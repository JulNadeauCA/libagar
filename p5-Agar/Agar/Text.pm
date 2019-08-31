package Agar::Text;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Text - text rendering functions for Agar widgets

=head1 SYNOPSIS

  use Agar;
  use Agar::Text;

=head1 DESCRIPTION

These methods are useful when implementing Agar widgets which need to
render text. They must be called from the rendering context (draw
routine) of an L<Agar::Widget(3)>.

=head1 METHODS

=item B<Agar::Text::PushState()>

Push the current text rendering state onto the text rendering state stack.

Useful if you want to change the state temporarily, you can pop the stack
again once you've finished.

=item B<Agar::Text::PopState()>

Pop the text rendering state to revert to the text rendering state previously
pushed onto the stack.

=item B<Agar::Text::Justify($mode)>

Set the text justification mode. This may be "left", "right" or "center".

=item B<Agar::Text::Valign($mode)>

Set the vertical alignment mode. This may be "top", "middle" or "bottom".

=item B<Agar::Text::ColorRGB($r, $g, $b)>

Set the color for rendered text from red, green and blue 8-bit values.

=item B<Agar::Text::ColorRGBA($r, $g, $b, $a)>

Set the color for rendered text from red, green, blue and alpha
(transparency) 8-bit values.

=item B<Agar::Text::BGColorRGB($r, $g, $b)>

Set the background color for rendered text from red, green and blue 8-bit
values.

=item B<Agar::Text::BGColorRGBA($r, $g, $b, $a)>

Set the background color for rendered text from red, green, blue and alpha
(transparency) 8-bit values.

=item B<Agar::Text::SetFont($font)>

Set the font for the current text rendering state.

=item B<$w = Agar::Text::Width($text)>

Returns the width in pixels of the given text if it were to be rendered
using the current font.

=item B<$h = Agar::Text::Height($text)>

Returns the height in pixels of the given text if it were to be rendered
using the current font.

=over 4

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Font(3)>, L<Agar::Widget(3)>

=cut
