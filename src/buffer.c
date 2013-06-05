#include	<stdlib.h>
#include	"buffer.h"

static struct {
  t_buff	*l;
  t_buff	*r;
}		g_buff;

#include	<stdio.h>
#define	SEP "%ld ===================================\n"
void
buff_dumper() {
  t_buff *a = g_buff.l;
  while (a) {
    fprintf(stdout, SEP, a->count);
    fflush(stdout);
    write(1, a->buff, a->count);
    write(1, "\n", 1);
    a = a->next;
  }
}

static int
init_buff(t_buff *obj) {
  obj->count = 0;
  if (!(obj->buff = malloc(BUFF_SIZE * sizeof(*(obj->buff)))))
    return (EXIT_FAILURE);
  obj->next = NULL;
  return (EXIT_SUCCESS);
}

int
buff_init(t_opts *opt) {
  (void)opt;
  if (!(g_buff.l = malloc(sizeof(*(g_buff.l)))))
    return (EXIT_FAILURE);
  if (!(g_buff.r = malloc(sizeof(*(g_buff.r))))) {
    free(g_buff.r);
    return (EXIT_FAILURE);
  }
  if (init_buff(g_buff.l) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  if (init_buff(g_buff.r) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  return (EXIT_SUCCESS);
}

static t_buff *
add_buff(t_buff *obj) {
  t_buff	*new;

  if (!(new = malloc(sizeof(*new))))
    return (NULL);
  if (init_buff(new) == EXIT_FAILURE)
    return (NULL);
  obj->next = new;
  return (new);
}

int
push_buff(t_buff *buff, char *str, size_t count) {
  size_t	i = 0, d;

  while (buff->next) buff = buff->next;

  while (i < count) {
    if (buff->count == BUFF_SIZE) {
      if ((buff = add_buff(buff)) == NULL) {
	return (EXIT_FAILURE);
      }
    }
    d = (((count - i) < (BUFF_SIZE - buff->count)) ? count - i : BUFF_SIZE - buff->count);
    memcpy(buff->buff + buff->count, str + i, d);
    i += d;
    buff->count += d;
  }
  return (EXIT_SUCCESS);
}

int
push_buff_left(char *str, size_t count) {
  return (push_buff(g_buff.l, str, count));
}

int
push_buff_right(char *str, size_t count) {
  return (push_buff(g_buff.r, str, count));
}
