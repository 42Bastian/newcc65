#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */

/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

#include <stdio.h>
/*
   Write a string to fd. 
   Entry: string = Pointer to null-terminated string.
          fd     = File descriptor of pertinent file.
*/
fputs(string,fd) char *string; int fd; 
{
/* we can do better than this ...
  while(*string)
	if(fputc(*string++,fd)==EOF) return(EOF);
*/
  write(fd, string, strlen(string));
  return(0);
}

