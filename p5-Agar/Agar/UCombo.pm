package Agar::UCombo;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::UCombo - a more configurable drop-down list widget

=head1 SYNOPSIS

  use Agar;
  use Agar::UCombo;
  
  Agar::UCombo->new($parent);

=head1 DESCRIPTION

Please see AG_UCombo(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::UCombo>

=head1 METHODS

=over 4

=item B<$widget = Agar::UCombo-E<gt>new($parent, [%options])>

Constructor.

Recognised options include:

=over 4

=item C<scrollToSel>

Z<>

=back

=item B<$widget-E<gt>sizeHint($text,$numItems)>

=item B<$widget-E<gt>sizeHintPixels($w,$h)>

=item B<$list_widget = $widget-E<gt>list()>

=item B<$button_widget = $widget-E<gt>button()>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Button(3)>, L<Agar::Combo(3)>, L<Agar::Tlist(3)>

=cut
