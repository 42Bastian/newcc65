
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* error stuff */

#include <stdio.h>
#include <stdlib.h>

#include "global.h"

/* list.c */
void list_line(int override);

// macro.h
extern int processing_macro;
extern char *macroCurrName;
extern short macroCurrFile;
extern short macroCurrLine;
extern char * in_name[64];

void barf(char * msg,int arg1,int arg2,int arg3)
 {
//  list_line(1);
  errcount++;
  printf("%s:%d:",CurrentFile,line_nbr);
  if ( processing_macro )
    printf("macro(%s,%s,%d):",in_name[macroCurrFile],macroCurrName,macroCurrLine);
  
  printf("Error:");
  printf(msg, arg1, arg2, arg3);
  printf("\n");
}

void fatal(char *msg)
{
  fprintf(stderr,msg);
  exit(-1);
}
