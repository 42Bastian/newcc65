
/*
   Misc cruft used by cross-compiler variant of CC65
*/

#include <stdio.h>
#include <string.h>

#include "cc65.h"
#include "cclex.h"

/* internal prototypes */
FILE * copen(char * file,char mode);
int cclose(FILE * x);
void cprints(char * s);
void cputc(int c,FILE * fp);
void clear(char * ptr,int len);
int is_alpha(int c);
int is_digit(int c);
int tolower(int c);
int toupper(int c);
int is_power_of_two(int n);

int matchstr(char * line,char * str); // this is move from optimize.c !

/* return t if if line matches str */
 
int matchstr(char * line,char * str)
{
  for(; *str; ++line, ++str)
    if (*line != *str)
      return (0);
  return (1);
}

/*
bzero(char *dest, int len)
{
memset(dest, 0, len);
}
*/

FILE * copen(char * file,char mode)
{
char m[5];
FILE * p;

  m[0] = mode;
  m[1] = '\0';
  if ((p = fopen(file, m)) == NULL)
    {
      return ((FILE *) 0);	/* was -1 -- jrd */
    }
  else
    {
      return (p);
    }
}

int cclose(FILE * x)
{
  return(fclose(x));
}

void cprints(char * s)
{
  fputs(s, stdout);
}

void cputc(int c,FILE * fp)
{
  fputc(c, fp);
}

void clear(char * ptr,int len)
{
  while (--len >= 0)
    *ptr++ = '\0';
}

int is_alpha(int c)
{
  if (((c >= 'a') && (c <= 'z')) ||
      ((c >= 'A') && (c <= 'Z')) ||
      (c == '_'))
    return(1);
  else
    return(0);
}

int is_digit(int c)
{
  if ((c >= '0') && (c <= '9'))
    return(1);
  else
    return(0);
}

int tolower(int c)
 
{
  if ((c >= 'A') && (c <= 'Z'))
    return(c + 32);
  else
    return(c);
}

int toupper(int c)
{
  if ((c >= 'a') && (c <= 'z'))
    return(c - 32);
  else
    return(c);
}

int is_power_of_two(int n)
{
int c = 0;
  if ( n == 0)
    return 0;
    
  while ( (n & 1) == 0 )
    {
    n >>= 1;
    ++c;
    }
  if ( n == 1)
    return c;
  else
    return 0;
}

