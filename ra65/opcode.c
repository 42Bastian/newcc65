
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* opcode decoder */

#include "global.h"
#include "symtab.h"
#include "util.h"
#include "parse.h"
#include "gen.h"
#include "eval.h"



#include <ctype.h>

 
void barf(char * msg,int arg1,int arg2,int arg3);
int decode_operand(int, int *, int *, SYM **, char **);
void genbranch(SYM * sym,int offset);

/* opcode classes */

/* opcode is the compete instr */
#define	CLIMM	0
/* register inst */
#define CLREG	1
/* general addressing */
#define CLEA	2
/* subset addressing */
#define	CLSA	3
/* subset + accum */
#define CLRSA	4
/* branch */
#define CLBR	5

#define CLJSR	6
#define CLJMP	7

/* defs for operand types */

#define OPIMM	  0x0001		/* immediate */
#define	OPABS	  0x0002		/* absolute, long */
#define OPABSS  0x0004		/* absolute, short */
#define OPABSX  0x0008		/* long,x */
#define OPABSSX 0x0010		/* short,x */
#define OPABSY  0x0020		/* long,y */
#define OPABSSY 0x0040		/* short,y */
#define OPINDX  0x0080		/* (foo,x) */
#define OPINDY  0x0100		/* (foo),y */
#define OPACCUM 0x0200		/* A */
#define OPINDL  0x0400		/* (addr) also (foo) 42BS*/
#define OPBR    0x0800		/* like abs, but different so we can
				   tell what to generate */

/* the opcode table */

/* removed old-way 42BS */

struct op_elt 
	{
	char * name;      /* opcode name */
	char basecode;    /* basic opcode */
	int valid;        /* a bit mask of valid operand types */
	};

/* added OPINDL to LDA,STA,ORA,EOR,AND,SBC,ADC and CMP		*/
/* to allow zeropage indirect					*/
/* also JMP (abs,x)						*/
/* and  TRB,TSB							*/
/* and  BIT #xx,BRK #xx					    42BS*/

/* I'm not including RMBx/SMBx and BBRx/BBSx as these are
   65C02 and not 65SC02 opcodes 42BS */
   
