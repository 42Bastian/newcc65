
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* misc utils for assembler */

#include <stdio.h>
#include <ctype.h>

/*
char char_upcase(c)
char c;
{
  if ((c >= 'a') && (c <= 'z'))
	c ^= 0x20;
  return(c);
}
*/

int string_equal(s1, s2)
char * s1, * s2;
{
  if (strlen(s1) != strlen(s2))
	return(0);
  for ( ; *s1 && *s2 ; ++s1, ++s2)
	if (toupper(*s1) != toupper(*s2))
		return(0);
  return(1);
}

int read_line(f, l)
FILE * f;
char * l;
{
  int c;
  int ok;

  ok = 0;
  for ( ; ((c = fgetc(f)) != EOF) ; )
	{
/* printf("read-char '%c' %X ptr %X\n", c, c, l); */
	ok = 1;
	if (c == '\n')
		break;
	if (!((c == '\r') || (c == '\f')))	/* ^L */
	 *l++ = c;
	}
  *l = '\0';
  return(ok);
}

char * frob_name(char *name)
{
  return(name);
}


int filepos;
void save_pos(FILE *f)
{
  filepos = ftell(f);
}

void restore_pos(FILE *f)
{
  fseek(f,filepos,SEEK_SET);
}
