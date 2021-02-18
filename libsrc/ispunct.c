#include <stdlib.h>
#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
/*
   return 'true' if c is a punctuation character
   (all but control and alphanumeric)
*/
ispunct(c) 
int c; 
{
  return (!isalnum(c) && !iscntrl(c));
}
