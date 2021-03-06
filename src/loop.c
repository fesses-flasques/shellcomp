#include	<ncurses.h>
#include	<errno.h>
#include	<sys/wait.h>
#include	<sys/types.h>
#include	<fcntl.h>
#include	<utmp.h>
#include	<pty.h>
#include	"build_opts.h"
#include	"error.h"
#include	"windows.h"

static int forkint;
//#define NO_SECOND
static struct {
  fd_set	fd_save;
  fd_set	fd_select;
  int		fd_l;
  int		fd_r;
}		*g_select = NULL;

int
send_sigint(void) {
  return (EXIT_SUCCESS);
#if 1
  struct termios var;

  tcgetattr(g_select->fd_l, &var);
  var.c_lflag |= ISIG;
  tcsetattr(g_select->fd_l, TCSANOW, &var);
  ioctl(g_select->fd_l, TCSBRK, &var);
#else
  if (ioctl(g_select->fd_l, TIOCSIGNAL, SIGINT) != 0)
    perror("ERROR");
#endif
  return (EXIT_SUCCESS);
}

int
apply_sizes(void) {
  if (send_size(g_select->fd_l) == EXIT_FAILURE
#ifndef NO_SECOND
      || send_size(g_select->fd_r) == EXIT_FAILURE
#endif
     )
    return (EXIT_FAILURE);
  return (EXIT_SUCCESS);
}

static int
forkito(t_opts *opt, char *shell) {
  int	fd_slave;
  int	fd_master;
  int	fork_ret;
  char	*name;

  (void)opt;
  if ((fd_master = open("/dev/ptmx", O_RDWR)) == -1)
    return (-1);
  if (unlockpt(fd_master) == -1)
    return (-1);
  if (!(name = ptsname(fd_master)))
    return (-1);
  if ((fd_slave = open(name, O_RDWR)) == -1)
    return (-1);
  if ((fork_ret = fork()) == -1)
    return (-1);

  if (!(fork_ret)) {	// Fils
    close(fd_master);
    if (login_tty(fd_slave) == -1)
      return (-2);
    execlp(shell, shell, NULL);
    return (-2);
  }

  forkint = fork_ret;
  close(fd_slave);
  return (fd_master);
}

static int
launch_shells(t_opts *opt) {
  char		**rem;

  rem = opts_getarg(opt, 0);
  g_select->fd_l = 0;
  g_select->fd_r = 0;
  if (((g_select->fd_l = forkito(opt, rem[0])) == -1)
#ifndef NO_SECOND
      || ((g_select->fd_r = forkito(opt, rem[1])) == -1)
#endif
     ) {
    return (EXIT_FAILURE);
  }
  if (g_select->fd_l == -2 || g_select->fd_r == -2) {
    return (EXIT_FAILURE);
  }
  if (apply_sizes() == EXIT_FAILURE)
    return (EXIT_FAILURE);
  return (EXIT_SUCCESS);
}

#define	BUFF_READ_SIZE 13
static int
monitoring_checkshell(int fd, int (*cb)(char *, size_t)) {
  char		buff[BUFF_READ_SIZE];
  ssize_t	i;

  if ((i = read(fd, buff, BUFF_READ_SIZE)) == -1) {
    return (EXIT_FAILURE);
  }
  return (cb(buff, i));
}

static int
shell_transmit(char *str, size_t s) {
  if (g_run.running == 0)
    return (EXIT_SUCCESS);
  write(g_select->fd_l, str, s);
#ifndef NO_SECOND
  write(g_select->fd_r, str, s);
#endif
  return (EXIT_SUCCESS);
}

static int
monitoring(t_opts *opt) {
  (void)opt;
  if (FD_ISSET(0, &g_select->fd_select)) {
    monitoring_checkshell(0, &shell_transmit);
  }
  else if (FD_ISSET(g_select->fd_l, &g_select->fd_select)) {
    if (monitoring_checkshell(g_select->fd_l, &write_to_left) == EXIT_FAILURE)
      return (EXIT_FAILURE);
  }
#ifndef NO_SECOND
  else if (FD_ISSET(g_select->fd_r, &g_select->fd_select)) {
    if (monitoring_checkshell(g_select->fd_r, &write_to_right) == EXIT_FAILURE)
      return (EXIT_FAILURE);
  }
#endif
  return (EXIT_SUCCESS);
}

static int
do_select(t_opts *opt, int highest) {
  int		ret;

  memcpy(&g_select->fd_select, &g_select->fd_save, sizeof(fd_set));
  ret = select(highest, &g_select->fd_select, NULL, NULL, NULL);
  if (ret != -1) {
    if ((monitoring(opt) == EXIT_FAILURE)
	|| (update_display() == EXIT_FAILURE)
	|| (refresh_win(opt) == EXIT_FAILURE))
      return (EXIT_FAILURE);
  }
  else if (errno != EINTR)
    return (fail_print(ERR_SLCT));
  return (EXIT_SUCCESS);
}

static int
fd_setter(void) {
  FD_ZERO(&g_select->fd_save);
  FD_SET(0, &g_select->fd_save);
  FD_SET(g_select->fd_l, &g_select->fd_save);
#ifndef NO_SECOND
  FD_SET(g_select->fd_r, &g_select->fd_save);
#endif
  return (EXIT_SUCCESS);
}

int
callback_childs() {
  int		status;
  pid_t		done;

  close(g_select->fd_l);
#ifndef NO_SECOND
  close(g_select->fd_r);
#endif
  while (1) {
    done = wait(&status);
    if (done == -1) {
      if (errno == ECHILD)
	return (EXIT_SUCCESS);
      return (EXIT_FAILURE);
    }
    else {
      if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
	return (EXIT_SUCCESS);
      }
    }
  }
  return (EXIT_SUCCESS);
}

int
loop_init(t_opts *opt) {
  (void)opt;
  if (!(g_select = malloc(sizeof(*g_select))))
    return (fail_print(ERR_MALLOC));
  memset(g_select, 0, sizeof(*g_select));
  return (EXIT_SUCCESS);
}

void
loop_delete(void) {
  if (g_select->fd_l)
    close(g_select->fd_l);
  if (g_select->fd_r)
    close(g_select->fd_r);
  free(g_select);
}

int
loop(t_opts *opt) {
  int		highest;

  if (launch_shells(opt) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  fd_setter();
  highest = (
      g_select->fd_l > g_select->fd_r ?
      g_select->fd_l :
      g_select->fd_r) + 1;
  for (;;) {
    if (do_select(opt, highest) == EXIT_FAILURE) {
      return (EXIT_FAILURE);
    }
  }
  return (EXIT_SUCCESS);
}
