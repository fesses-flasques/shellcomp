#ifndef		INTERFASH_H_
# define	INTERFASH_H_

# include	<signal.h>
# include	"build_opts.h"

int		loop(t_opts *);
int		launch(t_opts *);
void		sig_catch(int);
int		callback_childs();

//TMP
void	logger_int(int);
void	logger(char *);

typedef		struct {
  int		running;
}		t_run;

extern t_run	g_run;

#endif
