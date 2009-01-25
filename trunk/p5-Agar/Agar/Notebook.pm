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

Extends Agar::Widget and Agar::Object. Please see AG_Notebook(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Notebook-E<gt>new($parent, { flags })>

Constructor.

Recognised flags include:

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

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

=head1 MAINTAINER

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Notebook(3)>, L<Agar::Box>

=cut
