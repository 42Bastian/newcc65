
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/
/* 42BS	added '*' as comment-symbol (first row only */

/* the input line parser */
#include <stdio.h>

#include "parse.h"

extern short asm_var;

void barf(char * msg,int arg1,int arg2,int arg3);

char label_buf[32];		/* buf for labels */
char suffix_buf[8];		/* buf for suffixes */
char op_buf[32];		/* char for ops and pseudos */

int id_char_p(char c);
int id_start_p(char c);
int special_p(char c);
int white_p(char c);
int find_string_end(char * s,int i,char c);
int find_char(char * s,int  i,char c);
void parse_line(char * line,struct parse * p,int macrodef);


int id_char_p(char c)
{
  return(((c >= 'a') && (c <= 'z')) ||
	 ((c >= 'A') && (c <= 'Z')) ||
	 ((c >= '0') && (c <= '9')) ||
	 (c == '_') || (c == '.') || ( c == '@')
	 );
}

int id_start_p(char c)
{
  return(((c >= 'a') && (c <= 'z')) ||
	 ((c >= 'A') && (c <= 'Z')) ||
	 (c == '_') || (c == '.')
	 );
}

int special_p(char c)
{
  return((c == '=') || (c == '*'));
}


int white_p(char c)
{
  return((c == ' ') || (c == '\t'));
}

int find_string_end(char * s,int i,char c)
{
  char cc;

  if (s[i] == c)
	i++;
  for ( ; ((cc = s[i]) && cc != c) ; i++)
	;
  return(i);
}

int find_char(char * s,int  i,char c)
{
  char cc;

  for ( ; ((cc = s[i]) && (cc != c)) ; i++)
	;
  return(c);
}

void parse_line(char * line,struct parse *  p,int macrodef)
{
  int i, j, len;
  char c;
  char * ptr;

/* first clear the whole thing */
  p->label = p->label_suffix = p->opcode = p->comment = (char *)0;
  p->comment_column = 0;
  for (i = 0 ; i < 32 ; i++)
	  p->arg[i] = 0;

  if ( !(len = strlen(line)) ) return;

  i = 0;


  if (line[0] == ';' 
   || line[0] == '*' /*42BS*/
   || line[0] == '#' /*42BS*/)		/* comment only? */
	{
	p->comment = line;
	p->comment_column = 0;
	return;
	}
 
  if ( !white_p(line[0]) )
{	
/* parse a label */
	if (id_start_p(line[0]) || (macrodef && line[0] == '\\') )
		{
		p->label = ptr = label_buf;
		while ((id_char_p(line[i])|| (macrodef && line[0] == '\\') ) && (i < len))
			*ptr++ = line[i++];
		*ptr = '\0';
/* collect the label suffix if any */
		if ((i < len) && (!white_p(line[i])))
			{
			p->label_suffix = ptr = suffix_buf;
			while ((!white_p(c = line[i])) && 
				(!id_char_p(c)) &&
				(!special_p(c)) &&
				(i < len))
		  {
				  *ptr++ = line[i++];
		  }
			*ptr = '\0';
			}
		}
	    else
		barf("illegal label char",0,0,0);
	};
 
/* done with label if any, find op */
  while ((i < len) && white_p(line[i]))
	  i++;
  if (i >= len)
	  return;
  if (line[i] == ';')
	  {
	  p->comment = line + i;
	  p->comment_column = i;
	  return;
	  }
	  
  p->opcode = ptr = op_buf;

/* "*="? */
  if ((line[i] == '*') && (line[i+1] == '='))
	  {
	  *ptr++ = line[i++];
	  *ptr++ = line[i++];
	  *ptr = '\0';
	  }
  else

/* "="? */
  if (line[i] == '=')
	  {
	  *ptr++ = line[i++];
	  *ptr = '\0';
	  }
  else
  if (id_start_p(line[i]))
	  {
	  *ptr++ = line[i++];
	  while (id_char_p(line[i]))
		  *ptr++ = line[i++];
	  *ptr = '\0';		/* terminate the op string */
	}

/* look for args */
  while ((i < len) && (white_p(c = line[i])))
	  i++;
	
  if (i >= len)
	 {
	 return;
	 }

  if (c == ';')
	{
	  p->comment = line + i;
	  p->comment_column = i;
  	return;
	}
    else
	{
	for (j = 0 ; j < 32 ; j++)
		{
		p->arg[j] = line + i;
		c = line[i];
		switch (c)
			{
			case '"':
				{
				i = 1 + find_string_end(line, i, c);
				break;
				}
			case '\'':
				{
				i += 2;		/* skip the quote and char */
				if (line[i] == '\'')
					i++;
				break;
				}
			default:
				{
				while ((c = line[i]) && (c != ';') &&
					(c != ',') && (!white_p(c)))
					i++;
				}
			}
		c = line[i];		/* get char that term'ed us */
		line[i] = '\0';		/* end this str */
		i++;			/* next char */
		if (c == ',')		/* another fld? */
			for ( ; ((c = line[i]) && white_p(c)) ; i++)
				;
		if ((i >= len) || (c == ';') || white_p(c))	/* done? */
			break;
		}			/* loop collecting flds... */
	}				/* found all flds... */	

/* now if anything left, it's a comment */
  if (i >= len)
	return;
  for ( ; ((c = line[i]) && (white_p(c))) ; i++)
	;
  if (i >= len)
	return;
  if (c)
	{
	p->comment = line + i;
	p->comment_column = i;
	}

  return;
}
	
