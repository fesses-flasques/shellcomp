#include	<stdlib.h>
#include	"buffer.h"

static struct {
  t_buff	l;
  t_buff	r;
}		g_buff;

int
push_buff(t_buff *buff, char *str, size_t count) {
  (void)str;
  (void)buff;
  (void)count;
  return (EXIT_SUCCESS);
}

int
push_buff_left(char *str, size_t count) {
  return (push_buff(&g_buff.l, str, count));
}

int
push_buff_right(char *str, size_t count) {
  return (push_buff(&g_buff.r, str, count));
}
