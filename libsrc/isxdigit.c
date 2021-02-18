#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
/*
   return 'true' if c is a hexadecimal digit
   (0-9, A-F, or a-f)
*/
isxdigit(c) 
int c; 
{
  return ((c<='f' && c>='a') ||
          (c<='F' && c>='A') ||
          (c<='9' && c>='0'));
}
