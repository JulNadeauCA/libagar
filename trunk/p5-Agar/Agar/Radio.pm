package Agar::Radio;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Radio - a group of radio buttons

=head1 SYNOPSIS

  use Agar;
  use Agar::Radio;
  
  Agar::Radio->new($parent);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_Radio(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Radio-E<gt>new($parent)>

Constructor.

=item B<$index = $widget-E<gt>addItem($label)>

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

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Radio(3)>

=cut
