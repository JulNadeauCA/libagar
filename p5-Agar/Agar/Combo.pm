package Agar::Combo;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Combo - a drop-down list widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Combo;
  
  Agar::Combo->new($parent);

=head1 DESCRIPTION

Please see AG_Combo(3) for a full explanation of what its methods
do and what bindings and events it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Combo>

=head1 METHODS

=over 4

=item B<$widget = Agar::Combo-E<gt>new($parent, [%options])>

Constructor.

Recognised options include:

=over 4

=item C<poll>

=item C<tree>

=item C<anyText>

=item C<scrollToSel>

Z<>

=back

=item B<$widget-E<gt>sizeHint($text,$numItems)>

=item B<$widget-E<gt>sizeHintPixels($w,$h)>

=item B<$widget-E<gt>select($item)>

=item B<$widget-E<gt>selectText($text)>

=item B<$list_widget = $widget-E<gt>list()>

=item B<$box_widget = $widget-E<gt>tbox()>

=item B<$button_widget = $widget-E<gt>button()>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Button(3)>, L<Agar::Menu(3)>, L<Agar::UCombo(3)>,
L<Agar::Window(3)>

=cut
