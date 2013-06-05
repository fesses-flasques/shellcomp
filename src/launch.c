#include	<curses.h>
#include	"windows.h"
#include	"shellcomp.h"

t_run		g_run;

static int
init_run(t_opts *opt) {
  (void)opt;
  g_run.running = 1;
  return (EXIT_SUCCESS);
}

int
launch(t_opts *opt) {
  int	ret;

  if (init_run(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  if (win_init(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  ret = loop(opt);
  if (callback_childs() == EXIT_FAILURE)
    ret = EXIT_FAILURE;
  if (win_destroy(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  return (ret);
}
