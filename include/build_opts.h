/*
** build_opts.h for  in /home/carbon_q//Epitech/Tek2/042_Zappy/git
** 
** Made by quentin carbonel
** Login   <carbon_q@epitech.net>
** 
** Started on  Thu Jul  5 17:32:31 2012 quentin carbonel
** Last update Thu Jul  5 17:32:32 2012 quentin carbonel
*/

#ifndef		BUILD_OPTS_H_
# define	BUILD_OPTS_H_

# include	<stdlib.h>
# include	<string.h>
# include	<unistd.h>
# ifdef		DEBUG
#  include	<stdio.h>
# endif

# define	ADD_OPTS	(256)
# define	ISLOWER(c)	(c >= 'a' && c <= 'z')
# define	ISUPPER(c)	(c >= 'A' && c <= 'Z')
# define	SRCCASE(c)	(ISLOWER(c) ? 'a' : 'A')
# define	TABOPT(c)	(1 << (c - SRCCASE(c)))
# define	MAPOPT(x, c)	(x & TABOPT(c))
# define	HASARGS(c)	(c == ':' || c == '*')

typedef struct	s_arg
{
  char		opt;
  char		**args;
  struct s_arg	*next;
}		t_arg;

typedef		struct
{
  int		optlow;
  int		optup;
  char		*fmt;
  char		opts[ADD_OPTS];
  char		ignored[ADD_OPTS];
  t_arg		*args;
  char		**rest;
}		t_opts;

char		*opts_getlastarg(t_opts *opts, char c);
int		build_opts(t_opts *, char *, int, char **);
char		**opts_getarg(t_opts *, char);
int		opts_isset(t_opts *, char);
void		unbuild_opts(t_opts *);
int		init_opts(t_opts *, char *);

#endif
