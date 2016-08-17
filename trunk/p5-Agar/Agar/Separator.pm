package Agar::Separator;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Separator - a simple line for visually separating widgets

=head1 SYNOPSIS

  use Agar;
  use Agar::Separator;
  
  Agar::Separator->newVert($parent);

=head1 DESCRIPTION

Please see AG_Separator(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Separator>

=head1 METHODS

=over 4

=item B<$widget = Agar::Separator-E<gt>newHoriz($parent,[%options])>

=item B<$widget = Agar::Separator-E<gt>newVert($parent,[%options])>

=item B<$widget = Agar::Separator-E<gt>newHorizSpacer($parent,[%options])>

=item B<$widget = Agar::Separator-E<gt>newVertSpacer($parent,[%options])>

Constructors.

=item B<$widget-E<gt>setPadding($pixels)>

=back

=head1 AUTHOR

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009-2016 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Box(3)>, L<Agar::Fixed(3)>

=cut
