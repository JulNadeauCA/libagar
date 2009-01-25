package Agar::Textbox;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Textbox - a more configurable editable textbox widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Textbox;
  
  Agar::Textbox->new($parent);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_Textbox(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Textbox-E<gt>new($parent, { flags })>

Constructor.

Recognised flags include:

=over 4

=item C<multiLine>

=item C<static>

=item C<password>

=item C<abandonFocus>

=item C<intOnly>

=item C<floatOnly>

=item C<catchTab>

=item C<noEmacs>

=item C<noWordSeek>

=item C<noLatin1>

Z<>

=back

=item B<$widget-E<gt>sizeHint($text)>

=item B<$widget-E<gt>sizeHintPixels($w,$h)>

=item B<$widget-E<gt>sizeHintLines($numLines)>

=item B<$pos = $widget-E<gt>getCursorPos()>

=item B<$widget-E<gt>setCursorPos($pos)>

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

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Textbox(3)>

=cut
