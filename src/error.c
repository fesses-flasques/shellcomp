#include	<stdio.h>
#include	<stdlib.h>
#include	"error.h"

static char *err_table[] = {
  NULL,
  "Bad options input. See -h for help",
  "Malloc failure",
  "signal() failure",
  "ncurses: resizeterm() failure",
  "ncurses: clear() failure",
  "ioctl() failure",
  "select() failure",
  "ncurses: newwin() failure",
  "ncurses: box() failure",
  "ncurses: wresize() failure",
  "ncurses: mvwin() failure"
};

int
fail_print(e_errndx ndx) {
  if (ndx == PERROR)
    perror("Error");
  else
    fprintf(stderr, "%s\n",
	ndx < (sizeof(err_table) / sizeof(*err_table)) ?
	err_table[ndx] :
	"Error index out of bound"
	);
  return (EXIT_FAILURE);
}
