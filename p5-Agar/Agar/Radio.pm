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

Please see AG_Radio(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Radio>

=head1 METHODS

=over 4

=item B<$widget = Agar::Radio-E<gt>new($parent)>

Constructor.

=item B<$index = $widget-E<gt>addItem($label)>

=back

=head1 AUTHOR

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009-2016 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Checkbox(3)>, L<Agar::Label(3)>

=cut
