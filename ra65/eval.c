
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* expression evaluator */
/* redesigned in Feb. 1997 42BS 
   now, an expression must be real math., so no --1 alowed, write
  -(-1) !
*/
#include <ctype.h>

#include "global.h"
#include "symtab.h"
#include "eval.h"

 

/* error.c */
void barf(char * msg,int arg1,int arg2,int arg3);

/* parse.c */
int  white_p(char c);
int  id_char_p(char c);
int  id_start_p(char c);


/************************/

int digit(char c)
{

  if ((c >= '0') && (c <= '9'))
	return(c - '0');
    else
  if ((c >= 'A') && (c <= 'F'))
	return(c - 'A' + 10);
    else
	return(-1);
}

char * grok_fixnum(char * str,int base,int *result)
 
{
  int d;
  char * s;

  s = str;
  *result = 0;

  while (((d = digit(toupper(*s))) >= 0) && (d <= base))
	{
	  *result = *result * base + d;
	  s++;
	}
	
  return(s);
}
  
/* eval one q, return updated string ptr */
char * eval_q(str, val, expr_flags, sym_ref)
char * str;
int * val;
int * expr_flags;
SYM ** sym_ref;
{
  char c;
  char * p;
  SYM * sy;
  static char tbuf[80];
  int uni = 0;
 
   while (white_p(*str)) 
     str++; 
/* check for unary minus,not,HI,LOW */ /* 42BS */

   switch (*str)
   {
     case '-' : uni = 1; str++; break;
     case '~' : uni = 2; str++; break;
     case '<' : uni = 3; str++; break;
     case '>' : uni = 4; str++; break;
   }	
	
  while((c = *str) && white_p(c))
    str++;
	
  if (id_start_p(c))
  {
/* symbol */
     p = tbuf;
     *p++ = *str++;
     while( id_char_p(*str) )
       *p++ = *str++;

    *p = '\0';
		if ((tbuf[0] == '.') && (tbuf[1] == '\0'))
			*val = pc;
	  else
		{
		  sy = find_sym(tbuf, 1);

      *val = 0;
      
		  while ( sy->flags & RELATIVE )
      {
        *val += sy->value;
        sy = sy->rel;
      }
		
		  if ((!(sy->flags & DEFINED)) && (pass == 1) && !(sy->flags & GLOBAL))
			  barf("undefined symbol '%s'", (int)tbuf,0,0);

      if ( *expr_flags & E_REL )
      {
        barf("Expression with >1 rel. symbols !",0,0,0);
      }
      else
      {
		    if (!(sy->flags & DEFINED))
			    *expr_flags |= E_UNDEF;
			  
		    if (!(sy->flags & ABS))
			    *expr_flags |= E_REL;
			    
		    if ( sy->flags & SYMBSSZP )
		      *expr_flags |= E_LO_BYTE;
      }

		  *sym_ref = sy;
		  *val = sy->value;
//		printf("eval:(%s,%x)%x %d\n",sy->name,sy->flags,*expr_flags,*val);
		
	  }
	}
  else if (c == '$')	                     /* hex constant */
		str = grok_fixnum(str + 1, 16, val);
		
  else if (c == '0')	                     /* octal constant */
		str = grok_fixnum(str, 8, val);
		
  else if (c == '%')		                    /* binary constant */
		str = grok_fixnum(str + 1, 2, val);
			
	else if (((c > '0') && (c <= '9')))	    /* decimal constant */
		str = grok_fixnum(str, 10, val);

	else if ( c == '@' )
	  *val = asm_var;
		
  else if (c == '\'')
	{
		*val = str[1];
		str += 2;
	}
  else if (c == '*')
	{
		*val = pc;
		str++;
	}
  else if (c == '(')
  {
		str = eval_internal(str + 1, val, expr_flags, sym_ref);
		while (white_p(*str))
		  str++;
		if ( *str != ')' )
		  barf("eval error: missing ')'",0,0,0);
		str++;
	}
  else
  	barf("eval error: bogus operand '%c'", c,0,0);
  	
  if ( uni && uni < 3 && *sym_ref && ( (*sym_ref)->flags & XREF) )
    barf("Only '+' and '-' allowed with undefined/external symbols",0,0,0);
    
  switch (uni)
  {
		case 1 : *val = -(*val); break;
		case 2 : *val = ~(*val); break;
		case 3 : (*expr_flags) |= E_LO_BYTE;
			       *val &= 0xFF;
			       break;
		case 4 : (*expr_flags) |= E_HI_BYTE;
             *val >>= 8;
			        break;
			       
	}
  return(str);
}

char * eval_product(char *str, int *val,int *expr_flags,SYM **sym_ref)
{
 int accum = 0,dummy = 0;
 char op;

	/* get value */
	str = eval_q(str, &accum, expr_flags, sym_ref);
    
	while (white_p(*str))
		str++;
	while ( (*str == '*') || (*str == '/') )
	{
		op = *str;
		str++;
		str = eval_q(str, &dummy, expr_flags, sym_ref);

		if (*sym_ref &&  ((*sym_ref)->flags & XREF))
		  barf("Only '+' and '-' allowed with undefined/external symbols",0,0,0);

		if ( op == '*' )
		  accum *= dummy;
		if ( op == '/' )
		  if ( val )
		    accum /= dummy;
		  else
		    barf("Division by Zero",0,0,0);

	  while (white_p(*str))
		  str++;
	}
	*val = accum;
	return (str);
}

char * eval_internal(str, val, expr_flags, sym_ref)
char * str;
int * val;
int * expr_flags;
SYM ** sym_ref;
{
  int accum = 0,dummy = 0;
  char op;
  

/* eval. multiplication/division */
  str = eval_product(str, &accum, expr_flags, sym_ref);
  	 
  while (white_p(*str))
    str++;
    
  while ( (op = *str)== '+' ||
		          	   op == '-' ||
			            op == '&' ||
			            op == '|' ||
			            op == '^' )
  {
		str++;

		str = eval_product(str, &dummy, expr_flags, sym_ref);
		switch (op)
		{
		  case '+' : accum += dummy; break;
		  case '-' : accum -= dummy; break;
		  case '|' : accum |= dummy; break;
		  case '&' : accum &= dummy; break;
		  case '^' : accum ^= dummy; break;
		}
	
	}
  *val = accum;
  return(str);
}

int eval(char * str,int  * expr_flags,SYM ** sym_ref)

{
  int accum;

  *expr_flags = 0;
  *sym_ref = (SYM *)0;
  eval_internal(str, &accum, expr_flags, sym_ref);

#ifdef DEBUG
  printf("eval('%s')->%X flags %X sym %X\n", str, accum, *expr_flags, *sym_ref);
#endif
  return(accum);
}

