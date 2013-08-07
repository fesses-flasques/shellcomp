#ifndef		BUFFER_H_
# define	BUFFER_H_

# include	<sys/ioctl.h>
# include	"build_opts.h"
# define	BUFF_SIZE	1024
# define	LIMIT_BUFF	9 // TODO
# define	CRLF		"\r\n"

typedef	struct	s_buff {
  size_t	count;
  char		*buff;
  struct s_buff	*next;
}		t_buff;

int	push_buff_left(char *, size_t);
int	push_buff_right(char *, size_t);
int	buff_init(t_opts *);
void	buff_dumper();
int	buff_lines_r(int (*)(char *, size_t), struct winsize *);
int	buff_lines_l(int (*)(char *, size_t), struct winsize *);

#endif
