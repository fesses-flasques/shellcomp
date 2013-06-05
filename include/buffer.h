#ifndef		BUFFER_H_
# define	BUFFER_H_

typedef	struct	s_buff {
  size_t	count;
  char		*buff;
  struct s_buff	*next;
}		t_buff;

int	push_buff_left(char *, size_t);
int	push_buff_right(char *, size_t);
#endif
