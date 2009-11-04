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

Extends Agar::Widget and Agar::Object. Please see AG_Menu(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Menu-E<gt>new($parent,{flags})>

=item B<$widget = Agar::Menu-E<gt>newGlobal({flags})>

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

=head1 MAINTAINER

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Menu(3)>, L<Agar::Surface>

=cut
