#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
/*
   return 'true' if c is an ASCII character (0-127)
*/
isascii(c) 
char * c; 
{
  /* c is a simulated unsigned integer */
  return (c <= 127);
}
