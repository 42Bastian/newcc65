#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
/*
   return 'true' if c is alphanumeric
*/
isalnum(c) int c; 
{
  return ((c<='z' && c>='a') ||
          (c<='Z' && c>='A') ||
          (c<='9' && c>='0'));
}
