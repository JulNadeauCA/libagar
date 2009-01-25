package Agar;

use 5.6.1;
use strict;
require DynaLoader;

our @ISA = qw(DynaLoader);
our $VERSION = '1.3.3';

@Agar::Widget::ISA = qw(Agar::Object);
@Agar::Window::ISA = qw(Agar::Widget);
@Agar::Box::ISA = qw(Agar::Widget);
@Agar::Button::ISA = qw(Agar::Widget);
@Agar::Checkbox::ISA = qw(Agar::Widget);
@Agar::Combo::ISA = qw(Agar::Widget);
@Agar::Console::ISA = qw(Agar::Widget);
@Agar::Editable::ISA = qw(Agar::Widget);
@Agar::FileDlg::ISA = qw(Agar::Widget);
@Agar::Fixed::ISA = qw(Agar::Widget);
@Agar::Label::ISA = qw(Agar::Widget);
@Agar::Menu::ISA = qw(Agar::Widget);
@Agar::MPane::ISA = qw(Agar::Widget);
@Agar::Notebook::ISA = qw(Agar::Widget);
@Agar::Numerical::ISA = qw(Agar::Widget);
@Agar::Pane::ISA = qw(Agar::Widget);
@Agar::PopupMenu::ISA = qw(Agar::Widget);
@Agar::ProgressBar::ISA = qw(Agar::Widget);
@Agar::Radio::ISA = qw(Agar::Widget);
@Agar::Scrollbar::ISA = qw(Agar::Widget);
@Agar::Scrollview::ISA = qw(Agar::Widget);
@Agar::Separator::ISA = qw(Agar::Widget);
@Agar::Slider::ISA = qw(Agar::Widget);
@Agar::Textbox::ISA = qw(Agar::Widget);
@Agar::Tlist::ISA = qw(Agar::Widget);
@Agar::Toolbar::ISA = qw(Agar::Widget);
@Agar::UCombo::ISA = qw(Agar::Widget);

bootstrap Agar $VERSION;

sub Agar::Object::downcast {
	my $class = $_[0]->getClassName();
	if ($class =~ s/^AG_/Agar::/ && $class->isa('Agar::Object')) {
		return bless $_[0], $class;
	}
}

1;

__END__

=head1 NAME

Agar - Perl interface to the Agar GUI library

=head1 SYNOPSIS

  use Agar;

  Agar::InitCore({ verbose => 1 }) || die "Agar: ".Agar::GetError();
  Agar::InitVideo(640, 480, 32, {
  	fullScreen => 0,
  	resizable => 1,
  	openGL => 0,
  	openGLOrSDL => 1,
  });

  printf "Agar version = %s (%s)\n",
      Agar::Version(),
      Agar::Release();

  Agar::EventLoop();

=head1 DESCRIPTION

This is the Perl interface to Agar, a portable and device-independent
graphical application toolkit (see L<http://libagar.org/> for more
information). This specific module deals with the Agar initialization
routines and globals.

=head1 METHODS

=over 4

=item B<Agar::InitCore([%options])>

Initialize the Agar-Core library. At this point, there is no video or GUI
specific code involved (see L</CORE OPTIONS>).

=item B<Agar::InitVideo($width,$height,$depth,[%options])>

Initialize the Agar-GUI library and create a new video display of the
specified dimensions in pixels, and depth in bits per pixel (see
L</VIDEO OPTIONS>).

=item B<Agar::InitVideoSDL($sdl_surface,[%options])>

In cooperation with the SDL_perl bindings, this call is similar to InitVideo
but allows you to provide your own SDL::Surface for drawing to.

WARNING: When using the Agar Perl bindings with SDL_perl, you must C<use Agar>
I<before> you C<use> any SDL modules.

=item B<Agar::ResizeDisplay($x,$h)>

Change the display size.

=item B<Agar::SetRefreshRate($fps)>

Attempt to limit the number of updates per second.

=item B<Agar::Version>

Return a scalar value containing the version number of the installed
Agar library.

=item B<Agar::Release>

Return the codename of the installed Agar release.

=item B<Agar::SetError($string)>

Set the Agar error string. When Agar is compiled for multithreading,
this is actually a thread-specific value.

=item B<Agar::GetError>

Return the Agar error string. This is a scalar value which can contain
UTF-8 characters.

=item B<Agar::EventLoop>

Agar's default event loop.

=item B<Agar::BeginRendering>

Any calls to any Widgets' Draw methods must be preceded by a call to this
method. Only needed for writing a custom event loop.

=item B<Agar::EndRendering>

Call this after each frame's batch of drawing is complete. Only needed for a
custom event loop.

