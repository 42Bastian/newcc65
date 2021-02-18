// this file is compiled whenever an object file is changed
#include <stdio.h>

#define VERSION "2.1.3"

void version(void)
{
  printf("cc65 v "VERSION" [*nix version] (compiled:%s)\n",__DATE__);
}

