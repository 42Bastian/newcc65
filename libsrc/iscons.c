#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
#include <stdio.h>
/*
   Determine if fd is the console.
*/
iscons(fd) 
int fd; 
{
  return(fd == 0);		/* kludge.  need a way to tell what iocb is open on */
}
