-*- Text -*-

This is the second betatest distribution of CC65 and associated stuff.
Please see INTRO.DOC for instructions on getting started.

Since the last go-round, the following bugs have been fixed:

o  The low-level file-open code now upcases the filename before
   attempting to open it.  This should fix the lossage observed
   on systems like DOS 2.5 that treat lowercase characters as
   illegal.

o  The command-line parser should now be able to recognize DOS XL
   command lines, as well as SpartaDos.

As before, all executables have their base address set to #x2000, with
the exeception of CC65.COM, which starts at #x2600.  Please let me
know if you run into memory problems; I suspect I can get another half
K out of CC65 before running into stack problems. 

All text files are in ATASCII except the ones in DOC.ARC, which are in
mushy-dos format, ie CRLF as line delimiters. 

Since I've been remiss in writing the doc for the library routines
etc, I've included the library source.  You shouldn't have to rebuild
it, but it may be useful for reference.  If anyone feels inclined to
write part of the doc for me, don't let me stop you!  BTW when I built
LIBSRC.ARC I wasn't very selective, I just grabbed eveything out of
my current development directory that looked relevant.  There are
probably some duplicates and extra files in there, esp since many of
the routines were originally in C, then later hand-coded in assembler.
Caveat emptor.

Please let me know how this stuff works for you.  I'd like to declare
it done and post it for real.

