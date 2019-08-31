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

Please see AG_Textbox(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Textbox>

=head1 METHODS

=over 4

=item B<$widget = Agar::Textbox-E<gt>new($parent, [%options])>

Constructor.

Recognised options include:

=over 4

=item C<multiLine>

=item C<static>

=item C<password>

=item C<abandonFocus>

=item C<intOnly>

=item C<floatOnly>

=item C<catchTab>

=item C<noEmacs>

=item C<noWordSeek>

=item C<noLatin1>

Z<>

=back

=item B<$widget-E<gt>sizeHint($text)>

=item B<$widget-E<gt>sizeHintPixels($w,$h)>

=item B<$widget-E<gt>sizeHintLines($numLines)>

=item B<$pos = $widget-E<gt>getCursorPos()>

=item B<$widget-E<gt>setCursorPos($pos)>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Editable(3)>, L<Agar::Scrollbar(3)>

=cut
