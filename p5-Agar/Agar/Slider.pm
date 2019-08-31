package Agar::Slider;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Slider - a slider widget for setting numerical values

=head1 SYNOPSIS

  use Agar;
  use Agar::Slider;
  
  Agar::Slider->newHoriz($parent);

=head1 DESCRIPTION

Please see AG_Slider(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Slider>

=head1 METHODS

=over 4

=item B<$widget = Agar::Slider-E<gt>newHoriz($parent,[%options])>

=item B<$widget = Agar::Slider-E<gt>newVert($parent,[%options])>

Constructors.

=item B<$widget-E<gt>setIncrementInt($step)>

=item B<$widget-E<gt>setIncrementFloat($step)>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Scrollbar(3)>, L<Agar::Numerical(3)>

=cut
