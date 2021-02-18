#include <stdlib.h>
#define NOARGC  /* no argument count passing */
#define FIXARGC /* don't expect arg counts passed in */

/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/*
   atoi(s) - convert s to integer.
*/
atoi(s) 
char *s; 
{
  int sign, n;
  while(isspace(*s)) ++s;
  sign = 1;
  switch(*s) 
    {
    case '-': sign = -1;
    case '+': ++s;
    }
  n = 0;
  while(isdigit(*s)) 
	n = 10 * n + *s++ - '0';
  return (sign * n);
}
