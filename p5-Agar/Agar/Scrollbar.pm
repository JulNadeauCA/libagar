package Agar::Scrollbar;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Scrollbar - a scrollbar widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Scrollbar;
  
  Agar::Scrollbar->newVert($parent);

=head1 DESCRIPTION

Please see AG_Scrollbar(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Scrollbar>

=head1 METHODS

=over 4

=item B<$widget = Agar::Scrollbar-E<gt>newHoriz($parent,[%options])>

=item B<$widget = Agar::Scrollbar-E<gt>newVert($parent,[%options])>

Constructors.

=item B<$widget-E<gt>setIncrementInt($step)>

=item B<$widget-E<gt>setIncrementFloat($step)>

=item B<$widget-E<gt>incAction($coderef)>

=item B<$widget-E<gt>decAction($coderef)>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Scrollview(3)>

=cut
