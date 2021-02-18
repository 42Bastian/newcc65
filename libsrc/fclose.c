#define NOARGC  /* no argument count passing */
#define FIXARGC /* don't expect arg counts passed in */
#include <stdio.h>

/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/*
   Close fd 
   Entry: fd = File descriptor for file to be closed.
   Returns NULL for success, otherwise ERR
*/
fclose(fd)
int fd; 
{
/*
  if (close(fd) >= 0)
    return(NULL);
   else
    return(ERR);
 ... we can do better than that... */

  return(close(fd) >= 0 ? NULL : ERR);
}

