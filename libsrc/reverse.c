#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
/*
** reverse string in place 
*/
reverse(s) 
char *s; 
{
  char *j;
  int c;
  j = s + strlen(s) - 1;
  while(s < j) 
    {
    c = *s;
    *s++ = *j;
    *j-- = c;
    }
}

