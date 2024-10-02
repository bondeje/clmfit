.POSIX:
.OBJDIR: .
CC = cl
NAME = lmfit
CFLAGS_COMMON = /Wall /WX /W3 /wd4820 /wd4711 /wd4710 /wd4100 /wd4668 /wd4047 /O2
CFLAGS_DEBUG = $(CFLAGS_COMMON) -DTIMEIT
IFLAGS = 
# preface with /link if used
LFLAGS = 

OBJ_FILES = $(NAME).obj

RM = del /s /f

all: $(OBJ_FILES) $(NAME)_query.exe

check: test$(NAME).exe test$(NAME)_jac.exe $(NAME)_query.exe
	test$(NAME).exe
	test$(NAME)_jac.exe
	$(NAME)_query.exe 9 5 5

clean:
	$(RM) *.obj *.dll *.exe

.SUFFIXES: .c .obj

.c.obj:
	$(CC) $(IFLAGS) $(CFLAGS_COMMON) /c $<

$(NAME)_query.exe: $(NAME)_query.c $(OBJ_FILES)
	$(CC) $(IFLAGS) $(CFLAGS_COMMON) $(NAME)_query.c $(OBJ_FILES) /Fe$@ $(LFLAGS)

test$(NAME).exe: test$(NAME).c $(OBJ_FILES)
	$(CC) $(IFLAGS) $(CFLAGS_DEBUG) test$(NAME).c $(OBJ_FILES) /Fe$@ $(LFLAGS)

test$(NAME)_jac.exe: test$(NAME)_jac.c $(OBJ_FILES)
	$(CC) $(IFLAGS) $(CFLAGS_DEBUG) test$(NAME)_jac.c $(OBJ_FILES) /Fe$@ $(LFLAGS)
