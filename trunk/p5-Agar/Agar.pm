package Agar;

use 5.6.1;
use strict;
require DynaLoader;

our @ISA = qw(DynaLoader);
our $VERSION = '1.3.3';

bootstrap Agar $VERSION;

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