=item B<Agar::ProcessEvent($event)>

Pass a single SDL::Event into Agar. Only needed for a custom event loop.

=item B<Agar::ProcessTimeouts($ticks)>

Updates Agar's timing clock. Expects to be passed the output of
SDL::App::ticks. Only needed for writing a custom event loop.

=item B<Agar::GetConfig>

Returns the main Agar::Config object.

=item B<Agar::GetViewObject>

Returns the root view surface as an Agar::Object. Only needed for writing a
custom event loop.

=item B<Agar::DrawAll>

Causes all windows to be drawn. Only needed for writing a custom event loop.

=item B<Agar::FindWidget($name)>

Returns the first widget it finds with the specified name, or undef if none
was found.

=item B<Agar::FindObject($name)>

Returns the first object it finds with the specified name, or undef if none
was found.

=item B<Agar::FindWidgetAtPoint($x, $y)>

Returns the deepest widget in the object tree at the specified absolute screen
coordinates, or undef if none was found.

=item B<Agar::FindWidgetOfClassAtPoint($class, $x, $y)>

Returns the deepest widget of the specified Perl-style class in the object
tree at the specified absolute screen coordinates, or undef if none was found.

=item B<Agar::InfoMsg($text)>

Creates a modal dialog box displaying some informational text.

=item B<Agar::WarningMsg($text)>

Creates a modal dialog box displaying some warning text.

=item B<Agar::ErrorMsg($text)>

Creates a modal dialog box displaying some error text.

=item B<Agar::InfoMsgTimed($ms, $text)>

Creates a modal dialog box displaying some informational text that disappears
after the specified number of milliseconds.

=item B<Agar::WarningMsgTimed($ms, $text)>

Creates a modal dialog box displaying some warning text that disappears
after the specified number of milliseconds.

=item B<Agar::ErrorMsgTimed($ms, $text)>

Creates a modal dialog box displaying some error text that disappears
after the specified number of milliseconds.

=item B<Agar::InfoMsgIgnorable($key, $text)>

Creates a modal dialog box displaying some informational text, with a checkbox
that allows the user to ignore subsequent messages with the same key string.

=item B<Agar::WarningMsgIgnorable($key, $text)>

Creates a modal dialog box displaying some warning text, with a checkbox.
that allows the user to ignore subsequent messages with the same key string.

=item B<Agar::PromptMsg($text, $codeRef)>

Creates a dialog box with a text entry widget, and calls the specified code
reference when a value has been entered.

This increments the internal reference count of the code reference, then
decrements it again after it's been called.

=back

=head1 CORE OPTIONS

=over 4

=item B<verbose>

If true, Agar can print messages on the standard output / error. Defaults
to false.

=back

=head1 VIDEO OPTIONS

=over 4

=item B<fullScreen>

Request a full-screen display on initialization. May or may not be relevant
or available.

=item B<resizable>

Whether to allow the user to resize the application window. Only meaningful
if the underlying graphics system involves a window manager.

=item B<openGL>

Require that the underlying graphics system provide an OpenGL context, and
instruct the Agar GUI to use OpenGL for all rendering. If OpenGL is not
available, the initialization will fail.

=item B<openGLOrSDL>

Always use OpenGL if it is available, otherwise fall back to the SDL for
video rendering. For applications designed for modern hardware, which do not
include any OpenGL or SDL specific code, this is recommended.

=item B<hwSurface>

Attempt to create the video surface in hardware memory. Specific to the
underlying graphics system.

=item B<asyncBlits>

In SDL mode, use the B<SDL_ASYNCBLIT> flag. This flag is recommended only
when the application is executing on a multiprocessor system.

=item B<anyFormat>

In SDL mode, use the B<SDL_ANYFORMAT> flag. This flag disables operation
through an emulated surface for the specified depth.

=item B<hwPalette>

When using color-index display mode, request exclusive palette access.

=item B<doubleBuf>

Enable hardware double-buffering. Only valid in conjunction with B<hwSurface>
on concerned graphics systems.

=item B<noFrame>

When using a window system, disable window decorations associated with the
application window.

=item B<bgPopupMenu>

Set up an event handler such that a right-click on the empty window background
causes Agar's internal window manager to display a pop-up menu. Disabled by
default.

=item B<noBgClear>

Prevent Agar's internal window manager from clearing the background of
the display.

=back

=head1 AUTHOR

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

=head1 COPYRIGHT

Copyright (c) 2008 Hypertriton, Inc. All rights reserved.
This program is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar::Surface>, L<Agar::PixelFormat>, L<Agar::Window>, L<http://libagar.org/>

=cut
