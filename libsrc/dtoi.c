#define NOARGC  /* no argument count passing */
#define FIXARGC /* don't expect arg counts passed in */

#include <stdlib.h>
#include <stdio.h>
/*
   dtoi -- convert signed decimal string to integer nbr
           returns field length, else ERR on error
*/
dtoi(decstr, nbr)
char *decstr;
int *nbr;
{
  int len, s;
  if((*decstr)=='-') {s=1; ++decstr;} else s=0;
  if((len=utoi(decstr, nbr))<0) return ERR;
  if(*nbr<0) return ERR;
  if(s) {*nbr = -*nbr; return ++len;} else return len;
}
