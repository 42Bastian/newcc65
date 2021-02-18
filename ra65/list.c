
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* listing code */

#include "parse.h"
#include "global.h"
#include <stdio.h>

void   outlchar(char ch);
void outlstr(char *str);
void out2col(int col);
void outnum(int num,int base,int nplaces,char fill);
void newpage();
void list_line(int override);
void init_list();

/* defs for field positions in output line */

#define	PC_POS	7
#define VAL_POS	10
#define OBJ_POS	12
#define	N_OBJ	4
#define	LAB_POS	26
#define	OP_POS	34
#define	ARG_POS	40
#define COM_POS	50

char listbuf[132];
int listidx;
int page_line;

FILE * listf;

char hexchar[] = "0123456789ABCDEF";

/* output one listing char */
void   outlchar(char ch)
{
  listbuf[listidx++] = ch;
}

void outlstr(char *str)
{
  for ( ; *str ; )
	outlchar(*str++);
}

/* output spaces until column, at least one */
void out2col(int col)
{
  outlchar(' ');
  while(listidx < col)
	outlchar(' ');
}

void outnum(int num,int base,int nplaces,char fill)
 
{
  int i;
  char digit[10];

  for (i = 0 ; i < nplaces ; i++)
	if (num == 0)
		digit[i] = fill;
	    else
		{
		digit[i] = hexchar[num % base];
		num = num / base;
		}
  while (i > 0)
	outlchar(digit[--i]);
		
}

void new_page()
{
  listidx = 0;
  outlchar('\f');
  outlstr(page_title);
  out2col(64);
  outlstr("Page ");
  outnum(page_nbr, 10, 4, ' ');
  outlchar('\n');
  outlchar('\0');
  fputs(listbuf, listf);
  if (strlen(page_subttl) > 0)
	{
	listidx = 0;
	outlstr(page_subttl);
	outlchar('\n');
	outlchar('\0');
	fputs(listbuf, listf);
	}
  fputs("\n\n", listf);
  page_nbr++;
  line_in_page = 0;
}


void list_line(int override)
{
  int i;

  if (!(override || (list_p && (pass > 0))))
	{
	obj_count = 0;
	return;
	}
	
  if (line_in_page > 55) 
	new_page();

  listidx = 0;
  if (!line_listed_p)
	outnum(line_nbr, 10, 5, ' ');

  if (list_v_p)
	{
	out2col(VAL_POS);		/* get set to out random value */
	outnum(list_v, 16, 4, '0');
	}
    else
	{
	if ((obj_count > 0) || (list_pc_p))
		{
		out2col(PC_POS);
		outnum(list_pc, 16, 4, '0');
		}
	if (obj_count > 0)
		{
		out2col(OBJ_POS);
		for (i = 0 ; i < obj_count ; i++)
			{
			outnum(obj[i] & 0xFF, 16, 2, '0');
			outlchar(' ');
			}
		}
	}

  if(!line_listed_p)
	{
	if (p.label)
		{
		out2col(LAB_POS);
		outlstr(p.label);
		if (p.label_suffix)
			outlstr(p.label_suffix);
		}

	if (p.opcode)
		{
		out2col(OP_POS);
		outlstr(p.opcode);
		}

	if (p.arg[0])
		{
		out2col(ARG_POS);
		outlstr(p.arg[0]);
		for (i = 1 ; p.arg[i] ; i++)
			{
			outlchar(',');
			outlstr(p.arg[i]);
			}
		}

	if (p.comment)
		{
		if (p.comment_column > 0)
			out2col(COM_POS);
		    else
			out2col(LAB_POS);
		outlstr(p.comment);
		}
	}
  outlchar('\n');
  outlchar('\0');
  fputs(listbuf, listf);
  obj_count = 0;
  line_listed_p = 1;
  list_v_p = 0;
  list_pc_p = 0;
  line_in_page++;
}

void init_list()
{
  page_title[0] = '\0';
  page_subttl[0] = '\0';
  page_nbr = 1;
  if (list_name)
	listf = fopen(list_name, "w");
    else
	listf = stdout;
  line_in_page = 999;
}
