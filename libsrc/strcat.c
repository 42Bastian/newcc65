#define FIXARGC
#define NOARGC
#include <stdlib.h>

/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* somebody ought to rewrite this in m65 code */
strcat(str1, str2)
char * str1;
char * str2;
{
  strcpy(str1 + strlen(str1), str2);
}
