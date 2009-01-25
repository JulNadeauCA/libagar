package Agar::Tlist;

use strict;
use Agar;

1;

__END__

=head1 NAME

Agar::Tlist - an (optionally nested) list widget

=head1 SYNOPSIS

  use Agar;
  use Agar::Tlist;
  
  Agar::Tlist->new($parent);

=head1 DESCRIPTION

Extends Agar::Widget and Agar::Object. Please see AG_Tlist(3) for a
full explanation of what its methods do and what bindings and events
it defines, if any.

=head1 METHODS

=over 4

=item B<$widget = Agar::Tlist-E<gt>new($parent, { flags })>

Constructor.

Recognised flags include:

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

=head1 MAINTAINER

Julien Nadeau E<lt>F<vedge@hypertriton.com>E<gt>

=head1 COPYRIGHT

Copyright (c) 2009 Hypertriton, Inc. All rights reserved.
This program is free software. You can redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Agar>, L<Agar::Widget>, L<Agar::Object>, L<AG_Tlist(3)>

=cut
