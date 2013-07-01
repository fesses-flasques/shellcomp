#include	<signal.h>
#include <sys/ioctl.h>
#include	<stdlib.h>
#include	<unistd.h>
static struct winsize wins;

void
take_it(int useless) {
  (void)useless;
  ioctl(0, TIOCGWINSZ, &wins);
  printf("%u _ %u\n", wins.ws_col, wins.ws_row);
}

signals_init() {
  signal(SIGWINCH, take_it);
}

int
main() {
  signals_init();
  take_it(9);
  while (1) pause();
}
