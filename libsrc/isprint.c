#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
/*
   return 'true' if c is a printable character
   (32-126)
*/
isprint(c) 
int c; 
{
  return (c>=32 && c<=126);
}
