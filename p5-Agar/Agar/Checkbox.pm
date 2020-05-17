package Agar::Checkbox;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Checkbox - a 2-state checkbox widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Checkbox;
  
  Agar::Checkbox->new($parent);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_Checkbox(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Checkbox>

=head1 METHODS

=over 4

=item B<$widget = Agar::Checkbox-E<gt>new($parent, [%options])>

Constructor.

Recognised options include:

=over 4

=item C<set>

Z<>

=back

=item B<$widget-E<gt>toggle()>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Button(3)>

=cut
