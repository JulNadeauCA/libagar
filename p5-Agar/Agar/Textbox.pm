package Agar::Textbox;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Textbox - a text input widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Textbox;
  
  Agar::Textbox->new($parent);

=head1 DESCRIPTION

The AG_Textbox(3) widget implements single- or multi-line text edition.

It provides a "string" binding which can be connected to a "C" string in a fixed-size buffer.
It also provides a "text" binding which can be connected to a multilingual AG_TextElement(3).

Technically AG_Textbox(3) is a simple container widget which embeds an AG_Editable(3) field
with optional text labels and scrollbars. The core edition functionality is implemented in
AG_Editable(3).

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Textbox>

=head1 METHODS

=over 4

=item B<$textbox = Agar::Textbox-E<gt>new($parent, $label, [%options])>

Allocate, initialize and attach a new Textbox.
$label specifies a static text label to display next to the AG_Editable(3) field.
Recognised options include:

=over 4

=item C<multiLine>

Multi-line mode. Newlines are entered literally. Horizontal and vertical scrollbars are created.

=item C<wordWrap>

Enable word wrapping in multiline mode.

=item C<password>

Password-style entry where characters are hidden.

=item C<abandonFocus>

Arrange for the widget to abandon its focus when the return key is pressed.

=item C<readOnly>

Make the string read-only.
This has the same effect as disabling the widget generically via Agar::Widget(3),
except that the textbox is not grayed out.

=item C<intOnly>

Restricts input to valid integers only.

=item C<floatOnly>

Restricts input to valid floating-point numbers in decimal and scientific
notation ("inf" and the Unicode symbol for Infinity may also be used).

=item C<catchTab>

Cause tabs to be entered literally into the string (by default, the tab
key moves focus to the next widget).

=item C<excl>

By default, external changes to the contents of the buffer are allowed and
handled in a safe manner (at some cost). C<excl> specifies that this textbox
will access the buffer in an exclusive manner. This allows important optimizations.

=item C<noEmacs>

Disable emacs-style function keys.

=item C<noLatin1>

Disable traditional LATIN-1 key combinations.

=item C<noPopup>

Disable the standard right-click popup menu.

=item C<multilingual>

Allow the user to select different languages from the right-click popup
menu (provided the widget is bound to an AG_TextElement(3).
Z<>

=back

=item B<$textbox-E<gt>sizeHint($text)>

Request that an initial geometry sufficient to hold the string $text in its entirety.

=item B<$textbox-E<gt>sizeHintPixels($w,$h)>

Request an explicit initial geometry of $w x $h pixels.

=item B<$textbox-E<gt>sizeHintLines($numLines)>

Request an that initial geometry sufficient to contain $numLines lines vertically.

=item B<$pos = $textbox-E<gt>getCursorPos()>

Return the current position of the cursor in the buffer.

=item B<$textbox-E<gt>setCursorPos($pos)>

Moves the cursor to $pos (if bounds checking allow it).

=item B<$bool = $textbox-E<gt>getFlag($option)>

Return the current state of a named $option.

=item B<$textbox-E<gt>setFlag($option)>

Enable the named $option.

=item B<$textbox-E<gt>unsetFlag($option)>

Disable the named $option.

=item B<$text = $textbox-E<gt>getString()>

Return the contents of the textbox as text.

=item B<$textbox-E<gt>setString($text)>

Set the contents to $text.

=item B<$textbox-E<gt>clearString($text)>

Clear the contents of the textbox.

=item B<$integer = $textbox-E<gt>getInt()>

Return an integer representation of the contents of the textbox.

=item B<$float = $textbox-E<gt>getFloat()>

Return a floating-point representation of the contents of the textbox.

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Editable(3)>, L<Agar::Scrollbar(3)>

=cut
