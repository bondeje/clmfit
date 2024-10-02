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

all: $(OBJ_FILES)

check: test$(NAME).exe test$(NAME)_jac.exe
	test$(NAME).exe
	test$(NAME)_jac.exe

clean:
	$(RM) *.obj *.dll *.exe

.SUFFIXES: .c .obj

.c.obj:
	$(CC) $(IFLAGS) $(CFLAGS_COMMON) /c $<

test$(NAME).exe: test$(NAME).c $(OBJ_FILES)
	$(CC) $(IFLAGS) $(CFLAGS_DEBUG) test$(NAME).c $(OBJ_FILES) /Fe$@ $(LFLAGS)

test$(NAME)_jac.exe: test$(NAME)_jac.c $(OBJ_FILES)
	$(CC) $(IFLAGS) $(CFLAGS_DEBUG) test$(NAME)_jac.c $(OBJ_FILES) /Fe$@ $(LFLAGS)
