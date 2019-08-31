package Agar::Menu;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Menu - a (possibly nested) menubar widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Menu;
  
  Agar::Menu->new($parent);

=head1 DESCRIPTION

Please see AG_Menu(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Menu>

=head1 METHODS

=over 4

=item B<$widget = Agar::Menu-E<gt>new($parent,[%options])>

=item B<$widget = Agar::Menu-E<gt>newGlobal([%options])>

Constructors.

=item B<$widget-E<gt>rootItem()>

=item B<$widget-E<gt>expandItem($item, $x, $y)>

=item B<$widget-E<gt>collapseItem($item)>

=item B<$widget-E<gt>setPadding($l,$r,$t,$b)>

=item B<$widget-E<gt>setLabelPadding($l,$r,$t,$b)>

=back

=head1 ITEMS

Items within Agar::Menus implement the Agar::MenuItem class.

=over 4

=item B<$child = $item-E<gt>nodeItem($text)>

=item B<$child = $item-E<gt>actionItem($text, \&coderef)>

=item B<$child = $item-E<gt>actionItemKbd($text, $key, $modifiers, \&coderef)>

=item B<$item-E<gt>separator()>

=item B<$item-E<gt>section($text)>

=item B<$item-E<gt>setIcon($surface)>

=item B<$item-E<gt>setLabel($text)>

=item B<$item-E<gt>enable()>

=item B<$item-E<gt>disable()>

=item B<$selected_child = $item-E<gt>selected()>

=item B<$parent = $item-E<gt>parentItem()>

=item B<$parent = $item-E<gt>parentMenu()>

=back

=head1 POPUP MENUS

A popup menu might be used when the user right-clicks on something, or in a
similar situation.

=over 4

=item B<$popup = Agar::PopupMenu-E<gt>new($parent)>

Constructor.

=item B<$popup-E<gt>show()>

=item B<$popup-E<gt>hide()>

=item B<$popup-E<gt>destroy()>

=item B<$item = $popup-E<gt>rootItem()>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Window(3)>

=cut
