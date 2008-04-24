use Agar;

Agar::InitCore('foo', {verbose => 1});
Agar::InitVideo(320, 240, 32, {resizable => 1});

my $win = Agar::Window->new();
$win->caption("Perl ($0)");
$win->show();

Agar::EventLoop();

