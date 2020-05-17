#!/usr/bin/env perl

use Agar;
use Agar::Window;
use Agar::Label;
#use Agar::Separator;

Agar::InitCore() || die Agar::GetError();
Agar::InitGraphics() || die Agar::GetError();
my $win = Agar::Window->new({ main => 1 });
$win->caption("perl $0");
$win->setStyle('color', 'rgb(0,100,0)');
$win->setStyle('font-family', 'Helvetica');
$win->setStyle('font-weight', 'bold');

my $lbl = Agar::Label->new($win, 'Hello, world!');
$lbl->setStyle('font-size', '300%');

#Agar::Separator->newHoriz($win);

my $lbl = Agar::Label->new($win, 'Agar v' . Agar::Version);
$lbl->setStyle('font-size', '160%');
$lbl->setStyle('text-color', 'rgb(50%,90%,50%)');

Agar::Label->new($win, "Perl $^V");

$win->show();

Agar::EventLoop();