struct op_elt optab[] =
	{
	{"LDA", 0xAD, 
		OPIMM | OPABS | OPABSS | OPABSX | OPABSSX |
		OPABSY | OPINDX | OPINDY | OPINDL},
	{"STA", 0x8D,
		OPABS | OPABSS | OPABSX | OPABSSX |
		OPABSY | OPINDX | OPINDY | OPINDL },
	{"JMP", 0x4C, OPABS | OPINDL | OPINDX},
	{"JSR", 0x20, OPABS},
	{"RTS", 0x60, 0},
	{"STZ", 0x9c, OPABS | OPABSX | OPABSS | OPABSSX}, 	/*KAO / 42BS*/
	{"LDX", 0xAE, OPIMM | OPABS | OPABSS | OPABSY | OPABSSY},
	{"STX", 0x8E, OPABS | OPABSS | OPABSY | OPABSSY},
	{"LDY", 0xAC, OPIMM | OPABS | OPABSS | OPABSY | OPABSSY},
	{"STY", 0x8C, OPABS | OPABSS | OPABSSX},
	{"AND", 0x2D, 
		OPIMM | OPABS | OPABSS | OPABSX | OPABSSX |
		OPABSY | OPINDX | OPINDY | OPINDL}, /*42BS*/
	{"BIT", 0x2C, OPABS | OPABSS | OPABSX | OPABSSX | OPIMM},
	{"CMP", 0xCD, 
		OPIMM | OPABS | OPABSS | OPABSX | OPABSSX |
		OPABSY | OPINDX | OPINDY | OPINDL},
	{"CPX", 0xEC, OPIMM | OPABS | OPABSS},
	{"CPY", 0xCC, OPIMM | OPABS | OPABSS},
	{"ADC", 0x6D, 
		OPIMM | OPABS | OPABSS | OPABSX | OPABSSX |
		OPABSY | OPINDX | OPINDY | OPINDL},
	{"SBC", 0xED, 
		OPIMM | OPABS | OPABSS | OPABSX | OPABSSX |
		OPABSY | OPINDX | OPINDY | OPINDL},
	{"ORA", 0x0D, 
		OPIMM | OPABS | OPABSS | OPABSX | OPABSSX |
		OPABSY | OPINDX | OPINDY | OPINDL},
	{"EOR", 0x4D, 
		OPIMM | OPABS | OPABSS | OPABSX | OPABSSX |
		OPABSY | OPINDX | OPINDY | OPINDL},
	{"INC", 0xEE, OPABS | OPABSS | OPABSX | OPABSSX},
	{"DEC", 0xCE, OPABS | OPABSS | OPABSX | OPABSSX},
	{"INA", 0x1A, 0}, /* KAO */
	{"DEA", 0x3A, 0}, /* KAO */
	{"INX", 0xE8, 0},
	{"DEX", 0xCA, 0},
	{"INY", 0xC8, 0},
	{"DEY", 0x88, 0},
	{"ASL", 0x0E, OPABS | OPABSS | OPABSX | OPABSSX | OPACCUM},
	{"ROL", 0x2E, OPABS | OPABSS | OPABSX | OPABSSX | OPACCUM},
	{"LSR", 0x4E, OPABS | OPABSS | OPABSX | OPABSSX | OPACCUM},
	{"ROR", 0x6E, OPABS | OPABSS | OPABSX | OPABSSX | OPACCUM},
	{"TAX", 0xAA, 0},
	{"TXA", 0x8A, 0},
	{"TAY", 0xA8, 0},
	{"TYA", 0x98, 0},
	{"TSX", 0xBA, 0},
	{"TXS", 0x9A, 0},
	{"CLV", 0xB8, 0},
	{"CLD", 0xD8, 0},
	{"SED", 0xF8, 0},
	{"CLI", 0x58, 0},
	{"SEI", 0x78, 0},
	{"CLC", 0x18, 0},
	{"SEC", 0x38, 0},
	{"BPL", 0x10, OPBR},
	{"BMI", 0x30, OPBR},
	{"BVC", 0x50, OPBR},
	{"BVS", 0x70, OPBR},
	{"BCC", 0x90, OPBR},
	{"BCS", 0xB0, OPBR},
	{"BNE", 0xD0, OPBR},
	{"BEQ", 0xF0, OPBR},
	{"BRA", 0x80, OPBR}, /* KAO */
	{"BRK", 0x00, OPIMM}, /* 42BS */
	{"RTI", 0x40, 0},
	{"PHP", 0x08, 0},
	{"PLP", 0x28, 0},
	{"PHA", 0x48, 0},
	{"PLA", 0x68, 0},
	{"PHX", 0xda, 0}, /* KAO */
	{"PLX", 0xfa, 0}, /* KAO */
	{"PHY", 0x5a, 0}, /* KAO */
	{"PLY", 0x7a, 0}, /* KAO */
	{"NOP", 0xEA, 0},
	{"TSB", 0x0C, OPABSS | OPABS},
	{"TRB", 0x1C, OPABSS | OPABS},
	{0, 0, 0}
	};

/* valid addressing modes, per opclass */
int mvalid[] = 
	{
	0,					/* climm, no args */
	0,					/* clreg, not used? */
	OPIMM | OPABS | OPABSS | OPABSX | 
		OPABSSX | OPABSY | OPABSSY |
		OPINDX | OPINDY | OPINDL,	/* general addressing */
	OPIMM | OPABS | OPABSS | OPABSX | OPABSSX,	/* subset */
	OPIMM | OPABS | OPABSS | OPABSX | OPABSSX | OPACCUM,  /* sub + a */
	OPABS,					/* branch, abs */
	OPABS,					/* jsr, abs */
	OPABS | OPINDL | OPINDX			/* jmp, abs or ind */
	};


/* given a name, return T if it's an opcode, and if so, return
   its code and flags */
