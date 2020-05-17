#!/usr/bin/env perl
#
# Public domain
#
# Demonstrates many of the Agar-GUI widgets available through the Perl
# bindings.
# 
# Usage: perl agardemo.pl [--driver=DRIVER]
# 
# The "dummy" driver can be used to test from command-line.
#

use strict;
use warnings;
use Agar;
use Getopt::Long;

my $driver = "";

GetOptions("driver=s" => \$driver);

#
# Initialize the Agar library.
#
Agar::InitCore('agarperldemo', { verbose => 1,
                                 createDataDir => 1 }) || die Agar::GetError();

Agar::InitGraphics($driver) || die Agar::GetError();

# Bind ESCAPE and other standard keys.
Agar::BindStdGlobalKeys();

# Create a new Agar window.
my $win = Agar::Window->newNamed("main");
$win->caption("$0 (perl $^V)");

#
# Arrange for Agar::Quit() to be called when the window is closed.
#
# Note: An alternate way to accomplish the same thing would be to
# pass { main => 1 } to Agar::Window->new().
#
$win->setEvent('window-close', sub { Agar::Quit() });

# Set an explicit window geometry (default = auto-size).
# $win->setGeometry(80,60, 640,480);


################################
# Load/save dialogs
################################

my $loadWin = Agar::Window->new({ keepAbove=>1 });
$loadWin->caption('Load');
my $loadDlg = Agar::FileDlg->new($loadWin, { load=>1 });
$loadWin->setEvent('window-close', sub { $loadWin->hide() });
#$loadDlg->cancelAction(sub { $loadWin->hide() });

my $saveWin = Agar::Window->new({ keepAbove=>1 });
$saveWin->caption('Save');
my $saveDlg = Agar::FileDlg->new($saveWin, { save=>1 });
$saveWin->setEvent('window-close', sub { $saveWin->hide() });
#$saveDlg->cancelAction(sub { $saveWin->hide() });

################################
# Menu
################################

my $menuroot = Agar::Menu->new($win);
my $menu = $menuroot->rootItem()->nodeItem('Menu');
$menu->actionItem('Click me!', sub {
	Agar::InfoMsg("Don't do that! " .
                      "\xF0\x9F\x98\x89");              # U+1f609 WINKING FACE
 });
$menu->separator();
my $submenu = $menu->nodeItem('Hover over me');
$submenu->actionItem('Click me, too!', sub { Agar::InfoMsg("Oi!") });

################################
# Tabs
################################

my $nb = Agar::Notebook->new($win, { hFill=>1, vFill=>1 });
my $tab1 = $nb->addVertTab('One')->box();
my $tab2 = $nb->addVertTab('Two')->box();
my $tab3 = $nb->addVertTab('Three')->box();

my $pane = Agar::Pane->newHoriz($tab1, { hFill=>1, vFill=>1 });
my $lbox = $pane->leftPane();
my $rbox = $pane->rightPane();

################################
# Console
################################

my $con = Agar::Console->new($rbox, { hFill=>1, vFill=>1 });

################################
# Buttons
################################

my $button = Agar::Button->new($lbox, "We're off to Button Moon;");
my $checkbox = Agar::Checkbox->new($lbox, "we'll follow Mr. Spoon.");
my $sticky = Agar::Button->new($lbox, "Button Moon!", { sticky=>1 });
my $radio = Agar::Radio->new($lbox);
$radio->addItem("foo");
$radio->addItem("bar");
$radio->addItem("baz");

Agar::Separator->newHoriz($lbox);

################################
# Textboxes
################################

my $textbox = Agar::Textbox->new($lbox, 'Edit me');
$textbox->expandHoriz();
my $spin = Agar::Numerical->new($lbox, 'Spin spin spin');
$spin->setRangeInt(0, 100);
$spin->setPrecision('f', 0);
my $slider = Agar::Slider->newHoriz($lbox, { hFill=>1 });
$slider->setInt('value', 0);
$slider->setInt('min', 0);
$slider->setInt('max', 10);

Agar::Separator->newHoriz($lbox);

################################
# Drop-down lists
################################

my $combo = Agar::Combo->new($lbox, 'Choose your foos', { hFill=>1 });
$combo->sizeHint('blah', 3);
$combo->list()->addItem('foo');
$combo->list()->addItem('bar');
$combo->list()->addItem('baz');
my $ucombo = Agar::UCombo->new($lbox, { hFill=>1 });
$ucombo->sizeHint('blah', 3);
$ucombo->list()->addItem('foo');
$ucombo->list()->addItem('bar');
$ucombo->list()->addItem('baz');

Agar::Separator->newHoriz($lbox);

