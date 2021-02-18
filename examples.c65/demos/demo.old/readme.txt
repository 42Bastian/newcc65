Here are some short example programs for the CC65 C compiler, as modified
for the Atari Lynx.

If you are using Bastian's loader program, you might occasionally get
some strange behavior from the maze programs and from the sprite
program.  This is because Suzy can get interrupted and not complete
drawing some text in the loader program.  Currently things aren't
properly restored before the program starts so the RNG will behave
strangely.  Sorry.

If you have Watcom C, the makefile should produce all the demo
programs for you.  If you don't, sorry, you'll have to patch it
for use with whatever make you do have.