int opcode_p(char * name,int *  code,int * valid)
{
  int i;

  if (strlen(name) != 3)
    return(0);		/* don't bother */

  for (i = 0 ; ; i++)
	{
    if (optab[i].name ==  0)
      return(0);
 
	if (string_equal(optab[i].name, name))
		{
		*code = (int)(optab[i].basecode & 0xff); /*42BS*/
		*valid = optab[i].valid;
		return(1);
		}
	}
}


/* handed an opcode and opflags, finish generating this instruction */

void assemble_op(int basecode,int valid,char ** args)
 
{
  int operand, optype, expr_flags;
  SYM * rel_to_sym;

/* later, leave a slot for relocatable stuff, ptr to sym this
   word is relative to, or something */

/*  set_label();  */
  gen_label();
  operand = optype = 0;
  if (valid)
    optype = decode_operand(valid, &operand, &expr_flags, &rel_to_sym,args);

  if ( CurrSegment != TEXT )
  {
    barf("Code only in TEXT-CurrSegment !",0,0,0);
    return;
  }
  
/* now have operand etc */
  switch (optype)
	{
	case 0:			/* no operand */
		{
		genlit(basecode);
		break;
		}
	case OPIMM:
		{
		switch ( basecode ){
		case 0x00 :			/* BRK #xx*/
			genlit( 0x00 );
			break;
		case 0x2c :			/* BIT #xx*/
			genlit(0x89);
			break;
		default	:
		/* bit of a kludge here... gen code ^ 0x04, but if bit 0 is
		    clear, clear bit 3 as well */
		  if (basecode & 0x01)
			  genlit(basecode ^ 0x04);
		      else
			  genlit((basecode ^ 0x04) & 0xF7);
		}
		genbyte(operand, expr_flags, rel_to_sym);
		break;
		}
	case OPABS:
		{
		genlit(basecode);
		genword(operand, expr_flags, rel_to_sym);
		break;
		}
	case OPBR:
		{
		genlit(basecode);
		genbranch(rel_to_sym, operand);
		break;
		}
	case OPABSS:
		{
		if ( basecode == 0x9c ) /*42BS*/
		  genlit( 0x64 );
		else
		  genlit(basecode ^ 0x08);
		/* rel is an error here... */
		genbyte(operand, expr_flags, rel_to_sym);
		break;
		}
	case OPABSX:			/* xxxx,x */
		{
		if ( basecode == 0x9c )
 		  genlit(0x9e);
		else
		  genlit(basecode ^ 0x10);
		genword(operand, expr_flags, rel_to_sym);
		break;
		}
	case OPABSSX:
		{
		if ( basecode == 0x9c ) /*42BS*/
		  genlit( 0x74 );
		else
		  genlit(basecode ^ 0x18);
		/* rel is an error... */
		genbyte(operand, expr_flags, rel_to_sym);
		break;
		}
	case OPABSY:			/* xxxx,y */
		{
		genlit(basecode ^ 0x14);
		genword(operand, expr_flags, rel_to_sym);
		break;
		}
	case OPABSSY:
		{
		genlit(basecode ^ 0x18);
		genbyte(operand, expr_flags, rel_to_sym);
		break;
		}
	case OPINDX:			/* (xx,x) */
		{
		if ( basecode == 0x4c )
		  {genlit( 0x7c);
		   genword(operand,expr_flags, rel_to_sym);
		   break;
		 }
		genlit(basecode ^ 0x0C);
		genbyte(operand, expr_flags, rel_to_sym);
		break;
		}
	case OPINDY:				/* (xx),y */
		{
		genlit(basecode ^ 0x1C);
		genbyte(operand, expr_flags, rel_to_sym);
		break;
		}
	case OPACCUM:		/* A */
		{
		genlit(basecode ^ 0x04);
		break;
		}
	case OPINDL:
		{
		if ( (basecode & 0xf) == 0xd ) /* check for LDA (xx)... */
		{
		  genlit(basecode ^ 0x1f);
		  genbyte(operand, expr_flags, rel_to_sym);
		  break; 
		}
		genlit(basecode ^ 0x20);
		genword(operand, expr_flags, rel_to_sym);
		break;
		}
	}

}

