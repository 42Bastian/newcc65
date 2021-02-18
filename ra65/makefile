# Makefile for a65 && link65 (and others)  -Intruder  Oct 1993
#
# intruder@hacktic.nl

CFLAGS = -pipe -O3 -m486 -fomit-frame-pointer -Wall -DUNIX
CC=gcc
LDFLAGS= -s
BINDIR=../bin
CP = cp 
RM = rm -f
EXE = #.exe
OBJS = a65.o symtab.o parse.o opcode.o util.o list.o xpseudo.o \
                error.o eval.o xgen.o copyleft.o macro.o

EXECS = ra65$(EXE) printobj$(EXE) link65$(EXE) libr65$(EXE) bin2obj$(EXE)


all:	$(EXECS)

ra65$(EXE) :   $(OBJS) version.o makefile 
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS) version.o

version.o: $(OBJS) makefile
$(OBJS) : global.h
.c.o:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -c $<

printobj$(EXE) : printobj.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ printobj.c

link65$(EXE) : link65.o obj.h copyleft.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ link65.o copyleft.o

libr65$(EXE) : libr65.c libr.h obj.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ libr65.c

bin2obj$(EXE): bin2obj.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ bin2obj.c

install : $(EXECS)
	$(CP) $? $(BINDIR)

clean:
	$(RM) *.o core $(EXECS) compile.err errlist *.bak
