
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* symbol table code */

#include "util.h"
#include "symtab.h"
#include "parse.h"
#include "global.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*error.c*/
extern void barf(char * msg,int arg1,int arg2,int arg3);
/* xgen.c */
int gen_n_passes();

#define	HASHMOD	512

SYM* symtab[HASHMOD];
int local_count;

int hash(register char *str)
{
register  int accum;
register  char c;

  for (accum = 0 ; ( c = *str++ ) ; )
  {
	  accum = ((accum << 1) & 0x3FFF) ^ c;
  }
  return(accum & (HASHMOD-1));
}

SYM * find_sym(name, new_ok)
char * name;
int new_ok;
{
  SYM * sym_p;
  int idx;
  char *p;
//
// insert value of @
//
  if ( (p = strchr(name,'@')) )
  {
    bcopy(p,p+3,strlen(p+1));
    sprintf(p,"%03d",asm_var);
  }
//
// now go searching
//
//  printf("findsym %s\n",name);
  idx = hash(name);
  
  for (sym_p = symtab[idx] ; sym_p ; sym_p = sym_p->next)
	  if (!strcmp(sym_p->name, name))
	    break;

  if ( sym_p &&
       ( !(sym_p->flags & LOCAL) ||
         ((sym_p->flags & LOCAL) && sym_p->local == local_count)
       ))
  {
//   printf("findsym:(%s)%2x\n",sym_p->name,sym_p->local);
    sym_p->flags |= USED;
    return (sym_p);
  }

  if (!new_ok)	return(0);

  sym_p = (SYM* )malloc((sizeof (SYM)) + strlen(name) + 1);
  sym_p->line = line_nbr;
  sym_p->value = 0;
  sym_p->flags = USED;

  sym_p->nbr = -1;
  sym_p->defline = -1;
  sym_p->local = -1;
  
  if ( name[0] == '.' )
  {
    sym_p->flags |= LOCAL;
    sym_p->local = local_count;
  }
  strcpy(sym_p->name, name);
  sym_p->next = symtab[idx];
  symtab[idx] = sym_p;

//    printf("findsym:(%s)\n",sym_p->name);
  
  return(sym_p);
}

SYM * assign_sym(name, suffix, value, no_redef)
char * name, *suffix;
int value;
int no_redef;
{
  SYM * sym_p;

  sym_p = find_sym(name, 1);
  
  if (no_redef && (sym_p->flags & DEFINED) && (pass != gen_n_passes()-1))
  {
    barf("Label already defined at line %d",sym_p->defline,0,0);
    return (sym_p);
  }
	if (sym_p->value != value && (pass != gen_n_passes()-1) )
		barf("'%s' redefined: %X vs %X\n",(int) name,(int)sym_p->value, value);
		
//printf("assign_sym:%s %02x %d\n",sym_p->name,sym_p->flags,(int)sym_p->value);  

  sym_p->defline = line_nbr;
  sym_p->value = value;
  sym_p->local = -1;
  sym_p->flags |= DEFINED;
  switch( CurrSegment )
  {
    case TEXT : sym_p->flags |= SYMTEXT; break;
    case DATA : sym_p->flags |= SYMDATA; break;
    case BSS  : sym_p->flags |= SYMBSS;  break;
    case BSSZP: sym_p->flags |= SYMBSSZP;break;
  }
  if ( sym_p->name[0] == '.' )
  {
    if ( suffix )
      barf("No suffix allowed for local labels \n",0,0,0);
      
    sym_p->flags |= LOCAL;
    sym_p->local = local_count;
  }
  if ( suffix && suffix[0] == ':' && suffix[1] == ':' )
  {
    sym_p->flags |= GLOBAL;
    NextLocal();
  }
  
  
//printf("assign_sym:%s %02x %d\n",sym_p->name,sym_p->flags,(int)sym_p->local);  
//printf("decl:(%s,%d) at %d\n",sym_p->name,value,line_nbr);
  
  return(sym_p);
}

void NextLocal()
{
  ++local_count;
}

void set_label()
{
  if (p.label)
	{
	  assign_sym(p.label,p.label_suffix , pc, 1);
	  if (!p.opcode)
		{
		  list_v = pc;
		  list_v_p = 1;
		}
	  else
		{
		  list_pc = pc;
		}
	}
}

void init_sym()
{
  int i;

  for (i = 0 ; i < HASHMOD ; i++)
	  symtab[i] = 0;

  local_count = 1;
}

void describe_symtab()
{
  int i, j;
  SYM * sym_p;

  for (i = 0 ; i < HASHMOD ; i++ )
	{
	for (j = 0, sym_p = symtab[i] ; sym_p ; j++)
		sym_p = sym_p->next;
	printf("Bucket %d contains %d syms\n", i, j);
	}
}

void map_syms(void (* fun)())
 
{
  SYM * sym_p;
  int i;

  for (i = 0 ; i < HASHMOD ; i++ )
	{
	for (sym_p = symtab[i] ; sym_p ; )
		{
		(*fun)(sym_p);
		sym_p = sym_p->next;
		}
	}
}
extern char * in_name[];
int find_udef()
{
  SYM * sym_p;
  int i;
  int udef = 0;
  int save_line = line_nbr;
  
  for (i = 0 ; i < HASHMOD ; i++)
    for (sym_p = symtab[i]; sym_p ; )
    {
//      printf("sym(%s) flags(%02x)\n",sym_p->name,sym_p->flags);
      
      if ( (sym_p->flags & USED) && !(sym_p->flags & DEFINED) )         
      {
        line_nbr = sym_p->line;
//not yet        strcpy(CurrentFile,in_name[sym_p->nbr]);
        if ( sym_p->flags & LOCAL)
          barf("'%s' localy undefined (first use)",(int)sym_p->name,0,0);
        else
          barf("'%s' undefined (first use)",(int)sym_p->name,0,0);
        ++udef;
      }
      sym_p = sym_p->next;
    }
   line_nbr = save_line;
  return udef;
}

#ifdef DEBUG
void dump_syms()
{
  char * name[32];
  int i, j;
  SYM * sym_p;
  FILE * f;

  sprintf(name, "pass%d.sym", pass);
  f = fopen(name, "w");
  for (i = 0 ; i < HASHMOD ; i++ )
	{
	for (j = 0, sym_p = symtab[i] ; sym_p ; j++)
		{
		fprintf(f, "%s:\t\t%X\n", sym_p->name, sym_p->value);
		sym_p = sym_p->next;
		}
	}
  fclose(f);
}
#endif

