#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
/*
   return 'true' if c is upper-case alphabetic
*/
isupper(c) 
int c; 
{
  return (c<='Z' && c>='A');
}
