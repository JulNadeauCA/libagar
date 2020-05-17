package Agar::Config;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Config - interface to Agar's configuration interface

=head1 SYNOPSIS

  use Agar;
  use Agar::Config;
  
  # global Agar properties:
  $config = Agar::GetConfig();
  # object properties:
  $config = $object->getProps();

=head1 DESCRIPTION

This class provides access to AG_Prop(3), which is used for global
configuration parameters and also the properties of individual objects.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> B<Agar::Config>

=head1 METHODS

=over 4

=item B<$config-E<gt>lock()>

Obtains the mutex for modifying this config object.

=item B<$config-E<gt>unlock()>

Releases the mutex for modifying this config object.

=item B<$config-E<gt>load($path)>

Load the configuration properties from the named file.

=item B<$config-E<gt>save($path)>

Save the configuration properties to the named file.

=item B<$value = $config-E<gt>getBool($name)>

=item B<$value = $config-E<gt>getInt($name)>

=item B<$value = $config-E<gt>getUint($name)>

=item B<$value = $config-E<gt>getUint8($name)>

=item B<$value = $config-E<gt>getSint8($name)>

=item B<$value = $config-E<gt>getUint16($name)>

=item B<$value = $config-E<gt>getSint16($name)>

=item B<$value = $config-E<gt>getUint32($name)>

=item B<$value = $config-E<gt>getSint32($name)>

=item B<$value = $config-E<gt>getFloat($name)>

=item B<$value = $config-E<gt>getDouble($name)>

=item B<$value = $config-E<gt>getString($name)>

=item B<$config-E<gt>getInt($name, $value)>

=item B<$config-E<gt>getUint($name, $value)>

=item B<$config-E<gt>getUint8($name, $value)>

=item B<$config-E<gt>getSint8($name, $value)>

=item B<$config-E<gt>getUint16($name, $value)>

=item B<$config-E<gt>getSint16($name, $value)>

=item B<$config-E<gt>getUint32($name, $value)>

=item B<$config-E<gt>getSint32($name, $value)>

=item B<$config-E<gt>getFloat($name, $value)>

=item B<$config-E<gt>getDouble($name, $value)>

=item B<$config-E<gt>getString($name, $value)>

These methods set and get the values of properties of different types.

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>

=cut
