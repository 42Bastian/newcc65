
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/


/* pseudo ops */


#include "util.h"
#include "gen.h"
#include "parse.h"
#include "symtab.h"
#include "global.h"
#include "eval.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct pseudo
	{
	char * pname;		/* opname */
	void (* func)();		/* fun to call */
	};
/* prototypes */

extern int eval(char * str,int  * expr_flags,SYM ** sym_ref);
extern void gen_lbr(int basecode,SYM * sym);
extern void barf(char * msg,int arg1,int arg2,int arg3);
extern void new_page();

/* macro.h */
extern int processing_macro;

/* util.c */
void save_pos(FILE *);
void restore_pos(FILE *);

/* a65.c */
extern FILE * inf;

/* xgen.c */
void open_segment(short segment);
void close_segment(short segment);
void genbss(int cnt);

/******************/

int do_pseudo();

void dirbyte();
void dirdbyte();
void dirword();
void dirsetpc();
void equate();
void blkb();
#ifdef LIST
void append_args(char * target);
void strip_quotes(char * target,char * source);
void title();
void sbttl();
#endif
void ifzero();
void ifnonzero();
void   endcond();
void dirend();
void globalsym();
void xrefsym();
void xrefzpsym();
void ldax();
void lbr(int basecode);
void   ldax();
void   lbeq();
void   lbne();
void   lbcs();
void   lbcc();
void   lbpl();
void   lbmi();
void   lbra();

void ldas(); // load A signed
void ldau(); // load A unsigned

void macro(); // define a macro
void endm();  // end of macro-definition

void ifnd();  // test if a label is not defined
void ifd();   // test if a label is defined
void ifvar(); // test is a macro-parameter exists

void inc_asm_var();
void dec_asm_var();
void rept();
void endr();

void text();
void data();
void bsszp();
void bss();

int Genesis;

struct pseudo ps[] =
	{
	{".byte", dirbyte},{"byte", dirbyte},{"dc.b", dirbyte},
	{".word", dirword},{"word", dirword},{"dc.w", dirword},
	{"*=", dirsetpc},
	{"org", dirsetpc},
	{"=", equate},{".equ", equate},{"equ", equate},
	{".blkb", blkb},{".ds", blkb},{"ds", blkb},
#ifdef LIST
	{".title", title},
	{".page", title},
	{".sbttl", sbttl},
#endif
	{".dbyt", dirdbyte},
	{".if", ifnonzero},{"if", ifnonzero},
	{".ifnz", ifnonzero},{"ifnz", ifnonzero},
	{".ifne", ifnonzero},{"ifne", ifnonzero}, 
	{".ifeq", ifzero},{"ifeq", ifzero},
	{".ifz", ifzero},{"ifz", ifzero},
	{"ifnd", ifnd},
	{"ifd", ifd},
	{"ifvar",ifvar},
	{".endc", endcond},{"endc", endcond},{"endif", endcond},
	{".end", dirend},{"end", dirend},
	{".globl", globalsym},{"global", globalsym},
	{"xref",xrefsym},	{"xrefzp",xrefzpsym},
	{"ldax", ldax},
	{"lbeq", lbeq},
	{"lbne", lbne},
	{"lbcs", lbcs},
	{"lbcc", lbcc},
	{"lbpl", lbpl},
	{"lbmi", lbmi},
	{"lbra", lbra},
	{"ldas", ldas},
	{"ldau", ldau},
	{"macro", macro},
	{"endm", endm},
	{"inc@",inc_asm_var},
	{"dec@",dec_asm_var},
	{"rept",rept},
	{"endr",endr},
	{"text",text},{"data",data},{"bsszp",bsszp},{"bss",bss},
	/* more later */
	{0, 0}
	};

int do_pseudo()			/* return t if did one */
{
  int i;
  void (* f)();
 
  for (i = 0 ; ps[i].pname ; i++)
	{
	if (string_equal(ps[i].pname, p.opcode))
		{
		f = ps[i].func;
		(*f)();
		return(1);
		}
	}
  return(0);
}

