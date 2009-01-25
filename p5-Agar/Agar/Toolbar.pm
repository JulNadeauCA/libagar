package Agar::Toolbar;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Toolbar - a widget containing row(s) of Buttons

=head1 SYNOPSIS

  use Agar;
  use Agar::Toolbar;
  
  Agar::Toolbar->newHoriz($parent, 1);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_Toolbar(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Toolbar-E<gt>newHoriz($parent,$numRows,{flags})>

=item B<$widget = Agar::Toolbar-E<gt>newVert($parent,$numRows,{flags})>

Constructors.

Recognised flags include:

=over 4

=item C<homogenous>

=item C<sticky>

=item C<multiSticky>

Z<>

=back

=item B<$widget-E<gt>setActiveRow($row)>

=item B<$button_widget = $widget-E<gt>addTextButton($text)>

=item B<$button_widget = $widget-E<gt>addIconButton($surface)>

=item B<$widget-E<gt>select($button)>

=item B<$widget-E<gt>deselect($button)>

=item B<$widget-E<gt>selectOnly($button)>

=item B<$widget-E<gt>selectAll()>

=item B<$widget-E<gt>deselectAll()>

=item B<$row = $widget-E<gt>getRow($index)>

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

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Toolbar(3)>, L<Agar::Button>

=cut
