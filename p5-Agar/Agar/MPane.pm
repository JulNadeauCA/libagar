package Agar::MPane;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::MPane - 2, 3 or 4 Box widgets arranged as panes

=head1 SYNOPSIS

  use Agar;
  use Agar::MPane;
  
  Agar::MPane->new($parent);

=head1 DESCRIPTION

Please see AG_MPane(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::MPane>

=head1 METHODS

=over 4

=item B<$widget = Agar::MPane-E<gt>new($parent, [%options])>

Constructor.

Recognised options include:

=over 4

=item C<hFill>

=item C<vFill>

=item C<frames>

=item C<forceDiv>

Z<>

=back

=item B<$box_widget = $widget-E<gt>pane($index)>

=back

=head1 AUTHOR

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009-2016 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Box(3)>, L<Agar::Fixed(3)>, L<Agar::Pane(3)>,
L<Agar::Scrollview(3)>, L<Agar::Window(3)>

=cut
