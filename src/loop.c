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

//#define NO_SECOND
static struct {
  fd_set	fd_save;
  fd_set	fd_select;
  int		fd_l;
  int		fd_r;
}		g_select;

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
  close(fd_slave);
  return (fd_master);
}

static int
launch_shells(t_opts *opt) {
  char		**rem;

  rem = opts_getarg(opt, 0);
  g_select.fd_l = 0;
  g_select.fd_r = 0;
  if (((g_select.fd_l = forkito(opt, rem[0])) == -1)
#ifndef NO_SECOND
      || ((g_select.fd_r = forkito(opt, rem[1])) == -1)
#endif
     ) {
    return (EXIT_FAILURE);
  }
  if (g_select.fd_l == -2 || g_select.fd_r == -2) {
    exit(1);
  }
  return (EXIT_SUCCESS);
}

#define	BUFF_SIZE 4096
static int
monitoring_checkshell(int fd, int (*cb)(char *, size_t)) {
  char		buff[BUFF_SIZE];
  ssize_t	i;

  if ((i = read(fd, buff, BUFF_SIZE)) == -1) {
    return (EXIT_FAILURE);
  }
  return (cb(buff, i));
}

static int
shell_transmit(char *str, size_t s) {
  write(g_select.fd_l, str, s);
  write(g_select.fd_r, str, s);
  return (EXIT_SUCCESS);
}

static int
monitoring(t_opts *opt) {
  (void)opt;
  if (FD_ISSET(0, &g_select.fd_select)) {
    monitoring_checkshell(0, &shell_transmit);
  }
  else if (FD_ISSET(g_select.fd_l, &g_select.fd_select)) {
    if (monitoring_checkshell(g_select.fd_l, &write_to_left) == EXIT_FAILURE)
      return (EXIT_FAILURE);
  }
#ifndef NO_SECOND
  else if (FD_ISSET(g_select.fd_r, &g_select.fd_select)) {
    if (monitoring_checkshell(g_select.fd_r, &write_to_right) == EXIT_FAILURE)
      return (EXIT_FAILURE);
  }
#endif
  return (EXIT_SUCCESS);
}

static int
do_select(t_opts *opt, int highest) {
  int		ret;

  memcpy(&g_select.fd_select, &g_select.fd_save, sizeof(fd_set));
  ret = select(highest, &g_select.fd_select, NULL, NULL, NULL);
  if (ret == -1 && errno != EINTR) {
    return (fail_print(ERR_SLCT));
  }
  if (ret != -1) {
    if (monitoring(opt) == EXIT_FAILURE) {
      return (EXIT_FAILURE);
    }
    refresh_win(opt);
  }
  return (EXIT_SUCCESS);
}

static int
fd_setter(void) {
  FD_ZERO(&g_select.fd_save);
  FD_SET(0, &g_select.fd_save);
  FD_SET(g_select.fd_l, &g_select.fd_save);
#ifndef NO_SECOND
  FD_SET(g_select.fd_r, &g_select.fd_save);
#endif
  return (EXIT_SUCCESS);
}

int
callback_childs() {
  int		status;
  pid_t		done;

  close(g_select.fd_l);
#ifndef NO_SECOND
  close(g_select.fd_r);
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
loop(t_opts *opt) {
  int		highest;

  if (launch_shells(opt) == EXIT_FAILURE) {
    return (EXIT_FAILURE);
  }
  fd_setter();
  highest = (
      g_select.fd_l > g_select.fd_r ?
      g_select.fd_l :
      g_select.fd_r) + 1;
  for (;;) {
    if (do_select(opt, highest) == EXIT_FAILURE) {
      return (EXIT_FAILURE);
    }
  }
  return (EXIT_SUCCESS);
}
