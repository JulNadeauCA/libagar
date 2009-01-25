package Agar::Colors;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Colors - customize the Agar color scheme

=head1 SYNOPSIS

  use Agar;
  use Agar::Colors;

=head1 DESCRIPTION

Agar defines a table of colors that are use when drawing different GUI
elements. This class enables the changing of the colors in this table.

  "Background"
  "Frame"
  "Line"
  "Text"
  "Window background"
  "Window highlight"
  "Window lowlight"
  "Titlebar (focused)"
  "Titlebar (unfocused)"
  "Titlebar caption"
  "Button"
  "Button text"
  "Disabled widget"
  "Checkbox"
  "Checkbox text"
  "Graph"
  "Graph X-axis"
  "HSV palette circle"
  "HSV palette tiling 1"
  "HSV palette tiling 2"
  "Menu"
  "Menu selection"
  "Menu option"
  "Menu text"
  "Menu separator 1"
  "Menu separator 2"
  "Notebook"
  "Notebook selection"
  "Notebook tab text"
  "Radio selection"
  "Radio overlap"
  "Radio highlight"
  "Radio lowlight"
  "Radio text"
  "Scrollbar"
  "Scrollbar buttons"
  "Scrollbar arrow 1"
  "Scrollbar arrow 2"
  "Separator line 1"
  "Separator line 2"
  "Tableview"
  "Tableview header"
  "Tableview header text"
  "Tableview cell text"
  "Tableview line"
  "Tableview selection"
  "Textbox"
  "Textbox text"
  "Textbox cursor"
  "Tlist text"
  "Tlist"
  "Tlist line"
  "Tlist selection"
  "Mapview grid"
  "Mapview cursor"
  "Mapview tiling 1"
  "Mapview tiling 2"
  "Mapview mouse selection"
  "Mapview effective selection"
  "Tileview tiling 1"
  "Tileview tiling 2"
  "Tileview text background"
  "Tileview text"
  "Transparent color"
  "HSV Palette bar #1"
  "HSV Palette bar #2"
  "Pane"
  "Pane (circles)"
  "Mapview noderef selection"
  "Mapview origin point"
  "Focus"
  "Table"
  "Table lines"
  "Fixed background"
  "Fixed box"
  "Text (disabled)"
  "Menu text (disabled)"
  "Socket"
  "Socket label"
  "Socket highlight"
  "Progress bar"
  "Window border"

=head1 METHODS

=over 4

=item B<Agar::Colors::Reset()>

Revert to the default color scheme.

=item B<Agar::Colors::Load($path)>

Loads the color scheme from the given Agar color scheme (acs) file.

=item B<Agar::Colors::Save($path)>

Saves the color scheme to the given Agar color scheme (acs) file.

=item B<Agar::Colors::SaveDefault()>

Saves the color scheme to the default location.

=item B<Agar::Colors::SetRGB($name, $r, $g, $b)>

Sets the named color using the given red, green and blue 8-bit values.

Returns 0 on success, -1 if the name was not recognised.

=item B<Agar::Colors::SetRGBA($name, $r, $g, $b, $a)>

Sets the named color using the given red, green, blue and alpha
(transparency) 8-bit values.

Returns 0 on success, -1 if the name was not recognised.

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

L<Agar>

=cut
