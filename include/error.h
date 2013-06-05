#ifndef		ERROR_H_
# define	ERROR_H_

typedef enum
{
  PERROR,
  ERR_OPT,
  ERR_MALLOC,
  ERR_SIG,
  ERR_RESIZE,
  ERR_CLEAR,
  ERR_IOCTL,
  ERR_SLCT,
  ERR_NEWWIN,
  ERR_BOX,
  ERR_WINRSZ,
  ERR_MVWIN
}		e_errndx;

int	fail_print(e_errndx);

#endif
