package Agar::ProgressBar;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::ProgressBar - a progress bar widget

=head1 SYNOPSIS

  use Agar;
  use Agar::ProgressBar;
  
  Agar::ProgressBar->newHoriz($parent);

=head1 DESCRIPTION

Please see AG_ProgressBar(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::ProgressBar>

=head1 METHODS

=over 4

=item B<$widget = Agar::ProgressBar-E<gt>newHoriz($parent,[%options])>

=item B<$widget = Agar::ProgressBar-E<gt>newVert($parent,[%options])>

Constructors.

Recognised options include:

=over 4

=item C<showPercent>

Display a text label with the percentage.

Z<>

=back

=item B<$widget-E<gt>setWidth($pixels)>

=item B<$pct = $widget-E<gt>getPercent()>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>

=cut
