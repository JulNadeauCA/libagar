package Agar::Label;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Label - a simple text label widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Label;
  
  Agar::Label->new($parent);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_Label(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Label-E<gt>new($parent, { flags })>

Constructor.

Recognised flags include:

=over 4

=item C<frame>

=item C<noMinSize>

=item C<partial>

=item C<regen>

Z<>

=back

=item B<$widget-E<gt>setPadding($left,$right,$top,$bottom)>

=item B<$widget-E<gt>justify($mode)>

=item B<$widget-E<gt>vAlign($mode)>

=item B<$widget-E<gt>sizeHint($numLines,$text)>

=item B<$widget-E<gt>setText($text)>

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

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Label(3)>

=cut
