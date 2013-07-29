#include	<curses.h>
#include	<sys/ioctl.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	"error.h"
#include	"buffer.h"
#include	"windows.h"
#include	"shellcomp.h"

static struct winsize	g_winsize;

static struct {
  WINDOW	*main;
  WINDOW	*bd_left;
  WINDOW	*bd_right;
  WINDOW	*left;
  WINDOW	*right;
  int		bd_x;
  int		bd_y;
  int		x;
  int		y;
}		*g_windows = NULL;

int
send_size(int fd) {
  struct winsize	ws;

  ws.ws_row = g_windows->y;
  ws.ws_col = g_windows->x;

  // Those are ignored
  ws.ws_xpixel = 0;
  ws.ws_ypixel = 0;

  if (ioctl(fd, TIOCSWINSZ, &ws) == -1)
    return (fail_print(ERR_IOCTL));
  return (EXIT_SUCCESS);
}

int
write_to_window(WINDOW *w, char *str, size_t s) {
  size_t	i = 0;

  while (i < s) {
    if (str[i] != '\r')
      wprintw(w, "%c", str[i]);
    ++i;
  }
  return (EXIT_SUCCESS);
}

int
write_to_right(char *str, size_t s) {
  return (push_buff_right(str, s));
}

int
write_to_left(char *str, size_t s) {
  return (push_buff_left(str, s));
}

int
update_display_r(char *str, size_t s) {
  return (write_to_window(g_windows->right, str, s));
}

int
update_display_l(char *str, size_t s) {
  return (write_to_window(g_windows->left, str, s));
}

