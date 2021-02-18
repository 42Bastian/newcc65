#define NOARGC  /* no argument count passing */
#define FIXARGC /* don't expect arg counts passed in */
#include <stdio.h>

/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/*
   Open file indicated by fn.
   Entry: fn   = ASCIIZ file name.
                 May be prefixed by letter of drive.
          mode = "a"  - append
                 "r"  - read
                 "w"  - write
                 "u"  - update
   (some of those might not work on atari -- jrd)
   Returns a file descriptor on success, else NULL.
*/
FILE * fopen(fn, mode)
char * fn, * mode;
{
  int iocb;
  
  iocb = copen(fn, *mode);
  if (iocb < 0)
	{
	errno = iocb;
	return(0);
	}
  return(iocb);		/* just return it as a (FILE * ) */
}

