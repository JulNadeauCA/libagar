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

=item B<$value = $widget-E<gt>getBool($name)>

=item B<$value = $widget-E<gt>getInt($name)>

=item B<$value = $widget-E<gt>getUint($name)>

=item B<$value = $widget-E<gt>getSint8($name)>

=item B<$value = $widget-E<gt>getUint8($name)>

=item B<$value = $widget-E<gt>getSint16($name)>

=item B<$value = $widget-E<gt>getUint16($name)>

=item B<$value = $widget-E<gt>getSint32($name)>

=item B<$value = $widget-E<gt>getUint32($name)>

=item B<$value = $widget-E<gt>getFloat($name)>

=item B<$value = $widget-E<gt>getDouble($name)>

=item B<$value = $widget-E<gt>getString($name)>

Each of these methods returns the value of the widget's "binding" (read:
"property") of the specified name and type.

=item B<$widget-E<gt>setBool($name)>

=item B<$widget-E<gt>setInt($name)>

=item B<$widget-E<gt>setUint($name)>

=item B<$widget-E<gt>setSint8($name)>

=item B<$widget-E<gt>setUint8($name)>

=item B<$widget-E<gt>setSint16($name)>

=item B<$widget-E<gt>setUint16($name)>

=item B<$widget-E<gt>setSint32($name)>

=item B<$widget-E<gt>setUint32($name)>

=item B<$widget-E<gt>setFloat($name)>

=item B<$widget-E<gt>setDouble($name)>

=item B<$widget-E<gt>setString($name)>

Each of these methods sets the value of the widget's "binding" (read:
"property") of the specified name and type.

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
