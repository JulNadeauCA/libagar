package Agar::Pane;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Pane - 2 Box widgets arranged as panes

=head1 SYNOPSIS

  use Agar;
  use Agar::Pane;
  
  Agar::Pane->newVert($parent);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_Pane(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Pane-E<gt>newHoriz($parent,{flags})>

=item B<$widget = Agar::Pane-E<gt>newVert($parent,{flags}>

Constructors.

Recognised flags include:

=over 4

=item C<div1Fill>

=item C<forceDiv1Fill>

=item C<frame>

=item C<div>

=item C<forceDiv>

=item C<unmovable>

Z<>

=back

=item B<$widget-E<gt>setDividerWidth($pixels)>

=item B<$widget-E<gt>setDivisionMin($pixels)>

=item B<$box_widget = $widget-E<gt>leftPane()>

=item B<$box_widget = $widget-E<gt>rightPane()>

=item B<$box_widget = $widget-E<gt>topPane()>

=item B<$box_widget = $widget-E<gt>bottomPane()>

=item B<$widget-E<gt>moveDivider($x)>

=item B<$widget-E<gt>moveDividerPct($percent)>

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

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Pane(3)>, L<Agar::Box>

=cut
