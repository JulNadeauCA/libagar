package Agar::Notebook;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Notebook - a tabbed container of multiple Box widgets

=head1 SYNOPSIS

  use Agar;
  use Agar::Notebook;
  
  Agar::Notebook->new($parent);

=head1 DESCRIPTION

Please see AG_Notebook(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Notebook>

=head1 METHODS

=over 4

=item B<$widget = Agar::Notebook-E<gt>new($parent, [%options])>

Constructor.

Recognised options include:

=over 4

=item C<hideTabs>

Z<>

=back

=item B<$widget-E<gt>setTabAlignment($mode)>

=item B<$tab = $widget-E<gt>addHorizTab($label)>

=item B<$tab = $widget-E<gt>addVertTab($label)>

=item B<$widget-E<gt>delTab($tab)>

=item B<$widget-E<gt>selectTab($tab)>

=back

=head1 TABS

The tabs in an Agar::Notebook implement the Agar::NotebookTab class.

=over 4

=item B<$box_widget = $tab-E<gt>box()>

=item B<$text = $tab-E<gt>getLabel()>

=back

=head1 AUTHOR

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009-2016 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Box(3)>, L<Agar::Pane(3)>

=cut