################################
# More buttons
################################

my $toolbar = Agar::Toolbar->newHoriz($lbox, 5);
$toolbar->addTextButton("Load...")->setEvent('button-pushed', sub {
	$loadWin->show();
});
$toolbar->addTextButton("Save...")->setEvent('button-pushed', sub {
	$saveWin->show();
});
$toolbar->setActiveRow(1);
$toolbar->addTextButton("Info")->setEvent('button-pushed', sub {
	Agar::InfoMsg("Do you know the Mushroom Man?");
});
$toolbar->addTextButton("Warning")->setEvent('button-pushed', sub {
	Agar::WarningMsg("Look, a three-headed monkey!");
});
$toolbar->addTextButton("Error")->setEvent('button-pushed', sub {
	Agar::ErrorMsg("Look, a three-headed monkey!");
});
$toolbar->setActiveRow(2);
$toolbar->addTextButton("InfoTimed")->setEvent('button-pushed', sub {
	Agar::InfoMsgTimed(2000, "Do you know the Mushroom Man?");
});
$toolbar->addTextButton("ErrorTimed")->setEvent('button-pushed', sub {
	Agar::ErrorMsgTimed(2000, "Look, a three-headed monkey!");
});
$toolbar->setActiveRow(3);
$toolbar->addTextButton("InfoIgnorable")->setEvent('button-pushed', sub {
	Agar::InfoMsgIgnorable('demo.ignore-info', "Do you know the Mushroom Man?");
});
#$toolbar->setActiveRow(4);
#$toolbar->addTextButton("Ask me")->setEvent('button-pushed', sub {
#	Agar::PromptMsg("What are you?", sub {
#		my $what = $_[0]->string(1);
#		$con->msg("you are '$what'");
#	});
#});

################################
# List (under 2nd tab)
################################

my $list = Agar::Tlist->new($tab2);
$list->sizeHint('M' x 20, 15);
$list->addItem('Simpsons');
$list->addItem('Homer')->setDepth(1);
$list->addItem('Marge')->setDepth(1);
$list->addItem('Bart')->setDepth(1);
$list->addItem('Lisa')->setDepth(1);
$list->addItem('Maggie')->setDepth(1);
$list->addItem('South Park');
$list->addItem('Stan')->setDepth(1);
$list->addItem('Kyle')->setDepth(1);
$list->addItem('Kenny')->setDepth(1);
$list->addItem('Cartman')->setDepth(1);

################################
# Event handlers
################################

$button->setEvent('button-pushed', sub {
	$con->msg("button pressed");
});
$checkbox->setEvent('checkbox-changed', sub {
	my $state = $_[0]->int(1) ? '' : 'un';
	$con->msg("checkbox ${state}checked");
});
$sticky->setEvent('button-pushed', sub {
	my $state = $_[0]->int(1) ? 'on' : 'off';
	$con->msg("sticky button pressed $state");
});
$radio->setEvent('radio-changed', sub {
	my $index = $_[0]->int(1);
	$con->msg("radio button changed to $index");
});
$textbox->setEvent('textbox-return', sub {
	my $text = $_[0]->receiver()->cast('Agar::Textbox')->getString();
	$con->msg("entered text '$text' (return)");
});
$textbox->setEvent('textbox-postchg', sub {
	my $text = $_[0]->receiver()->cast('Agar::Textbox')->getString();
	$con->msg("entered text '$text'");
});
$spin->setEvent('numerical-changed', sub {
	my $value = $_[0]->receiver()->downcast()->getDouble('value');
	$con->msg("spin control changed to $value");
});
$slider->setEvent('slider-changed', sub {
	my $value = $_[0]->receiver()->downcast()->getInt('value');
	$con->msg("slider changed to $value");
});
$combo->setEvent('combo-selected', sub {
	my $text = $_[0]->receiver()->downcast()->list()->selectedItem()->getText();
	$con->msg("combo selected $text");
});
$ucombo->setEvent('ucombo-selected', sub {
	my $text = $_[0]->receiver()->downcast()->list()->selectedItem()->getText();
	$con->msg("ucombo selected $text");
});
$loadDlg->setEvent('file-chosen', sub {
	my $path = $_[0]->string(1);
	$loadWin->hide();
	$con->msg("chose to load '$path'");
});
$saveDlg->setEvent('file-chosen', sub {
	my $path = $_[0]->string(1);
	$saveWin->hide();
	$con->msg("chose to save '$path'");
});

#
# Move the divider to X = (45% of the total width).
#
$pane->moveDividerPct(45);

################################
# Finished creating widgets,
# now run the program!
################################
$win->show();

Agar::EventLoop();

