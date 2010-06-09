package Agar::Widget;

use strict;
use Agar;

;1

__END__

=head1 NAME

Agar::Widget - base type for all Agar widgets

=head1 SYNOPSIS

  use Agar;
  use Agar::Widget;

=head1 DESCRIPTION

B<Agar::Widget> is the base type for all of Agar's widget types. It extends
B<Agar::Object>.

=head1 METHODS

=over 4

=item B<$widget-E<gt>draw()>

Causes the widget to draw itself and all its children. You shouldn't usually
need to call this.

=item B<$widget-E<gt>enable()>

Reverses the action of C<disable>. Causes the widget to be able to accept user
input again (the default state).

=item B<$widget-E<gt>disable()>

Causes the widget to cease accepting user input.

=item B<$enabled = $widget-E<gt>isEnabled()>

Returns a true value if and only if the widget is enabled (i.e. able to accept
user input).

=item B<$disabled = $widget-E<gt>isDisabled()>

The logical inverse of C<isEnabled>.

=item B<$widget-E<gt>setFocusable($on)>

Accepts a true or false value to allow or forbid the widget to take focus.

=item B<$widget-E<gt>focus()>

Gives focus to the widget (and all of its parent widgets, up to the
parent window if there is one).

=item B<$widget-E<gt>unfocus()>

Takes focus away from the widget and its children.

=item B<$focused = $widget-E<gt>isFocused()>

Returns a true value if and only if the widget, as well as its parent window,
are both holding focus.

=item B<$focused = $widget-E<gt>isFocusedInWindow()>

Returns a true value if and only if the widget currently has focus inside
of its parent window.

=item B<$win = $widget-E<gt>window()>

Returns the C<Agar::Window> object that contains this widget.

=item B<$widget-E<gt>requestSize($w,$h)>

Requests that the widget be given a certain width and height in pixels. It is
possible the request may be denied.

=item B<$widget-E<gt>setSize($w,$h)>

Forces the widget to have a certain width and height in pixels. Could cause
strange-looking layouts if not used carefully.

=item B<$w = $widget-E<gt>x()>

Returns the x coordinate of the widget's position relative to its parent
widget, or in absolute view coordinates if it is a window.

=item B<$h = $widget-E<gt>y()>

Returns the y coordinate of the widget's position relative to its parent
widget, or in absolute view coordinates if it is a window.

=item B<$w = $widget-E<gt>w()>

Returns the width of the widget in pixels.

=item B<$h = $widget-E<gt>h()>

Returns the height of the widget in pixels.

=item B<$widget-E<gt>setFlag($name)>

Sets the named flag to true.

=item B<$widget-E<gt>unsetFlag($name)>

Sets the named flag to false.

=item B<$flag = $widget-E<gt>getFlag($name)>

Returns the value of the named flag.

=item B<$flag = $widget-E<gt>expandHoriz()>

Expand the widget to fill all available space in the parent container
widget, horizontally.

=item B<$flag = $widget-E<gt>expandVert()>

Expand the widget to fill all available space in the parent container
widget, vertically.

=item B<$flag = $widget-E<gt>expand()>

Expand the widget to fill all available space in the parent container,
both horizontally and vertically.

=item B<$widget-E<gt>redraw()>

Request a redraw of the widget to the display as soon as possible.

=item B<$widget-E<gt>redrawOnChange($refresh_ms, $binding_name)>

Request an automatic redraw of the widget whenever the value associated
with the given binding changes.
The value is checked for changes at the specified interval in milliseconds
(specify an interval of -1 to undo).

=item B<$widget-E<gt>redrawOnTick($refresh_ms)>

Request an unconditional redraw of the widget at the specified interval
in milliseconds (specify an interval of -1 to disable).

=back

=head1 BASE FLAGS

The following flags may be passed in the constructor of the Widget subclass
being instantiated, in addition to any flags which the subclass might define.

=over 4

=item B<hFill>

Hint to container widgets that in a vertical packing, this widget can expand
to fill all remaining space.

=item B<vFill>

Hint to container widgets that in a horizontal packing, this widget can expand
to fill all remaining space.

=item B<hide>

Disable rendering of this widget (does not affect its children). Can be
modified with show/hide methods.

=item B<disabled>

Disable user input for this widget. Can be modified with enable/disable
methods.

=item B<focusable>

Widget is allowed to grab the focus.

=item B<unfocusedMotion>

Always receive C<mouse-motion> events, even when out of focus.

=item B<unfocusedButtonUp>

Always receive C<mouse-button-up> events, even when out of focus.

=item B<unfocusedButtonDown>

Always receive C<mouse-button-down> events, even when out of focus.

=item B<catchTab>

Tab key generates ordinary C<key-up> and C<key-down> events
instead of moving the focus between child widgets.

=item B<noSpacing>

Hint to this widget's parent that it doesn't want spacing or padding applied
to it.

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

=head1 MAINTAINER

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009 Hypertriton, Inc. All rights reserved.
This program is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar>, L<Agar::Object>, L<Agar::Window>

=cut
