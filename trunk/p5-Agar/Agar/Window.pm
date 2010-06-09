package Agar::Window;

use strict;
use Agar;

;1

__END__

=head1 NAME

Agar::Window - Agar GUI window

=head1 SYNOPSIS

  use Agar;
  use Agar::Window;
  use Agar::Surface;
  
  my $win = Agar::Window->new;
  my $modalWin = Agar::Window->new({ modal => 1 });
  my $namedWin = Agar::Window->newNamed("Hello");
  $win->caption("Hello, world");
  $win->icon(Agar::Surface->newFromBMP('icon.bmp'));

=head1 DESCRIPTION

The B<Agar::Window> object is the basic container of the Agar GUI. It extends
B<Agar::Widget>.

=head1 METHODS

=over 4

=item B<$win = Agar::Window-E<gt>new([\%options])>

Create a new, unnamed window. Available %options include:

=over 4

=item C<keepAbove>

Always keep this window on top of the other windows.

=item C<keepBelow>

Always keep this window below the other windows.

=item C<denyFocus>

Don't automatically grab focus in response to a click on the window area.

=item C<modal>

Place window in foreground and prevent all other windows from catching
any events. Multiple modal windows are arranged in a stack and whichever
was shown last (with the B<show> method) is the effective one. Implies the
C<noMaximize> and C<noMinimize> options.

=item C<noBackground>

Don't fill the window background prior to rendering its contents.

=item C<noUpdateRect>

Disable automatic updates of the video region corresponding to the
window area (may not be relevant depending on the display mode in
use).

=item C<noTitle>

Disable the window titlebar.

=item C<noBorders>

Don't draw decorative window borders.

=item C<plain>

Implies C<noTitle> and C<noBorders>.

=item C<noHResize>

Disable the horizontal window resize control.

=item C<noVResize>

Disable the vertical window resize control.

=item C<noResize>

Disable both resize controls.

=item C<noClose>

Disable the "close window" button in the titlebar.

=item C<noMinimize>

Disable the "minimize window" button in the titlebar.

=item C<noMaximize>

Disable the "maximize window" button in the titlebar.

=item C<hMaximize>

Arrange for the width of the window to always match the width of the
display.

=item C<vMaximize>

Arrange for the height of the window to always match the height of the
display.

=back

=item B<$win = Agar::Window-E<gt>newNamed($name, [\%options])>

Same as the B<new> constructor, except that a name is associated with the
window. If an already existing window has the given name, the constructor
returns undef.

=item B<$text = $win-E<gt>caption([$text])>

Return the current caption string. If the $text argument is given, set the
caption as well. $text may contain valid UTF-8.

=item B<$icon = $win-E<gt>icon([$icon])>

Windows have icons associated with them. These icons are displayed by Agar's
internal window manager when windows are minimized. This method return the
current icon associated with the window. If the $icon argument is given, set
the icon as well. Both the $icon parameter and the return value are
L<Agar::Surface> objects.

=item B<$win-E<gt>setGeometry($x,$y,$w,$h)>

Sets the position and size of the window to the specified (x,y) coordinates
and width and height.

=item B<$win-E<gt>setMinSize($w,$h)>

Enforce a minimum width and height on the window.

=item B<$win-E<gt>show()>

Enable drawing of this window.

=item B<$win-E<gt>hide()>

Disable drawing of this window.

=item B<$win-E<gt>draw()>

Cause the window to draw itself. Only really useful when writing a custom
event loop.

=item B<$win-E<gt>attach()>

Attach a "logical" child window to this window.
When the window is detached, the child window will be automatically
detached as well.

=item B<$win-E<gt>detach()>

Remove a "logical" child window from this window.

=item B<$widget = $win-E<gt>findFocused()>

Returns the child widget that currently has focus, or undef if there is none.

=back

=head1 AUTHOR

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

=head1 COPYRIGHT

Copyright (c) 2008 Hypertriton, Inc. All rights reserved.
This program is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar>, L<Agar::Surface>, L<Agar::Widget>

=cut