void dirbyte()
{
  int i, j, val, expr_flags;
  struct sym * rel_to_sym;
  char * arg;

  if (!disabled)
  {
    gen_label();
    if (!p.arg[0])
    {
      barf("missig parameter !",0,0,0);
    }
    else if ( CurrSegment == TEXT || CurrSegment == DATA )
    {
      for (i = 0 ; (arg = p.arg[i]) ; i++)
        if (arg[0] == '"')
        {
          for (j = 1 ; (arg[j] != '"') ; j++)
          {
            genlit(arg[j]);
          }
        }
        else
        {
          val = eval(arg, &expr_flags, &rel_to_sym);
          genbyte(val, expr_flags, rel_to_sym);
        }
    }
    else
      barf("Wrong CurrSegment !",0,0,0);
  }
}

void dirdbyte()
{
  int i, val, expr_flags;
  struct sym * rel_to_sym;

  if (!disabled)
  {
    if ( CurrSegment == TEXT || CurrSegment == DATA)
    {
      gen_label();
      for (i = 0 ; p.arg[i] ; i++)
      {
        val = eval(p.arg[i], &expr_flags, &rel_to_sym);
        genlit(val >> 8);
        genlit(val & 0xFF);
      }
    }
    else
      barf("Wrong CurrSegment !",0,0,0);
  }
}

void dirword()
{
  int i, val, expr_flags;
  struct sym * rel_to_sym;

  if(!disabled)
	{
    gen_label();
    if (!p.arg[0])
    {
      barf("missing parameter",0,0,0);
    }
	  else if ( CurrSegment == TEXT || CurrSegment == DATA )
	  {
		  for (i = 0 ; p.arg[i] ; i++)
      {
        val = eval(p.arg[i], &expr_flags, &rel_to_sym);
        genword(val, expr_flags, rel_to_sym);
      }
	  }
    else
      barf("Wrong CurrSegment !",0,0,0);
	}
}

void dirsetpc()
{
  
  barf("not in relocatable mode you don't!",0,0,0);
/*
  int val, expr_flags;
  struct sym * rel_to_sym;

  if (!disabled)
	{
	val = eval(p.arg[0], &expr_flags, &rel_to_sym);
	pc = val;
	list_pc = pc;
	list_pc_p = 1;
	}
*/
}

void equate()
{
  int val, expr_flags;
  struct sym * rel_to_sym;
  struct sym * sy;
  
  if (!disabled)
	{
	  if (!p.label)
		  barf("label required",0,0,0);
    else
    {
      val = eval(p.arg[0], &expr_flags, &rel_to_sym);
      sy = find_sym(p.label, 1);
      if ( (expr_flags & E_UNDEF) || (rel_to_sym && rel_to_sym->flags & XREF))
      {
        sy->rel = rel_to_sym;
        sy->flags = RELATIVE | DEFINED;
       }
      else
        sy->flags = DEFINED | ABS;
 
      list_v = sy->value = val;
      list_v_p = 1;
    }
  }
}

void blkb()
{
  int i, val, expr_flags;
  struct sym * rel_to_sym;

  if (!disabled)
	{
	gen_label();
	val = eval(p.arg[0], &expr_flags, &rel_to_sym);

	list_v = val;
	list_v_p = 1;

  if ( CurrSegment < BSS )
    for (i = 0 ; i < val ; i++)
	    genlit(0);
	else
	  genbss(val);

  }
/*	  pc += val; */
}

#ifdef LIST
/* util routine for title hacking routines */
void append_args(char * target)
{
  int i;

  *target = '\0';
  for (i = 0 ; p.arg[i] ; i++)
	{
	strcat(target, p.arg[i]);
	strcat(target, " ");
	}
}
void strip_quotes(char * target,char * source)
{
  if (*source == '"')
	source++;
  while (*source && (*source != '"'))
	*target++ = *source++;
  *target = '\0';
}

void title()
{
  if (!disabled)
	{
	if (*p.arg[0] == '"')
		strip_quotes(page_title, p.arg[0]);
	    else
		append_args(page_title);
/*  line_listed_p = 1;	*/	/* don't list it */
	if (pass > 0)
		new_page();
	}
}

void sbttl()
{
  if (!disabled)
	{
	if (*p.arg[0] == '"')
		strip_quotes(page_subttl, p.arg[0]);
	    else
		append_args(page_subttl);
	if (pass > 0)
		new_page();
/*  line_listed_p = 1;	*/	/* don't list it */
	}
}

#endif

void ifzero()
{
  int val, e_flags;
  struct sym * unused;

  if (!disabled)
	{
	val = eval(p.arg[0], &e_flags, &unused);
	list_v = val;
	list_v_p = 1;
	if (val)
		disabled = 1;
	}
}

