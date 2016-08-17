package Agar::FileDlg;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::FileDlg - a file selection dialog widget

=head1 SYNOPSIS

  use Agar;
  use Agar::FileDlg;
  
  Agar::FileDlg->new($parent);

=head1 DESCRIPTION

Please see AG_FileDlg(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::FileDlg>

=head1 METHODS

=over 4

=item B<$widget = Agar::FileDlg-E<gt>new($parent, [%options])>

Constructor.

Recognised options include:

=over 4

=item C<multi>

=item C<closeWin>

=item C<load>

=item C<save>

=item C<async>

Z<>

=back

=item B<$widget-E<gt>setDirectory($path)>

=item B<$widget-E<gt>setFilename($name)>

=item B<$widget-E<gt>okAction($coderef)>

=item B<$widget-E<gt>cancelAction($coderef)>

=back

=head1 AUTHOR

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009-2016 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar(3)>

=cut
