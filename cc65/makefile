#
# Makefile for cross-compiler version of CC65.
# (Modified for use on *nix machines -Intruder  1993)
#
# modified : 16.06.96 B.Schick
#            97/11/25 B.Schick

CC=gcc
CFLAGS = -m486 -O3 -Wall -ffast-math -fomit-frame-pointer
LDFLAGS= -s
BINDIR = ~/bin
CP = cp 
RM = rm -f
EXT = #.exe

C_SRCS = error.c expr1.c expr2.c expr3.c function.c glb.c \
	globlvar.c io.c lexer.c main.c \
	preproc.c rwords.c stmt1.c stmt2.c symtab.c path.c copyleft.c

# the optimizer stuff is removed from cc65 !! 42BS
        # opttab1.c optab2.c optimize.c

H_SRCS = cc65.h cclex.h

OBJS =  error.o expr1.o expr2.o expr3.o function.o glb.o globlvar.o \
	io.o lexer.o main.o preproc.o rwords.o\
	stmt1.o stmt2.o symtab.o path.o copyleft.o cruft.o

# the optimizer stuff is removed from cc65 !! 42BS
        #optab1.o optab2.o optimize.o

XOBJS = version.o

EXECS = cc65$(EXT) xopt$(EXT)

all : $(EXECS)

cc65$(EXT) : $(OBJS) $(XOBJS) code-65.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS) $(XOBJS) code-65.o

version.o : $(OBJS) code-65.o

$(OBJS) : $(H_SRCS)

$(XOBJS) : $(H_SRCS)

xopt$(EXT) : xopt.cc
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ xopt.cc

install : $(EXECS)
	$(CP) $(EXECS) $(BINDIR)

clean : 
	$(RM) *.o core $(EXECS) compile.err errlist *.bak

