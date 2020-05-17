package Agar::Box;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Box - general purpose widget container

=head1 SYNOPSIS

  use Agar;
  use Agar::Box;
  
  Agar::Box->newVert($parent);

=head1 DESCRIPTION

A general purpose widget container which packs its children
horizontally or vertically.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Box>

=head1 METHODS

=over 4

=item B<$widget = Agar::Box-E<gt>newHoriz($parent,[%options])>

Create a new horizontally-aligned box.

=item B<$widget = Agar::Box-E<gt>newVert($parent,[%options])>

Create a new vertically-aligned box.

Available options include:

=over 4

=item C<homogenous>

Force space to be divided in equal parts.

=item C<frame>

Render a decorative well / frame around the widgets.

=back

=item B<$widget-E<gt>setLabel($text)>

=item B<$widget-E<gt>setHomogenous($flag)>

=item B<$widget-E<gt>setPadding($padding)>

=item B<$widget-E<gt>setSpacing($spacing)>

=item B<$widget-E<gt>setHoriz()>

=item B<$widget-E<gt>setVert()>

=item B<$widget-E<gt>setDepth($depth)>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Fixed(3)>, L<Agar::Pane(3)>, L<Agar::Notebook(3)>,
L<Agar::Scrollview(3)>, L<Agar::Window(3)>

=cut