/* passed a bit mask of legal optypes, try to make sense out of the arg
   vector, and return a 16-bit operand value, and the type that it is.
   the value and relative flag are returned by reference; the type is
   returned as the function value;
*/
int decode_operand(valid, operand, expr_flags, rel_to_sym, args)
int valid;			/* mask of valid types */
int * operand;
int * expr_flags;
struct sym ** rel_to_sym;
char * args[];
{
  int val;
  int e_flags;			/* flags from eval */
/*  char indexed_p;	*/	/* if operand is indexed */
  char indirect_p;		/* if operand is indirect */
  int type;			/* optype we figure out */
  char * arg0;
  char * arg1;
  int short_p;

  arg0 = args[0];
  arg1 = args[1];

/* make sure there's an operand */
  if (!arg0)
  {
    if ( valid & OPACCUM )  return (OPACCUM); // let`s guess it`s accu
    barf("operand required",0,0,0);
    return(0);
  };

/* special case check for accum */
  if (string_equal(arg0, "A"))
  {
    return(OPACCUM);
  }

/* first check for some kind of indirect operand */
  if (arg0[0] == '(')
  {
    if (arg1)
    {
      indirect_p = toupper(arg1[0]);

      /* later, be smarter.  this should work for now... */
      if (indirect_p == 'Y')
      type = OPINDY;
      else
      type = OPINDX;
      val = eval(arg0 + 1, &e_flags, rel_to_sym);
      *operand = val;
      *expr_flags = e_flags;
    }
    else		/* must be (addr) */
    {
      type = OPINDL;
      val = eval(arg0 + 1, &e_flags, rel_to_sym);
      *operand = val;
      *expr_flags = e_flags;
    }
    return(type);
  }
  else		/* first arg doesn't start with a paren */
  {
    if ((arg0[0] == '#') || (arg0[0] == '='))	/* immediate? */
    {
      val = eval(arg0 + 1, &e_flags, rel_to_sym);	/* get expr value */
      /* make sure it's 8 bits... later */

      *operand = val & 0xFF;
      *expr_flags = e_flags;
      return(OPIMM);
    }
    else		/* not immediate */
    {
      val = eval(arg0, &e_flags, rel_to_sym);		/* get expr val */
      /* see if indexed by anything */
      if (arg1)
      {
        if (string_equal(arg1, "Y") || string_equal(arg1, "X"))
        {
          indirect_p = toupper(arg1[0]);
          /* later, figure out whether to use short
             addressing; for now just do it the easy way */

          short_p = ((((val & 0xFF00) == 0) &&
                     (!(e_flags & E_UNDEF))&&
                     (!(e_flags & E_REL)))||
                     ((e_flags & E_LO_BYTE))
                     );

          if (indirect_p == 'X')
          {
            if (short_p && (valid & OPABSSX))
              type = OPABSSX;
            else
              type = OPABSX;
          }
          else
          {
            if (short_p && (valid & OPABSSY))
              type = OPABSSY;
            else
              type = OPABSY;
          }
          *operand = val;
          *expr_flags = e_flags;
          return(type);
        }
        else
        {
        /* arg1 there but not x or y?  bogus */
          barf("too many operands",0,0,0);
          return(0);
        }
      }		/* env of 'arg1' */
      else
      {		/* no arg1, must be absolute */
          /* later, check for page 0. */
        *operand = val;
        *expr_flags = e_flags;
        if (valid & OPBR)
          return(OPBR);
        else
        {
          if ( !(valid & OPABSS) ) return OPABS;
          if (
              (( !(val & 0xFF00)      ) &&
              ( !(e_flags & E_UNDEF) ) &&
              ( !(e_flags & E_REL)))||
              ( e_flags & E_LO_BYTE)
             )
          {
//  	printf("line %d %X would have been short %d\n", line_nbr, val,e_flags); 
            return(OPABSS);
          }
          else
            return(OPABS);
        }
      }
    }			/* end of 'not immed' */
  }				/* end of 'not indirect' */		
}
