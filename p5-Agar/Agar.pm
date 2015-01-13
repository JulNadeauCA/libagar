package Agar;

use 5.6.1;
use strict;
require DynaLoader;

our @ISA = qw(DynaLoader);
our $VERSION = '1.5.0';

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
  Agar::InitGraphics();

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

=item B<Agar::InitGraphics([$driver_spec])>

Initialize the Agar-GUI library. If C<$driver_spec> is not specified, Agar
selects the "best" graphics driver for the current platform.
The special "<OpenGL>" string requests a driver with OpenGL capability or
fails if none is found. Similarly, "<SDL>" requires a driver based on the
SDL library.

A comma-separated list of specific drivers may be passed as C<$driver_spec>.
See L<AG_InitVideo(3)> for a list of available drivers.

=item B<Agar::ResizeDisplay($x,$h)>

Change the display size.

=item B<Agar::SetRefreshRate($fps)>

Set an advisory video refresh rate in updates per second. This setting is
only used by some video drivers, such as "sdlfb".

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

Block the current thread, wait for low-level events and process them.

=item B<Agar::Terminate($exitCode)>

Request termination of the event loop associated with the current thread.
If the current thread is the main thread, terminate the application with
C<$exitCode> as return code.

=item B<Agar::GetConfig>

Returns the main Agar::Config object.

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

=item B<Agar::Quit()>

Immediately terminate the application.

=item B<Agar::QuitGUI()>

Request termination of the application, but leave an opportunity for custom
exit hooks (such as exit confirmation dialogs).

=back

=head1 CORE OPTIONS

=over 4

=item B<verbose>

If true, Agar can print messages on the standard output / error. Defaults
to false.

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>oktal@gmx.co.ukE<gt>

=head1 MAINTAINER

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009 Hypertriton, Inc. All rights reserved.
This program is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar::Surface>, L<Agar::PixelFormat>, L<Agar::Window>, L<http://libagar.org/>

=cut
