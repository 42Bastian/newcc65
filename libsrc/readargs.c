#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */

/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* handy util for 8-bitters.  handed a prompt string, prompt with it,
   and read an arg string.  parse it into the passed argv */

#include <stdio.h>

int readargs(prompt, buf, argv)
char * prompt;
char * buf;
char ** argv;
{
  fputs(prompt, stderr);	/* prompt */
  fgets(buf, 80, stderr);	/* get a buf */
  return(_parseline(buf, argv));
}
