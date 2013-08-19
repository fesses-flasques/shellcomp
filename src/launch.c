#include	<curses.h>
#include	"buffer.h"
#include	"windows.h"
#include	"shellcomp.h"

t_run		g_run;

static int
run_init(t_opts *opt) {
  (void)opt;
  g_run.running = 1;
  return (EXIT_SUCCESS);
}

void	win_delete();
void	buff_delete();
void	loop_delete();
int	loop_init(t_opts *);

void
delete_all_modules() {
  win_delete();
  buff_delete();
  loop_delete();
}

int
init_all_modules(t_opts *opt) {
  if (win_init(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  if (buff_init(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  if (loop_init(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  return (EXIT_SUCCESS);
}

int
launch(t_opts *opt) {
  int	ret;

  if (run_init(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  ret = loop(opt);
  if (callback_childs() == EXIT_FAILURE)
    ret = EXIT_FAILURE;
  if (win_destroy(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  delete_all_modules();
  return (ret);
}