void ifnonzero()
{
  int val, e_flags;
  struct sym * unused;

  if (!disabled)
	{
	val = eval(p.arg[0], &e_flags, &unused);
	list_v = val;
	list_v_p = 1;
	if (!val)
		disabled = 1;
	}
}

void ifnd()
{
  if (!disabled)
    if ( find_sym(p.arg[0],0) )
      disabled = 1;
}

void ifd()
{
 
  if (!disabled)
    if ( !find_sym(p.arg[0],0) )
      disabled = 1;
}

void ifvar()
{
  if ( !processing_macro )
    barf("ifvar outside macro",0,0,0);
  else
  {
    if (!disabled)
      if ( !p.arg[0] )
        disabled = 1;
  }
}
   
void   endcond()
{
  disabled = 0;
}

void dirend()
{
/* zzz later, maybe deal with setting start addr here? */
  end_file = 1;
}

extern void ass_sym_values();
/* make symbol(s) global */

void globalsym()
{
  char * name;
  int i;
  struct sym * sym;

  for (i = 0 ; (name = p.arg[i]) ; i++)
	{
	  sym = find_sym(name, 1);
	  if (sym->flags & LOCAL)
	    barf("Declaring a local symbol global !",0,0,0);
	    
	  sym->flags |= GLOBAL|USED; /* we want an error for
	                                useless global declarations
	                             */
	}
	NextLocal();
}

void xrefsym()
{
  char * name;
  int i;
  struct sym * sym;

  for (i = 0 ; (name = p.arg[i]) ; i++)
	{
	  if ( !(sym = find_sym(name,0)) )
	  {
	    sym = find_sym(name,1);
	    sym->flags &= ~(USED|THIS_SEG);
	  }
//	  else if ( sym->flags & DEFINED)
//	    barf("symbol already defined : line %d",sym->defline,0,0);

	  if (sym->flags & LOCAL)
	    barf("No external ref. to a local symbol !",0,0,0);
	    
	  sym->flags |= DEFINED|XREF;
	  sym->defline = line_nbr;
	}
}
void xrefzpsym()
{
  char * name;
  int i;
  struct sym * sym;

  for (i = 0 ; (name = p.arg[i]) ; i++)
	{
	  if ( !(sym = find_sym(name,0)) )
	  {
	    sym = find_sym(name,1);
	    sym->flags &= ~(USED|THIS_SEG);
	  }
//	  else if ( sym->flags & DEFINED)
//	    barf("symbol already defined : line %d",sym->defline,0,0);

	  if (sym->flags & LOCAL)
	    barf("No external ref. to a local symbol !",0,0,0);
	    
	  sym->flags |= DEFINED|XREF|SYMBSSZP;
	  sym->defline = line_nbr;
	}
}

