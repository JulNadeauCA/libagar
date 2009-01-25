package Agar::UCombo;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::UCombo - a more configurable drop-down list widget

=head1 SYNOPSIS

  use Agar;
  use Agar::UCombo;
  
  Agar::UCombo->new($parent);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_UCombo(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::UCombo-E<gt>new($parent, { flags })>

Constructor.

Recognised flags include:

=over 4

=item C<scrollToSel>

Z<>

=back

=item B<$widget-E<gt>sizeHint($text,$numItems)>

=item B<$widget-E<gt>sizeHintPixels($w,$h)>

=item B<$list_widget = $widget-E<gt>list()>

=item B<$button_widget = $widget-E<gt>button()>

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

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_UCombo(3)>, L<Agar::Tlist>,
L<Agar::Button>

=cut
