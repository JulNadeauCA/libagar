package Agar;

use 5.6.1;
require DynaLoader;

$VERSION = '1.60';
our $XS_VERSION = $VERSION;
$VERSION = eval $VERSION;

our @ISA = qw(DynaLoader);

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

bootstrap Agar;

# Load the base extension with RTLD_GLOBAL.
sub dl_load_flags { 0x01 }

sub Agar::Object::downcast {
	my $class = $_[0]->getClassName();
	if ($class =~ s/^AG_/Agar::/ && $class->isa('Agar::Object')) {
		return bless $_[0], $class;
	}
}

sub Agar::Object::cast {
	return bless $_[0], $_[1];
}

1;

__END__

=head1 NAME

Agar - Perl interface to the Agar GUI library

=head1 SYNOPSIS

  use Agar;
  use Agar::Window;
  use Agar::Label;

  Agar::InitCore() || die Agar::GetError();
  Agar::InitGraphics() || die Agar::GetError();
  Agar::BindStdGlobalKeys();
  
  my $win = Agar::Window->new({ plain => 1 });
  my $lbl = Agar::Label->new($win);
  $lbl->setText("Hello, world! (agar %s)", Agar::Version());

  Agar::EventLoop();

=head1 DESCRIPTION

This is the Perl interface to Agar, a portable and device-independent
graphical application toolkit (available from L<http://libagar.org/>).

This module provides the Agar initialization routines and globals.

=head1 METHODS

=over 4

=item B<Agar::InitCore([progName], [%options])>

Initialize Agar's (non-GUI specific) utility library, Agar-Core.

=item B<Agar::InitGraphics([$driver_spec])>

Initialize the Agar GUI system. If no C<$driver_spec> is given, Agar will
selects the "best" graphics driver for the current platform. Otherwise,
C<$driver_spec> is taken to be a comma-separated list of drivers and
driver options. The first successfully initialized driver will be used.

The special string "<OpenGL>" requests any driver with OpenGL capability,
failing if OpenGL is unavailable. "<SDL>" requests a driver based on the
SDL library (and fail if SDL is unavailable).

See L<AG_InitGraphics(3)> for the list of available drivers.

=item B<Agar::ResizeDisplay($x,$h)>

Change the display size in pixels (single-window drivers only).

=item B<Agar::Version>

Return a scalar value containing the version number of the installed
Agar library.

=item B<Agar::Release>

Return the codename of the installed Agar release.

=item B<Agar::SetError($string)>

Set the Agar error string.

If threads support is enabled, the string uses thread-local storage.

=item B<Agar::GetError>

Return the Agar error string.

=item B<Agar::EventLoop>

Enter the standard Agar event loop (see L<AG_EventLoop(3)>,
L<AG_CustomEventLoop(3)>).

=item B<Agar::Terminate($exitCode)>

Request termination of the event loop associated with the current thread.
If the current thread is the main thread, terminate the application with
C<$exitCode> as return code.

=item B<Agar::GetConfig>

Returns the main Agar::Config object.

=item B<Agar::FindWidget($name)>

Returns the first widget it finds with the specified path name (e.g.,
"box0/button0". Return undef if no widget was found under that name.

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

=item B<createDataDir>

Create a per-user database directory for the application on startup
(if it doesn't already exist). The location of the directory is
platform-specific (under Unix, it is typically $HOME/.progName).
A non-empty progName argument should be provided.

=item B<softTimers>

Force the L<AG_Timer(3)> interface to use a software-based timing wheel
(updated by calling L<AG_ProcessTimeouts(3)>), as opposed to kernel / hardware
based timers where available.

By default, Agar will use the most efficient timer mechanism available
on the host platform (such as L<kqueue(2)>, timerfd or L<select(2)>).

=item B<verbose>

Allow error / warning messages on the standard output (default = no).

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar::Box(3)>, L<Agar::Button(3)>, L<Agar::Checkbox(3)>, L<Agar::Combo(3)>,
L<Agar::Config(3)>, L<Agar::Console(3)>, L<Agar::Editable(3)>, L<Agar::Event(3)>,
L<Agar::FileDlg(3)>, L<Agar::Fixed(3)>, L<Agar::Font(3)>, L<Agar::Label(3)>,
L<Agar::MPane(3)>, L<Agar::Menu(3)>, L<Agar::Notebook(3)>, L<Agar::Numerical(3)>,
L<Agar::Object(3)>, L<Agar::Pane(3)>, L<Agar::PixelFormat(3)>, L<Agar::Pixmap(3)>,
L<Agar::PopupMenu(3)>, L<Agar::ProgressBar(3)>, L<Agar::Radio(3)>,
L<Agar::Scrollbar(3)>, L<Agar::Scrollview(3)>, L<Agar::Separator(3)>,
L<Agar::Slider(3)>, L<Agar::Surface(3)>, L<Agar::Text(3)>, L<Agar::Table(3)>,
L<Agar::Textbox(3)>, L<Agar::Tlist(3)>, L<Agar::Toolbar(3)>, L<Agar::UCombo(3)>,
L<Agar::Widget(3)>, L<Agar::Window(3)>, L<http://libagar.org/>

=cut
