package Agar::Object;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Object - Agar's general base class

=head1 SYNOPSIS

  use Agar::Object;

=head1 DESCRIPTION

This is the root class for all Agar objects. It provides useful non-GUI specific
features such as event processing / virtual functions, portable serialization,
and a virtual filesystem.

The Agar object system is implemented in the (non-GUI specific) ag_core
library. Agar GUI classes such as L<Agar::Widget(3)> all inherit from
B<Agar::Object>.

=head1 INHERITANCE HIERARCHY

B<Agar::Object>

=head1 METHODS

=over 4

=item B<$object-E<gt>root()>

Returns the C<Agar::Object> that is at the root of the object tree in which
this object resides.

=item B<$object-E<gt>lock()>

Acquires the mutex that protects the read/write members of the object.

=item B<$object-E<gt>unlock()>

Releases the mutex previously acquired by C<lock>.

=item B<$object-E<gt>setEvent($eventName,$codeRef)>

Arranges for events of the specified type to be handled by the subroutine
provided. Overwrites any handler that was previously in place for this event
type on this object.

When the handler subroutine is called it shall have one argument: the
Agar::Event object that is being handled.

=item B<$object-E<gt>unsetEvent($eventName)>

Removes the event handler registered with the object for the specified event
type.

=item B<$codeRef = $object-E<gt>findEventHandler($eventName)>

Returns the handler subroutine associated with the specified event type on
this object. If no handler is found, it returns undef. If a handler is found
that was not created from Perl, it returns a false but defined value.

=item B<$object-E<gt>parent()>

Returns the parent object of this one.

=item B<$object-E<gt>findChild($name)>

Returns the first object found with the specified name among the descendants
of the object, or undef if none is found.

=item B<$object-E<gt>attachTo($newParent)>

Attaches the object to another, effectively changing its parent object.

=item B<$object-E<gt>detach()>

Detach the object from its parent, making it an orphan.

=item B<$object-E<gt>setName()>

Sets the object's name.

=item B<$object-E<gt>downcast()>

With a plain Agar::Object or Agar::Widget, attempts to re-bless the reference
into the derived class furthest down the inheritance tree.

Useful for processing the result of Agar::Event::receiver or
Agar::Object::findChild, for example.

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Event(3)>

=cut
