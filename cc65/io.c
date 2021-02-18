
/*	C I/O functions	*/

#include <stdio.h>
 
/* every time I thing some C weenie someplace might have opted to do
   something in a regular fashion, I get disappointed... */
#include <errno.h>
 
#include "cc65.h"
#include "cclex.h"
#include "error.h"
#include "code-65.h"

int alpha(int c);
int numeric(int c);
char nch();
char cgch();
char gch();
void do_kill();
int readline();
void flushout();
void outchar(char c);
void cout(char ch);
void sout(char *s); 
 
/* external prototypes */
/* cruft.c*/
int cclose(FILE * x);
 
int outcnt = 0;
 

/* not used?
char	ascii_tab[128] =
    {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0
    };
*/

/*
  alpha( c )
  Test if given character is alpha.
*/
/* not used...
int alpha(int c)
{
    return( ascii_tab[c] & 1 );
}
*/

/*
  numeric( c )
  Test if given character is numeric
*/
/* not used
int numeric(int c)
{
    return( ascii_tab[c] & 2 );
}
*/

/*
  an( c )
  Test if given character is alphanumeric
*/
/* not used
an(c)
int	c;
{
    return( ascii_tab[c] );
}
*/

/*
  pl( str )
  Print a string followed by a carriage return.
*/
/* not used?
pl(str)
char * str;
{
    printf("%s\n", str);
}
*/

/* apparently, all callers of this have been redefined
   with defines....
ch()
{
    return( *lptr & 0xFF );
}
*/


char nch()
{
  if (*lptr == '\0')
    {
      return (0);
    }
  else
    {
       return (lptr[1] & 0xFF);
 
    }
}
/*
  cgch()
  Get the current character in the input stream and advance line
  pointer (unless already at end of line).
*/
 
char cgch()
{
  if (*lptr == '\0')
    {
      return (0);
    }
  else
    {
      return (*lptr++ & 0xFF);

    }
}

 

/*
  gch()
  Get the current character in the input stream and advance line
  pointer (no end of line check is performed).
*/
 
char gch()
{
 
  return (*lptr++ & 0xFF);
 
}

void do_kill()
{
  lptr = line;
  *lptr = '\0';
}

/*
  readline()
  Get a line from the current input.  Returns -1 on end of file.
*/
int readline()
{
int k;
int len;
struct filent * pftab;

  while (1)
  {
    do_kill();
    if (inp == 0)
	  {
	    eof = 1;
	    return (0);
	  }
    len = 0;
    for(;;)
    {
      k = (int) fgets(line+len, linesize-len, inp);
      len = strlen(line);

      ++ln;
      if (k <= 0) /* eof? */
	    {
	      line[len] = '\0';
	      cclose(inp);
/*	    fclose(inp);	*/

	      if (ifile > 0)
	      {
	        inp = (pftab = &filetab[--ifile])->f_iocb;
	        ln = pftab->f_ln;
	        fin = pftab->f_name;
	      }
	      else
	      {
	        inp = 0;
	      }
	    }
      else
	    {
	      line[len - 1] = '\0';
 
	      if (source && (strlen(line) > 0))
	      {
	        ot(";");
	        ol(line);
	      }
	    }
      if (line[len-2]=='\\')
        len -=2;
      else
        break;
    }
    
    if (len)
	  {
	    lptr = line;
	    return (1);
	  }
  }
}

/*
  flushout()
  Flush output queue
*/
void flushout()
{
  *outqi = '\0';
 // peephole(outq); // optimizer now extern : xopt !!
  sout(outq);
  outqi = outq;
}

/*
  Output char c to assembler file.  Really just goes out to
  buffer, so the optimizer can work on it.
*/
 
void outchar(register char c)
{
  ++outcnt;
  *outqi++ = c;
}


/* out char to real output file */
void cout(register char ch)
{
  errno = 0;  
  fputc(ch, output);
  if (errno != 0)
    {
      printmsg("IO error #x%x\n", errno);
 
      exit(1);
    }
}

/* out string to real output file */
 
void sout(register char *s)
{
  while (*s)
    cout(*s++);
}

