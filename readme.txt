Note :
This version has been edited to fit to the new cc65.
The original version by Kurt Olsen is readme.old.
42Bastian

This is no native-compiler anymore !!!
The source has be changed to compile with gcc (or DJGPP under DOS).

There are a few things that you should know:

- the compiled programs use some zero page space, right around $80-$95.
  see global.m65.

- ints are 16 bit, so are unsigned ints.

- chars are 8 bit, and signed !!

- structs and unions hide there members.

- function definitions are K&R, not ANSI.

- register variables are emulated in the zero-page.

- the default stack is 4096 bytes.  you can change it with the -s flag
  to the linker.  like this:

     link65 -b200 -s512 myprogram.obj

  this results in a program that loads at $200 and has a stack of 512 bytes.
  notice that the -b takes hexdecimal numbers and -s takes decimal.  sorry.

- the stack starts right above the executable program.  so if you underflow
  the stack you will be stomping on your program.  you've been warned.

- it's sort of slow. Use xopt to make it a bit faster and a lot smaller.

- currently there isn't any floating point support or long integer
  support.

- don't even think about using any text output functions.

- or file handling stuff.

- look in examples.c65 to see some (simple/bad) examples of what you can
  do with this thing.  if you don't have gmake, well I might be
  persuaded to send you a copy.
  
- I've been using the extensions:

  .c   - c files
  .m65 - machine code files
  .obj - object files
  .com - final linked programs  (don't try to run these under dos)
  
- the .com files have a 10 byte BLL header:

  $80 $08 - a magic number to indicate BLL file
  $hh $ll - program starts at $hhll in memory
  $hh $ll - program is $hhll bytes long
  $dd $dd - reserved
  $dd $dd - reserved
  
- there's no argc or argv or environment, so don't try any funny stuff

- the C library is rather useless to us on the Lynx, since it all
  expects to be used on the 8 bit Atari home compuers (800, 400, XL, etc.)
  Therefor the functions in c.olb have been reduced, though the old sources
  are still there.

- symbol names are 32 chars

- if it totally sucks for you, sorry.

Well, that's all I can think of right now.  If you figure out why it
does not properly handle structs and unions I would be glad to know.
If you can think of some routines that you would like to see as part
of a Lynx specific library, let me know.

Don't forget that you can set CC65LIB to point to the location of your
runtime.run, and c.olb files.  Otherwise they need to be
in your current directory.

Also don't forget to set CC65INCLUDE !

The original documentation is in docs/ where you can see what's
what.

Edited : January 24,1998 42Bastian Schick

Original readme:

Kurt Olsen
June 23, 1996.
