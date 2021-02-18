
/*
   Error reporting code
*/

#include <stdio.h>

#include "cclex.h"
#include "cc65.h"
#include "error.h"
/*
  Error( msg )
  Print error with no prefix.
*/
void Error(char * msg)
 
{
  PError("", msg);
}

/*
  Syntax()
  Flag a syntax error.
*/
void Syntax()
{
  Error("syntax error");
}

/*
  Illegal( msg )
  Handle "illegal ..." messages.
*/
void Illegal(char *msg)
 
{
  PError("illegal ", msg);
}

/*
  Missing( msg )
  Handle "missing ..." messages.
*/
void Missing(char * msg)
{
  PError("missing ", msg);
}

/*
  MultDef( msg )
  Handle "multipy defined ..." messages.
*/
void MultDef(char * msg)
{
  PError("multiply defined ", msg);
}

/*
  Need( msg )
  Handle "need ..." messages.
*/
void Need(char * msg)
{
  PError("need ", msg);
}

/*
  needlval()
  Handle "need lvalue" messages.
*/
void needlval()
{
  Need("lvalue");
}

/* from here on down, we want to generate argcounts, so we undefine
   the magic symbol */

#undef NOARGC

/*
  ersum()
  Report errors for user
*/
void ersum()
{
  /* see if anything left hanging... */
  if (ncmp)
    Missing("closing bracket");
  if (errcnt == 0 && verbose)
    {
      printf("No errors.\n");
    }
}

/*
  PError( pfx, msg )
  Print prefix message pfx followed by error message msg.
*/
void PError(char *pfx,char * msg)
{
  printf("%s, line %d : %s%s\n", fin, ln, pfx, msg);
  if (verbose)
  {
    printf("line: %s\n", line);
    printf("curtok = %d\n", curtok);
    printf("nxttok = %d\n", nxttok);
  }
  
  if (++errcnt > 6)
    fatal("too many errors");
}

/*
  fatal( msg )
  Fatal error - print message and die
*/
void fatal(char * msg)
{
  printf("fatal error: %s\n", msg);
  printf("line: %s\n", line);
  printf("curtok = %d\n", curtok);
  printf("nxttok = %d\n", nxttok);
  printf("Can't recover from previous errors:  Good-bye!\n");
  exit(2);
}

/*
  Warning( msg )
  Print warning message.
*/
void Warning(char * msg)
{
  printf("warning: %s\n", msg);
  printf("line: %s\n", line);
  printf("curtok = %d\n", curtok);
  printf("nxttok = %d\n", nxttok);
}
