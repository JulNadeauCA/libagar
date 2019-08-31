package Agar::Toolbar;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Toolbar - a widget containing row(s) of Buttons

=head1 SYNOPSIS

  use Agar;
  use Agar::Toolbar;
  
  Agar::Toolbar->newHoriz($parent, 1);

=head1 DESCRIPTION

Please see AG_Toolbar(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Toolbar>

=head1 METHODS

=over 4

=item B<$widget = Agar::Toolbar-E<gt>newHoriz($parent,$numRows,[%options])>

=item B<$widget = Agar::Toolbar-E<gt>newVert($parent,$numRows,[%options])>

Constructors.

Recognised options include:

=over 4

=item C<homogenous>

=item C<sticky>

=item C<multiSticky>

Z<>

=back

=item B<$widget-E<gt>setActiveRow($row)>

=item B<$button_widget = $widget-E<gt>addTextButton($text)>

=item B<$button_widget = $widget-E<gt>addIconButton($surface)>

=item B<$widget-E<gt>select($button)>

=item B<$widget-E<gt>deselect($button)>

=item B<$widget-E<gt>selectOnly($button)>

=item B<$widget-E<gt>selectAll()>

=item B<$widget-E<gt>deselectAll()>

=item B<$row = $widget-E<gt>getRow($index)>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Button(3)>, L<Agar::Menu(3)>

=cut
