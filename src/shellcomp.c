#include	<fcntl.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	"build_opts.h"
#include	"shellcomp.h"
#include	"error.h"
#include	"windows.h"

void logger(char *str) {
  static FILE *fd = NULL;
  if (!fd) {
    fd = fopen("abc", "w");
    return ;
  }
  fprintf(fd, "%s", str);
  //fflush(fd);
}
void logger_int(int i) {
  char		*str = NULL;
  asprintf(&str, "val %d\n", i);
  logger(str);
}

int send_sigint(void);

void
sig_catch(int sig) {
  if (sig == SIGWINCH) {
    if (term_sizing(NULL) == EXIT_FAILURE)
      exit(EXIT_FAILURE);
    if (apply_sizes() == EXIT_FAILURE)
      exit(EXIT_FAILURE);
    clear_subwin();
    if (reload_interface(NULL) == EXIT_FAILURE)
      exit(EXIT_FAILURE);
  }
  if (sig == SIGINT) {
    send_sigint();
  }
}

static void
display_usage(void) {
  fprintf(stdout,
      "%s\n",
      "-h: display this help\n"
      "./interfash SHELL1 SHELL2"
      );
}

static int
check_opt(t_opts *opt) {
  char		**rem;

  if (opts_isset(opt, 'h')) {
    display_usage();
    return (EXIT_SUCCESS);
  }
  if (!(rem = opts_getarg(opt, 0)) || !(*rem) || !(rem[1]))
    return (EXIT_FAILURE);
  return (EXIT_SUCCESS);
}

static int
init_signals(__sighandler_t assign) {
  int		signum_tab[] = {
//    SIGINT,
    SIGTERM,
    SIGWINCH,
    SIGQUIT
  };
  unsigned int  i;

  i = 0;
  while (i < (sizeof(signum_tab) / sizeof(*signum_tab)))
    if (signal(signum_tab[i++], assign) == SIG_ERR)
      return (fail_print(ERR_SIG));
  return (EXIT_SUCCESS);
}

int	failer_buff(void);

int
main(int argc, char **argv) {
  t_opts	opt;

  logger(NULL); // TMP
#if 0
  return (failer_buff());
#endif
  if (build_opts(&opt, "h", argc, argv) == EXIT_FAILURE)
    return (fail_print(ERR_OPT));
  if (check_opt(&opt) == EXIT_FAILURE)
    return (fail_print(ERR_OPT));
  if (init_all_modules(&opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  if (init_signals(&sig_catch))
    return (fail_print(ERR_SIG));
  return (launch(&opt));
}