int
update_display(void) {
  register unsigned short	x, y;

  if (wmove(g_windows->left, 0, 0) == ERR ||
      wmove(g_windows->right, 0, 0) == ERR)
    return (EXIT_FAILURE);
  x = g_windows->x;
  y = g_windows->y;
  if (buff_lines_r(&update_display_r, x, y) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  if (buff_lines_l(&update_display_l, x, y) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  return (EXIT_SUCCESS);
}

static int
aff_invalid_range(t_opts *opt, int aff) {
  int		i = g_winsize.ws_col * g_winsize.ws_row;
  static WINDOW	*n = NULL;

  (void)opt;
  if (aff) {
    if (!n) {
      if (!(n = newwin(g_winsize.ws_row, g_winsize.ws_col, 0, 0)))
	return (EXIT_SUCCESS);
    }
    wmove(n, 0, 0);
    wattron(n, COLOR_PAIR(1));
    while (i-- > 0)
      wprintw(n, " ");
    wattroff(n, COLOR_PAIR(1));
    refresh();
    wrefresh(n);
    return (EXIT_SUCCESS);
  }
  delwin(n);
  n = NULL;
  return (EXIT_SUCCESS);
}

static int
invalid_range(void) {
  if (g_winsize.ws_row <= 6 || g_winsize.ws_col <= 18) {
    g_run->running = 0;
    return (EXIT_FAILURE);
  }
  g_run->running = 1;
  aff_invalid_range(NULL, 0);
  return (EXIT_SUCCESS);
}

int
refresh_win(t_opts *opt) {
  (void)opt;
  refresh();
  if (wrefresh(g_windows->bd_left) == ERR)
    return (EXIT_FAILURE);
  if (wrefresh(g_windows->left) == ERR)
    return (EXIT_FAILURE);
  if (wrefresh(g_windows->bd_right) == ERR)
    return (EXIT_FAILURE);
  if (wrefresh(g_windows->right) == ERR)
    return (EXIT_FAILURE);
  return (EXIT_SUCCESS);
}

static WINDOW *
init_winboxes_bd(t_opts *opt, int posx, int posy) {
  WINDOW	*win;

  (void)opt;
  if (!(win = newwin(g_windows->bd_y, g_windows->bd_x, posx, posy))) {
    fail_print(ERR_NEWWIN);
    return (NULL);
  }
  if (box(win, 0, 0) != OK) {
    fail_print(ERR_BOX);
    return (NULL);
  }
  return (win);
}

static int
init_winboxes(t_opts *opt) {
  if (!(g_windows->bd_left = init_winboxes_bd(opt, 0, 0)))
    return (EXIT_FAILURE);
  if (!(g_windows->bd_right = init_winboxes_bd(opt, 0, g_windows->bd_x)))
    return (EXIT_FAILURE);

  if (!(g_windows->left = newwin(g_windows->y, g_windows->x, 1, 1)))
    return (fail_print(ERR_NEWWIN));
  if (!(g_windows->right = newwin(g_windows->y, g_windows->x,
	  1, g_windows->bd_x + 1)))
    return (fail_print(ERR_NEWWIN));
  /**/
  return (EXIT_SUCCESS);
}

int
term_sizing(t_opts *opt) {
  if (ioctl(0, TIOCGWINSZ, &g_winsize) == -1)
    return (fail_print(ERR_IOCTL));
  if (resize_term(g_winsize.ws_row, g_winsize.ws_col) == ERR)
    return (fail_print(ERR_RESIZE));
  g_windows->bd_x = (g_winsize.ws_col / 2);
  g_windows->bd_y = g_winsize.ws_row;
  g_windows->x = g_windows->bd_x - 2;
  g_windows->y = g_windows->bd_y - 2;
  if (invalid_range() == EXIT_FAILURE)
    return (aff_invalid_range(opt, 1));
  return (EXIT_SUCCESS);
}

static int
apply_tc_windows(int (*pfunc)(WINDOW *, int, int), int y, int x, e_errndx e) {
  if (pfunc(g_windows->bd_left, y, x) != OK)
    return (fail_print(e));
  if (pfunc(g_windows->bd_right, y, x) != OK)
    return (fail_print(e));
  return (EXIT_SUCCESS);
}

static int
resize_wins(void) {
  if (wresize(g_windows->left, g_windows->y, g_windows->x) == ERR)
    return (fail_print(ERR_WINRSZ));
  if (wresize(g_windows->right, g_windows->y, g_windows->x) == ERR)
    return (fail_print(ERR_WINRSZ));
  return (EXIT_SUCCESS);
}

static int
mv_wins(void) {
  if (mvwin(g_windows->bd_right, 0, g_windows->bd_x))
    return (fail_print(ERR_MVWIN));
  if (mvwin(g_windows->right, 1, g_windows->bd_x + 1))
    return (fail_print(ERR_MVWIN));
  return (EXIT_SUCCESS);
}

int
reload_interface(t_opts *opt) {
  wclear(g_windows->main);
  wclear(g_windows->bd_left);
  wclear(g_windows->bd_right);

#if 0
  if (term_sizing(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
#endif
  if (!g_run->running)
    return (EXIT_SUCCESS);
  if (apply_tc_windows(
	wresize, g_windows->bd_y, g_windows->bd_x, ERR_WINRSZ
	) == EXIT_FAILURE ||
      apply_tc_windows(
	(void *)box, 0, 0, ERR_BOX
	) == EXIT_FAILURE ||
      resize_wins() == EXIT_FAILURE ||
      mv_wins() == EXIT_FAILURE)
    return (EXIT_FAILURE);
  if (refresh_win(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  return (EXIT_SUCCESS);
}

static int
load_interface(t_opts *opt) {
  wclear(g_windows->main);
  if (term_sizing(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  if (init_winboxes(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  if (g_run->running)
    if (refresh_win(opt) == EXIT_FAILURE)
      return (EXIT_FAILURE);
  return (EXIT_SUCCESS);
}

int
win_init(t_opts *opt) {
  if (!(g_windows = malloc(sizeof(*g_windows))))
    return (fail_print(ERR_MALLOC));
  if (!(g_windows->main = initscr()))
    return (fail_print(-1));
  keypad(stdscr, true);
  cbreak();
  noecho();
  start_color();
  use_default_colors();
  init_pair(1, COLOR_RED, COLOR_RED);
  return (load_interface(opt));
}

int
win_destroy(t_opts *opt) {
  (void)opt;
  logger("End win\n");
  endwin();
  logger("endwindone\n");
  exit(1);
  return (EXIT_SUCCESS);
}
