// this file is compiled whenever an object file is changed
#include <stdio.h>


void version(char *p,char *v)
{
  printf("%s v %s [*nix version] (compiled:%s)\n",p,v,__DATE__);
}

