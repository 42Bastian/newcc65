#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
#include <stdio.h>
/*
   utoi -- convert unsigned decimal string to integer nbr
            returns field size, else ERR on error
*/
#define ERR -1
utoi(decstr, nbr)  
char *decstr;  
int *nbr;  
{
  int d,t; d=0;
  *nbr=0;
  while((*decstr>='0')&(*decstr<='9')) 
    {
    t=*nbr;t=(10*t) + (*decstr++ - '0');
    if ((t>=0)&(*nbr<0)) return ERR;
    d++; *nbr=t;
    }
  return d;
}
