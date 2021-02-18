#define NOARGC  /* no argument count passing */
#define FIXARGC /* don't expect arg counts passed in */
#include <stdio.h>

/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/*
   Gets an entire string (including its newline
   terminator) or size-1 characters, whichever comes
   first. The input is terminated by a null character.
   Entry: str  = Pointer to destination buffer.
          size = Size of the destination buffer.
          fd   = File descriptor of pertinent file.
   Returns str on success, else NULL.
*/
fgets(str, size, fd)
char *	str;
int size, fd;
{
  int ch;
  char * retval;

  retval = NULL;		/* default */
  for ( ; --size > 0 ; )
	{
	ch = fgetc(fd);		/* get one */
	if (ch == EOF)
		break;
	if (!retval)
		retval = str;
	*str++ = ch;		/* store it */
	if (ch == '\n')
		break;		/* stop at EOL */
	}
  *str = '\0';
  return(retval);
}

