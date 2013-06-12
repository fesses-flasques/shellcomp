#ifndef		WINDOWS_H_
# define	WINDOWS_H_

# include	"build_opts.h"
# include	"shellcomp.h"

int		update_display(void);
int		write_to_right(char *, size_t);
int		write_to_left(char *, size_t);
int		reload_interface(t_opts *);
int		win_init(t_opts *);
int		win_destroy(t_opts *);
int		refresh_win(t_opts *);

#endif
