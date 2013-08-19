#include	<stdlib.h>
#include	"shellcomp.h"
#include	"windows.h"
#include	"buffer.h"

static struct {
  t_buff	*l;
  t_buff	*r;
}		g_buff = {
  NULL, NULL
};

static int
init_buff(t_buff *obj) {
  obj->count = 0;
  if (!(obj->buff = malloc(BUFF_SIZE * sizeof(*(obj->buff)))))
    return (EXIT_FAILURE);
  obj->buff[0] = '\n';
  obj->next = NULL;
  return (EXIT_SUCCESS);
}

static int
send_lines(t_buff *b, size_t count, int (*cb)(char *, size_t)) {
  if (!b)
    return (EXIT_SUCCESS);
  if (cb(b->buff + count, b->count - count) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  while ((b = b->next) != NULL) {
    if (cb(b->buff, b->count) == EXIT_FAILURE)
      return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}

inline static int
buff_lines_cb(t_buff *b, size_t i, int (*cb)()) {
  if (send_lines(b, i, cb) == EXIT_FAILURE) {
  }
  return (EXIT_SUCCESS);
}

inline static size_t
set_curr(t_buff *b, size_t i) {
#if 0
  // Replaced old routine less "sexy"
  return (++i >= b->count ? 0 : i);
#else
  return (++i * (i < b->count));
#endif
}

inline static size_t
set_buffcurr(t_buff **b, size_t i) {
  if (!(i = set_curr(*b, i)))
    *b = (*b)->next;
  return (i);
}

static int
buff_lines_each(t_buff *b, size_t i, int (*cb)(), struct winsize *ws) {
  size_t	current = i, nb = 0;
  t_buff	*s = b;

  while (s != NULL) {
    if (nb >= ws->ws_col || s->buff[current] == '\n') {
      current = set_buffcurr(&s, current);
      if (!(nb = 1 + buff_lines_each(s, current, cb, ws)))
	return (-1);
      if (nb >= ws->ws_row) {
	buff_lines_cb(b, i, cb);
	return (-1);
      }
      return (nb);
    }
    current = set_buffcurr(&s, current);
    ++nb;
  }
  return (1);
}

static int
buff_lines(
    t_buff *b,
    int (*cb)(char *, size_t),
    struct winsize *ws
    ) {
  if (buff_lines_each(b, 0, cb, ws) != -1)
    buff_lines_cb(b, 0, cb);
  else
    cb(CRLF, sizeof(CRLF));
  return (EXIT_SUCCESS);
}

int
buff_lines_r(int (*cb)(char *, size_t), struct winsize *ws) {
  return (buff_lines(g_buff.r, cb, ws));
}

int
buff_lines_l(int (*cb)(char *, size_t), struct winsize *ws) {
  return (buff_lines(g_buff.l, cb, ws));
}
#include	<stdio.h>

int
buff_init(t_opts *opt) {
  (void)opt;
  if (!(g_buff.l = malloc(sizeof(*(g_buff.l)))))
    return (EXIT_FAILURE);
  if (!(g_buff.r = malloc(sizeof(*(g_buff.r)))))
    return (EXIT_FAILURE);
  if (init_buff(g_buff.l) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  if (init_buff(g_buff.r) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  return (EXIT_SUCCESS);
}

static void
del_buff(t_buff **obj) {
  t_buff	*tmp = (*obj);

  *obj = (*obj)->next;
  free(tmp->buff);
  free(tmp);
}

static
void
buff_delete_each(t_buff *t) {
  t_buff	*b = t;

  while (t) {
    del_buff(&t);
    t = b->next;
    b = t;
  }
}

void
buff_delete(void) {
  buff_delete_each(g_buff.l);
  buff_delete_each(g_buff.r);
}

static void
buff_flush(t_buff **buff) {
  t_buff	*tmp = *buff;
  size_t	i = 0;

  while (tmp != NULL && i < LIMIT_BUFF) {
    ++i;
    tmp = tmp->next;
  }
  if (tmp != NULL) {
    while (tmp != NULL) {
      del_buff(buff);
      tmp = tmp->next;
    }
    clear_subwin();
  }
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

static int
push_buff(t_buff *buff, char *str, size_t count) {
  size_t	i = 0, d;

  while (buff->next)
    buff = buff->next;
  while (i < count) {
    if (buff->count >= BUFF_SIZE) {
      if ((buff = add_buff(buff)) == NULL)
	return (EXIT_FAILURE);
    }
    d = (((count - i) < (BUFF_SIZE - buff->count)) ?
	count - i :
	BUFF_SIZE - buff->count
	);
    memcpy(buff->buff + buff->count, str + i, d);
    i += d;
    buff->count += d;
  }
  return (EXIT_SUCCESS);
}

int
push_buff_left(char *str, size_t count) {
  buff_flush(&g_buff.l);
  return (push_buff(g_buff.l, str, count));
}

int
push_buff_right(char *str, size_t count) {
  buff_flush(&g_buff.r);
  return (push_buff(g_buff.r, str, count));
}

int cb_func(char *a, size_t b) {
  write(1, a, b);
  return (0);
}

int	failer_buff(void) {
  struct winsize ws = {40, 40, 0, 0};
  if (buff_init(NULL) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  push_buff_left("abcde", 5);
  push_buff_left("fghij", 5);
  push_buff_left("klmno", 5);
  push_buff_left("zxcvmnzmxcvn", 12);
  buff_lines_l(cb_func, &ws);
  return (EXIT_SUCCESS);
}
