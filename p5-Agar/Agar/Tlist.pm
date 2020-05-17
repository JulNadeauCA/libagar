package Agar::Tlist;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Tlist - an (optionally nested) list view widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Tlist;
  
  Agar::Tlist->new($parent);

=head1 DESCRIPTION

Please see AG_Tlist(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 INHERITANCE HIERARCHY

L<Agar::Object(3)> -> L<Agar::Widget(3)> -> B<Agar::Tlist>

=head1 METHODS

=over 4

=item B<$widget = Agar::Tlist-E<gt>new($parent, [%options])>

Constructor.

Recognised options include:

=over 4

=item C<multi>

=item C<multiToggle>

=item C<tree>

Z<>

=back

=item B<$widget-E<gt>setItemHeight($pixels)>

=item B<$widget-E<gt>setIcon($item,$surface)>

=item B<$widget-E<gt>sizeHint($text,$numItems)>

=item B<$widget-E<gt>sizeHintPixels($w,$h)>

=item B<$widget-E<gt>sizeHintLargest($numItems)>

=item B<$item = $widget-E<gt>addItem($text)>

=item B<$widget-E<gt>delItem($item)>

=item B<$widget-E<gt>beginRebuild()>

=item B<$widget-E<gt>endRebuild()>

=item B<$widget-E<gt>select($item)>

=item B<$widget-E<gt>selectAll()>

=item B<$widget-E<gt>deselect($item)>

=item B<$widget-E<gt>deselectAll()>

=item B<$item = $widget-E<gt>findByIndex($index)>

=item B<$item = $widget-E<gt>selectedItem()>

=item B<$item = $widget-E<gt>findText($text)>

=back

=head1 ITEMS

The items in a list implement the Agar::TlistItem class.

=over 4

=item B<$bool = $item-E<gt>isSelected()>

=item B<$text = $item-E<gt>getText()>

=item B<$item-E<gt>setText($text)>

=item B<$n = $item-E<gt>getDepth()>

=item B<$item-E<gt>setDepth($n)>

=item B<$bool = $item-E<gt>isExpanded()>

=item B<$item-E<gt>setExpanded($bool)>

=item B<$bool = $item-E<gt>hasChildren()>

=item B<$item-E<gt>setHasChildren($bool)>

=item B<$item-E<gt>setNoSelect($bool)>

=back

=head1 AUTHOR

Mat Sutcliffe E<lt>F<oktal@gmx.co.uk>E<gt>

Julien Nadeau Carriere E<lt>F<vedge@csoft.net>E<gt>

=head1 SEE ALSO

L<Agar(3)>, L<Agar::Table(3)>, L<Agar::TreeTbl(3)>

=cut
