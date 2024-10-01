.POSIX:
.OBJDIR: .
CC = gcc
NAME = lmfit
CFLAGS_COMMON = -Wall -Werror -Wextra -pedantic -Wno-unused -Wno-unused-parameter -Wno-strict-prototypes -g
CFLAGS_DEBUG = $(CFLAGS_COMMON) -DTIMEIT 
IFLAGS = 
LFLAGS = -lm

OBJ_FILES = $(NAME).o

RM = rm -f

all: $(OBJ_FILES)

check: test$(NAME) test$(NAME)_jac
	./test$(NAME)
	./test$(NAME)_jac

clean:
	$(RM) $(NAME) *.o *.a *.so *.dll *.exe test$(NAME) test$(NAME)_jac

.c.o:
	if [ -n"$(SANITIZE)" ] ; then export DBGOPT="-fsanitize=address,undefined"; else export DBGOPT="" ; fi ; \
	$(CC) $(IFLAGS) $(CFLAG_COMMONS) $$DBGOPT -c $< -o $@

test$(NAME): test$(NAME).o $(OBJ_FILES)
	if [ -n"$(SANITIZE)" ] ; then export DBGOPT="-fsanitize=address,undefined"; else export DBGOPT="" ; fi ; \
	$(CC) $(IFLAGS) $(CFLAGS_DEBUG) $$DBGOPT test$(NAME).o $(OBJ_FILES) -o $@ $(LFLAGS)

test$(NAME)_jac: test$(NAME)_jac.o $(OBJ_FILES)
	if [ -n"$(SANITIZE)" ] ; then export DBGOPT="-fsanitize=address,undefined"; else export DBGOPT="" ; fi ; \
	$(CC) $(IFLAGS) $(CFLAGS_DEBUG) $$DBGOPT test$(NAME)_jac.o $(OBJ_FILES) -o $@ $(LFLAGS)
