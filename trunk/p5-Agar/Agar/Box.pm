package Agar::Box;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Box - a container for stacking widgets

=head1 SYNOPSIS

  use Agar;
  use Agar::Box;
  
  Agar::Box->newVert($parent);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_Box(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Box-E<gt>newHoriz($parent,{flags})>

=item B<$widget = Agar::Box-E<gt>newVert($parent,{flags})>

Constructors.

Recognised flags include:

=over 4

=item C<homogenous>

=item C<frame>

Z<>

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

=head1 MAINTAINER

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Box(3)>

=cut
