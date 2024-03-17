
CC = gcc
OFILES = mpfit.o
LIBFILE = libmpfit.a
CFLAGS = -Wall -Werror -Wextra -pedantic -Wno-unused -DTIMEIT
EXEC = 

ifeq ($(OS), Windows_NT)
	RM = del /s /f
	RM_SUFFIX = 


# This is more of a shell function requirement than an OS
else
	RM = find . -type f -name
	RM_SUFFIX = -delete
	EXEC = ./
endif

all: $(LIBFILE) testmpfit testmpfit_jac test

test:
	$(EXEC)testmpfit
	$(EXEC)testmpfit_jac

clean:
	$(RM) $(OFILES) $(RM_SUFFIX)
	$(RM) "*.exe" $(RM_SUFFIX)
	$(RM) "*.a" $(RM_SUFFIX)
#	$(RM) "*.dll" $(RM_SUFFIX)

mpfit.o: clmfit.c mpfit.h
	$(CC) $(CFLAGS) -c -o $@ $< 

$(LIBFILE): $(OFILES)
	$(AR) r $@ $(OFILES)

testmpfit: testmpfit.c libmpfit.a
	$(CC) $(CFLAGS) testmpfit.c -o $@ -L. -lmpfit -lm

testmpfit_jac: testmpfit_jac.c libmpfit.a
	$(CC) $(CFLAGS) testmpfit_jac.c -o $@ -L. -lmpfit -lm
