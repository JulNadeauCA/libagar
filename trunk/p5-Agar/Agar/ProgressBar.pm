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

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009-2016 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar(3)>

=cut
