
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/


/* open kludge */

#include <file.h>

int open(name, mode)
char * name;
int mode;
{
  char copen_mode;

  if (mode & O_RDONLY)
	copen_mode = 'r';
    else
  if (mode & O_WRONLY)
	copen_mode = 'w';
    else
	/* error */
	return(-1);
  return(copen(name, copen_mode));
}

	