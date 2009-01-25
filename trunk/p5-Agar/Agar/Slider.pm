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

Extends Agar::Widget and Agar::Object. Please see AG_Slider(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Slider-E<gt>newHoriz($parent,{flags})>

=item B<$widget = Agar::Slider-E<gt>newVert($parent,{flags})>

Constructors.

=item B<$widget-E<gt>setIncrementInt($step)>

=item B<$widget-E<gt>setIncrementFloat($step)>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

=head1 MAINTAINER

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Slider(3)>

=cut