void ldax()
{
  int val, e_flags;
  struct sym * sy;

  if (*p.arg[0] != '#')
	{
	  val = eval(p.arg[0],&e_flags,&sy);
	  if (e_flags & E_REL)
	  {   
	  if ( e_flags & E_LO_BYTE )
    {
      genlit(0xa5);
      genbyte(val, e_flags, sy);
      genlit(0xa6);
      genbyte(val+1, e_flags, sy);
     }
    else
    { 
	    genlit(0xad);
	    genword(val,e_flags,sy);
	    genlit(0xae);
	    genword(val+1,e_flags,sy);
	   }
	    return;
    }
	  if ( val <= 254 )
	  {
	    genlit(0xa5);
	    genlit(val);
	    genlit(0xa6);
	    genlit(val+1);
	  }else
	  {
	    genlit(0xad);
	    genlit(val & 0xff); genlit(val >> 8);
	    ++val;
	    genlit(0xae);
	    genlit(val &0xff); genlit(val >> 8);
	  }
	}
	else
	{ 
  	val = eval(p.arg[0]+1, &e_flags, &sy);
  	genlit(0xA9);		/* lda immed */
  	if (e_flags & E_REL)
			genbyte(val, e_flags | E_LO_BYTE, sy);
  	else
			genlit(val & 0xFF);
			
  	genlit(0xA2);		/* ldx immed */
  	if (e_flags & E_REL)
  		genbyte(val, e_flags | E_HI_BYTE, sy);
  	else
			genlit(val >> 8);
	}
}
void ldau()
{
  int val, e_flags;
  struct sym * sy;

  val = eval(p.arg[0],&e_flags,&sy);
  genlit(0xa2); genlit(0);	// ldx #0
  if (e_flags & E_REL)
  {
    if ( e_flags & E_LO_BYTE )
    {
      genlit(0xa5);
      genbyte(val, e_flags, sy);
    }
    else
    {
      genlit(0xad);
		  genword(val, e_flags , sy);
		}
	}
	else
  {
    if ( val <= 254 )
	  {
	    genlit(0xa5);
	    genlit(val);
	  }
	  else
	  {
	    genlit(0xad);
	    genlit(val & 0xff); genlit(val >> 8);
	  }
	}
}	
void ldas()
{
  int val, e_flags;
  struct sym * sy;

  val = eval(p.arg[0],&e_flags,&sy);
	genlit(0xa2); genlit(0);	// ldx #0
  if (e_flags & E_REL)
  {
    if ( e_flags & E_LO_BYTE )
    {
      genlit(0xa5);
      genbyte(val, e_flags, sy);
    }
    else
    {  
    genlit(0xad);
		genword(val, e_flags , sy);
		}
	}
	else
  {
    if ( val <= 254 )
	  {
	    genlit(0xa5);
	    genlit(val);
	  }
	  else
	  {
	    genlit(0xad);
	    genlit(val & 0xff); genlit(val >> 8);
	  }
	}
	genlit(0x10); genlit(1);	// bpl *+3
	genlit(0xca);             // dex
}	
void lbr(int basecode)
{
  int val, e_flags;
  struct sym * sy;

  val = eval(p.arg[0], &e_flags, &sy);
  gen_lbr(basecode, sy);
}

void   lbeq() { lbr(0xF0); }  
void   lbne() { lbr(0xD0); }  
void   lbcs() { lbr(0xB0); }  
void   lbcc() { lbr(0x90); }  
void   lbpl() { lbr(0x10); }  
void   lbmi() { lbr(0x30); }  
void   lbra() { lbr(0x80); }

int DefineMacro(char *name);

void macro()
{
  if (p.arg[0])
  {
    DefineMacro(p.arg[0]);
  }
  else
    barf("missing macro-name !",0,0,0);
}

void endm()
{
  barf("endm without macro !",0,0,0);
}

void inc_asm_var()
{
  if ( ++asm_var > 999 )
    barf("range of @ exeeded",0,0,0);
}

void dec_asm_var()
{
  if ( --asm_var < 0 )
    barf("@ became negative",0,0,0);
}

int repeat_count;
int repeat_saveverbose;
void rept()
{
  if ( repeat_count )
  {
    barf("missing endr",0,0,0);
    return;
  }
  
  save_pos(inf);
  if ( !p.arg[0] || p.arg[1] )
  {
    barf("wrong number of parameters",0,0,0);
  }
  else
  {
  int val, e_flags;
  struct sym * sy;

	  val = eval(p.arg[0],&e_flags,&sy);
	  if (e_flags & E_REL || val <= 0)
	  {
	    barf("rept needs positive absolute value !",0,0,0);
	  }
	  else
	    repeat_count = val;
  }
  repeat_saveverbose = verbose;
  
}

void endr()
{
  if ( !repeat_count )
  {
    barf("missing rept",0,0,0);
    return;
  }
  if (--repeat_count)
  {
    if ( repeat_saveverbose ) verbose = repeat_saveverbose -1;
    restore_pos(inf);
  }
  else
    verbose = repeat_saveverbose;

}

void text()
{
  if (CurrSegment != TEXT )
  {
    close_segment(CurrSegment);
    open_segment(TEXT);
    CurrSegment = TEXT;
  }
}

void data()
{
  if (CurrSegment != DATA )
  {
    close_segment(CurrSegment);
    open_segment(DATA);
    CurrSegment = DATA;
  }
}

void bsszp()
{
  if (CurrSegment != BSSZP )
  {
    close_segment(CurrSegment);
    open_segment(BSSZP);
    CurrSegment = BSSZP;
  }
}

void bss()
{
  if (CurrSegment != BSS )
  {
    close_segment(CurrSegment);
    open_segment(BSS);
    CurrSegment = BSS;
  }
}


