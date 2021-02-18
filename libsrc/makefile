#
# makefile for CC65 runtime library
#
CC = cc65
LINK= link65
AR= libr65
AS= ra65
CP= cp
RM = rm -f


BINDIR = ../lib

GLOBAL = ../runtime/global.m65

.SUFFIXES: .com .obj .m65 .c

.c.obj:
	$(CC) $<
	xopt $*.m65
	$(AS) -o $@ $(GLOBAL) ../runtime/labels.m65 $*.m65
	$(RM) $*.m65

.m65.obj: $(GLOBAL)
	$(AS) -o $@  $(GLOBAL) ../runtime/labels.m65 $<

C_OBJS = atoi.obj iscntrl.obj isprint.obj itox.obj atoib.obj isalnum.obj \
	iscons.obj ispunct.obj itoab.obj otoi.obj  dtoi.obj \
	isspace.obj itod.obj utoi.obj isascii.obj isgraph.obj isupper.obj\
	itoo.obj isatty.obj islower.obj isxdigit.obj itou.obj toascii.obj\
	strcat.obj

#	fputs.obj fgets.obj gets.obj fprintf.obj \
#	reverse.obj  pmalloc.obj  fclose.obj  \
#	time.obj xtoi.obj fopen.obj open.obj \
#	fread.obj fwrite.obj

M65_OBJS =isalpha.obj isodigit.obj isdigit.obj iswhite.obj \
	bzero.obj bcopy.obj strchr.obj toupper.obj strlen.obj strcpy.obj\
	strcmp.obj mul.obj div.obj 

#	itoa.obj
#	stdio.obj frename.obj fdelete.obj close.obj closeall.obj cputc.obj \
#	cgetc.obj cgets.obj read.obj write.obj rwcommon.obj tprintf.obj \
#	heap.obj getch.obj parselin.obj tolower.obj \
	
install : c.olb 

c.olb : $(C_OBJS) $(M65_OBJS)
	$(AR) a c.olb $?
	$(CP) c.olb $(BINDIR)
clean:
	$(RM) *.obj
	$(RM) *.bak
	$(RM) c.olb
