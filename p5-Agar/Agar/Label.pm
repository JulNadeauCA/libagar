package Agar::Label;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Label - a simple text label widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Label;
  
  Agar::Label->new($parent, $text);

=head1 DESCRIPTION

Please see AG_Label(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Label>

=head1 METHODS

=over 4

=item B<$widget = Agar::Label-E<gt>new($parent, [%options])>

Constructor.

Recognised options include:

=over 4

=item C<frame>

=item C<noMinSize>

=item C<partial>

=item C<regen>

Z<>

=back

=item B<$widget-E<gt>setPadding($left,$right,$top,$bottom)>

=item B<$widget-E<gt>justify($mode)>

=item B<$widget-E<gt>vAlign($mode)>

=item B<$widget-E<gt>sizeHint($numLines,$text)>

=item B<$widget-E<gt>setText($text)>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Button(3)>, L<Agar::Checkbox(3)>, L<Agar::Editable(3)>,
L<Agar::Textbox(3)>

=cut
