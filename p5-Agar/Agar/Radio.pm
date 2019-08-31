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

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Checkbox(3)>, L<Agar::Label(3)>

=cut
