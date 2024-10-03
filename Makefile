.POSIX:
.OBJDIR: .
CC = gcc
NAME = lmfit
CFLAGS_COMMON = -Wall -Werror -Wextra -pedantic -Wno-unused -Wno-unused-parameter -Wno-strict-prototypes -g3 -O2
CFLAGS_DEBUG = $(CFLAGS_COMMON) -DTIMEIT
IFLAGS = 
LFLAGS = -lm

OBJ_FILES = $(NAME).o

RM = rm -f

all: $(OBJ_FILES)

check: test$(NAME) test$(NAME)_jac $(NAME)_query
	./test$(NAME)
	./test$(NAME)_jac
	./$(NAME)_query 9 5 5

clean:
	$(RM) $(NAME) *.o *.so test$(NAME) test$(NAME)_jac $(NAME)_query

.c.o:
	@if [ -n "$(SANITIZE)" ] ; then export DBGOPT="-fsanitize=address,undefined"; else export DBGOPT="" ; fi ; \
	$(CC) $(IFLAGS) $(CFLAGS_COMMON) $$DBGOPT -c $< -o $@

$(NAME)_query: $(NAME)_query.c $(OBJ_FILES)
	@if [ -n "$(SANITIZE)" ] ; then export DBGOPT="-fsanitize=address,undefined"; else export DBGOPT="" ; fi ; \
	$(CC) $(IFLAGS) $(CFLAGS_COMMON) $$DBGOPT $(NAME)_query.c $(OBJ_FILES) -o $@ $(LFLAGS)

test$(NAME): test$(NAME).c $(OBJ_FILES)
	@if [ -n "$(SANITIZE)" ] ; then export DBGOPT="-fsanitize=address,undefined"; else export DBGOPT="" ; fi ; \
	$(CC) $(IFLAGS) $(CFLAGS_DEBUG) $$DBGOPT test$(NAME).c $(OBJ_FILES) -o $@ $(LFLAGS)

test$(NAME)_jac: test$(NAME)_jac.c $(OBJ_FILES)
	@if [ -n "$(SANITIZE)" ] ; then export DBGOPT="-fsanitize=address,undefined"; else export DBGOPT="" ; fi ; \
	$(CC) $(IFLAGS) $(CFLAGS_DEBUG) $$DBGOPT test$(NAME)_jac.c $(OBJ_FILES) -o $@ $(LFLAGS)
