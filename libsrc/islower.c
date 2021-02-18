#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
/*
   return 'true' if c is lower-case alphabetic
*/
islower(c) 
int c; 
{
  return (c<='z' && c>='a');
}

