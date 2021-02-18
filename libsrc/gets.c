#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */

/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/*
   Gets an entire string from stdin (excluding its newline
   terminator) or size-1 characters, whichever comes
   first. The input is terminated by a null character.
   The user buffer must be large enough to hold the data.
   Entry: str  = Pointer to destination buffer.
   Returns str on success, else NULL.
*/
#include <stdio.h>
gets(str)
char *str; 
{
  return (fgets(str, 32767, stdin));
}

