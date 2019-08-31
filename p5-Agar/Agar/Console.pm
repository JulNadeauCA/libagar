package Agar::Console;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Console - for displaying lines of text like terminal output

=head1 SYNOPSIS

  use Agar;
  use Agar::Console;
  
  Agar::Console->new($parent);

=head1 DESCRIPTION

Please see AG_Console(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Console>

=head1 METHODS

=over 4

=item B<$widget = Agar::Console-E<gt>new($parent)>

Constructor.

=item B<$widget-E<gt>setPadding($pixels)>

=item B<$widget-E<gt>msg($text)>

=item B<$scrollbar_widget = $widget-E<gt>scrollBar()>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Console(3)>, L<Agar::Object(3)>, L<Agar::Scrollbar(3)>,
L<Agar::Widget(3)>

=cut
