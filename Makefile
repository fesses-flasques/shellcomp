NAME	=	shellcomp

src_dir	=	src/
obj_dir	=	obj/
inc_dir	=	include/

SRC	=	\
		$(src_dir)shellcomp.c				\
		$(src_dir)windows.c				\
		$(src_dir)launch.c				\
		$(src_dir)buffer.c				\
		$(src_dir)loop.c				\
		$(src_dir)error.c#				\
		\
		$(src_dir)/build_opts/build_opts.c		\
		$(src_dir)/build_opts/build_opts_pub.c

PRECC	=	$(obj_dir)/build_opts/build_opts.o				\
		$(obj_dir)/build_opts/build_opts_pub.o

OBJ	+=	$(SRC:.c=.o)


LDFLAGS	+=	-lncurses
LDFLAGS	+=	-lutil
CFLAGS	+=	-D_GNU_SOURCE
CFLAGS	+=	-Wall -Wextra
CFLAGS	+=	-I$(inc_dir)

RM	=	rm -fv
CC	=	clang

$(NAME):	$(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LDFLAGS) $(PRECC)

all:		$(NAME)

clean:
	@$(RM)	$(OBJ)

fclean:		clean
	@$(RM)	$(NAME)

re:		fclean all

.PHONY:		all clean fclean re
