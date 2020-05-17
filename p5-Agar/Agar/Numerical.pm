package Agar::Numerical;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Numerical - a spin-button widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Numerical;
  
  Agar::Numerical->new($parent);

=head1 DESCRIPTION

Please see AG_Numerical(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Numerical>

=head1 METHODS

=over 4

=item B<$widget = Agar::Numerical-E<gt>new($parent)>

Constructor.

=item B<$widget-E<gt>setWriteable($on)>

=item B<$widget-E<gt>setRangeInt($min,$max)>

=item B<$widget-E<gt>setRangeFloat($min,$max)>

=item B<$widget-E<gt>setRangeDouble($min,$max)>

=item B<$widget-E<gt>setValue($value)>

=item B<$widget-E<gt>setIncrement($step)>

=item B<$widget-E<gt>setPrecision($format,$prec)>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Button(3)>, L<Agar::Editable(3)>, L<Agar::Textbox(3)>

=cut
