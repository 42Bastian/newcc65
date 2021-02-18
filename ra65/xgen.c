//#define DEBUG
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/
 
/* code generator */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef O_BINARY
#  define O_BINARY 0
#endif

#include "global.h"
#include "parse.h"
#include "symtab.h"
#include "eval.h"
#include "obj.h"


/* structs */

/* when assembling, we collect a bunch of the following structs */
struct frag_literal
	{
	int nbytes;
	char bytes[32];
	};
	
struct frag
	{
	struct frag * f_next;  /* next frag */
#ifdef DEBUG
	int seg_pc;		       	 /* debug only */
#endif
	char type;			       /* type this frag */
	int f_value;			     /* value.  if literal, points to a frag_literal.  */
	struct frag_literal * fl_ptr;
	
	SYM * f_sym;		 /* sym defined, or sym rel to */
	};

/* prototypes */
void barf(char * msg,int arg1,int arg2,int arg3);
void map_syms(void (* fun)());

void out16(int i);
void out8(char i);
void outsym(SYM * sym);

struct frag * add_frag(int type,int value,SYM * sym);
void litbyte(char b);
void writelit(struct frag_literal * l);
void genlit(char byte);
void genbyte(int byte,int flags,SYM * sym);
void genword(int word,int flags,SYM * sym);
void genbranch(SYM * sym,int offset);
void gen_lbr(int basecode,SYM * sym);
void gen_label();
void dump_frags();
void init_gen();
void gen_o_open(char ** name,char * source_name);

int frag_gen_size(struct frag * this_frag,int frag_pc);
void ass_sym_values();
void assign_next_num(SYM * sym);
void asgn_sym_numbers();
void out_global_sym(SYM * sym);
void output_symtab();
void out_frags();
void compute_seg_size();
void output_header();
void gen_o_close();
int gen_n_passes();

/* fragment types */
#define FRAG_LITERAL		 0
#define FRAG_SYMDEF			 1
#define FRAG_WORD				  2
#define FRAG_BYTE_HI		 3
#define FRAG_BYTE_LO		 4
#define FRAG_BR_OFFSET	5
#define FRAG_LONG_BR		 6
#define FRAG_BSS       7


/* zzz */

char outname_buf[32];
int output_fd;

struct segmentS
  {
    struct frag * base;
    struct frag * last;
    int size;           // linked size
    int objsize;        // obj-size
  } segment[4];

char *segment_names[4]= {"TEXT","DATA","BSS","BSSZP"};

struct frag * all_frags;
struct frag * last_frag;

struct relfile rf;

/* data accumulated for header */
int seg_max_pc;			/* nbytes this seg takes when linked */
int seg_nbytes;			/* nbytes data this seg in obj file */
int seg_nsyms;			 /* number of (external) syms this seg */
int seg_nsbytes;		/* number of bytes of symtab this file */


char two_bytes[2];

int sym_num;
/* */

void open_segment(short segment);
void close_segment(short segment);

void open_segment(short s)
{
  all_frags = segment[s].base;
  last_frag = segment[s].last;
  seg_max_pc = segment[s].size;
  seg_nbytes = segment[s].objsize;
}

void close_segment(short s)
{
  segment[s].base = all_frags;
  segment[s].last = last_frag;
  segment[s].size = seg_max_pc;
  segment[s].objsize = seg_nbytes;
}

/*************/
/* functions */
/*************/
int written = 0;

void out16(int i)
{
  two_bytes[0] = i & 0xFF;
  two_bytes[1] = i >> 8;
  write(output_fd, two_bytes, 2);
  written += 2;
}

void out8(char i)
{
  two_bytes[0] = i & 0xFF;
  write(output_fd, two_bytes, 1);
  ++written;
}

void outsym(SYM * sym)
{
  char *p = sym->name;
  out8(strlen(p));		/* output length */
  while ( *p )
    out8(*p++);
    
  out16(sym->value);
  if ( sym->flags & XREF )
    out16(0);
  else
    out16(sym->flags & 0xff);
}

/*
 * the next section of code is dealing with frags
 */

struct frag * add_frag(int type,int value,SYM * sym)
{
  struct frag * this_frag;

  this_frag = (struct frag *)malloc(sizeof(struct frag));
  
  if (!all_frags)
    all_frags = this_frag;
  else
	  last_frag->f_next = this_frag;
	  
  last_frag = this_frag;
  this_frag->f_next = NULL;
  this_frag->type = type;
  this_frag->f_sym = sym;

  if (type == FRAG_LITERAL)
	{
	struct frag_literal * lit;
	
	  lit = (struct frag_literal *)malloc(sizeof(struct frag_literal));
  	this_frag->fl_ptr =  lit;
	  lit->nbytes = 0;
	}
  else
	  this_frag->f_value = value;
	  
  return(this_frag);
}

/* push a literal byte, maybe add new frag first */
void litbyte(char b)
{
  struct frag_literal * litbuf;
  if (last_frag && (last_frag->type == FRAG_LITERAL))
  {
  	litbuf = (struct frag_literal *)last_frag->fl_ptr;
  }
  else
	  litbuf = 0;
	  
  if ( !litbuf || litbuf->nbytes >= 32)
	{
	  add_frag(FRAG_LITERAL, 0, NULL);
	  litbuf = (struct frag_literal *)last_frag->fl_ptr;
	}
  litbuf->bytes[litbuf->nbytes++] = b;
}
 
/*
 * The three entry points used by the rest of the assembler 
 */

/* generate a literal byte */
void genlit(char byte)
{
  litbyte(byte);
}

/* generate a byte value */
void genbyte(int byte,int flags,SYM * sym)
{
  int type = FRAG_LITERAL;

  if (!(flags & E_REL))		/* relative? */
  {
    litbyte(byte);
  }
  else
	{
    if (flags & E_HI_BYTE)
      type = FRAG_BYTE_HI;
    else
      if (flags & E_LO_BYTE)
        type = FRAG_BYTE_LO;
      else
	      barf ("internal error, byte not hi or low?",0,0,0);
	      
  add_frag(type, byte, sym);
	}
}

void genbss(int count)
{
  add_frag(FRAG_BSS,count,(SYM *)0);
}

/* generate a word value */
void genword(int word,int flags,SYM * sym)
{
  if (!(flags & E_REL))		/* not relative? */
  {
    genlit(word & 0xFF);
    genlit(word >> 8);
  }
  else
  {			/* it is relative */
    add_frag(FRAG_WORD, word, sym);
  }
}

void genbranch(SYM * sym,int offset)
{
#ifdef DEBUG
  if (sym)
	printf("  genbranch %s+%X\n", sym->name, offset);
    else
	printf("  genbranch *+%X\n", offset);
#endif
  if (!sym)		/* no sym means pc, ie "beq *+3" */
    genlit(offset - 2);
  else
    add_frag(FRAG_BR_OFFSET, offset, sym);
}

void gen_lbr( int basecode,		/* base opcode */
              SYM * sym)
{
#ifdef DEBUG
  printf("  gen long branch %X %s\n", basecode, sym->name);
#endif
  add_frag(FRAG_LONG_BR, basecode , sym);
  /* we know basecode is a byte.  we'll use the top byte later,
     as a flag to say whether it's a long or short branch */
}

/* enter a label.  the label's in p.label */
void gen_label()
{
  SYM * sym = (SYM *)NULL;

  if (p.label)
	{
    sym = assign_sym(p.label,p.label_suffix, 0, 1);
    add_frag(FRAG_SYMDEF, 0, sym);
	}
}

#ifdef DEBUG
/* debug code */
void dump_frags()
{
  struct frag * this_frag;
  struct frag_literal * l;
  int i;
  FILE * fd;

//  fd = fopen("ra65.dmp", "w");
  fd = stdout;
  
  for (this_frag = all_frags ; this_frag ; this_frag = this_frag->f_next)
	{
	fprintf(fd, "PC:%04X frag %6p type %02X value %06X\n", 
		this_frag->seg_pc,
		this_frag, this_frag->type, this_frag->f_value);
		
	switch (this_frag->type)
		{
		case FRAG_SYMDEF:
			{
			fprintf(fd, "  sym(%s) level(%d)\n", this_frag->f_sym->name,(signed int)this_frag->f_sym->local);
			break;
			}
		case FRAG_LITERAL:
			{
			l = this_frag->fl_ptr;
			fprintf (fd, "  %d bytes:\n    ", l->nbytes);
			for (i = 0 ; i < l->nbytes ; i++)
				fprintf(fd, " %02X", l->bytes[i] & 0xFF);
			fprintf(fd, "\n");
			break;
			}
		case FRAG_WORD:
			{
			fprintf(fd, "  word      %s + %04X\n", 
				this_frag->f_sym->name, this_frag->f_value);
			break;
			}
		case FRAG_BYTE_HI:
			{
			fprintf(fd, "  high byte %s + %04X\n", 
				this_frag->f_sym->name, this_frag->f_value);
			break;
			}
		case FRAG_BYTE_LO:
			{
			fprintf(fd, "  low byte  %s + %04X\n", 
				this_frag->f_sym->name, this_frag->f_value);
			break;
			}
		case FRAG_BR_OFFSET:
			{
			fprintf(fd, "  branch    %s + %04X\n", 
				this_frag->f_sym->name, this_frag->f_value);
			break;
			}
		case FRAG_LONG_BR:
			{
			fprintf(fd, "  long branch %X %s\n", 
				this_frag->f_value, this_frag->f_sym->name);
			break;
			}
	  case FRAG_BSS:
	    {
	    fprintf(fd,"  bss size %d\n",this_frag->f_value);
	    break;
			}
		}
	}
//  fclose(fd);
}
#endif		/* debug */

/*
 * init and util code
 */



/* return the size in object-bytes of a frag, ie how much of the 
   final executable this frag accounts for. */
   
int frag_gen_size(struct frag * this_frag,int frag_pc)
/* int frag_pc;			 module pc this frag is sized at */
{
  SYM * sy;
  struct frag * xfrag;
  struct frag_literal * l;
  int offset, tmp;

  switch (this_frag->type)
	{
	case FRAG_SYMDEF:
		return(0);		/* generates no bytes */
	case FRAG_WORD:
		return(2);
	case FRAG_BYTE_HI:
	case FRAG_BYTE_LO:
		return(1);
	case FRAG_LITERAL: 
		{
		l = (struct frag_literal *)this_frag->fl_ptr;	/* n bytes this literal frag */
		return(l->nbytes);
		}
	case FRAG_BR_OFFSET: 
		return(1);		/* branch offset is one byte */
	case FRAG_LONG_BR:
		{
		tmp = (this_frag->f_value >> 8) ;	/* get top byte */
		if (tmp > 0)			/* it's already a size.*/
			{
#ifdef DEBUG
			printf("  size long branch, found size %d\n", tmp);
#endif
			return(tmp);		/* return it */
			}
		/* this one's tricky.  if the symbol's defined,
		   and in this module,
		   we're in good shape; just compute the offset
		   from the symbol to the module PC and mark the
		   frag short or long, depending on the offset.
		   if the symbol's not defined yet, scan forward
		   thru frags counting sizes until find symbol
		   or cumulative size gets too big.  If found it
		   within arbitrary offset, assume real offset
		   will be short enough to generate a short branch
		*/
		sy = this_frag->f_sym;		/* get the sym */
		if (sy->flags & THIS_SEG)	/* this sym defined here? */
			{
			/* sym's defined.  compute offset from PC. */
			offset = frag_pc - sy->value;
#ifdef DEBUG
			printf("  sym %s already defined, offset %d\n", 
				sy->name, offset);
#endif
			if ( (offset > -126) && (offset <124 ))	/* slush */
			  return(2);	/* can do short br */
			    else
            if ( (this_frag->f_value & 0xff) != 0x80)    /*42BS*/
              return(5);	/* must do long br */ 
            else
              return(3); /* only JMP xxx */           /*42BS*/				 
			}
			
		/* search forward thru frag list for the ref'ed sym */
		offset = 0;
#ifdef DEBUG
		printf("  searching forward for %s\n", sy->name);
#endif
		for (xfrag = this_frag->f_next ; xfrag ; xfrag = xfrag->f_next)
			{
			if (offset > 124)	/* too big? */
				if ( (this_frag->f_value & 0xff) != 0x80)   /*42BS*/
					return(5);	/* yes, assume long br */
				else
					return(3);
					
			if ((xfrag->type == FRAG_SYMDEF) &&
          (xfrag->f_sym == sy))
				return(2);	/* found it.  short br */
			if (xfrag->type == FRAG_LONG_BR)
				if ( (this_frag->f_value & 0xff) != 0x80)   /*42BS*/
				  offset += 5;	/* don't recurse */
				else
				  offset += 3; 
        else
          offset += frag_gen_size(xfrag, frag_pc);
#ifdef DEBUG
			printf("  see frag type %X, offset now %d\n",
				xfrag->type, offset);
#endif
			}
		/* oops! fell off end of frags? ok return long */
		return(5);
		}
  case FRAG_BSS:
    return(this_frag->f_value);
	default:
		barf("frag_gen_size: unknown frag type %x", this_frag->type,0,0);
	}
	return 0;
}
/**********************************************************************/
/* walk thru all frags assigning symbol offsets and long branch sizes */
/**********************************************************************/
void ass_sym_values()
{
  struct frag * this_frag;
  int segment_pc, tmp;

  segment_pc = 0;
  for (this_frag = all_frags ; this_frag ; this_frag = this_frag->f_next)
	{

#ifdef DEBUG
	this_frag->seg_pc = segment_pc;		/* sanity check */
#endif
	switch (this_frag->type)
		{
		case FRAG_SYMDEF:
			{
/* zzz do some checking before bash sym value */
			this_frag->f_sym->value = segment_pc;
		  this_frag->f_sym->flags |= THIS_SEG;
			break;
			}
		case FRAG_WORD:
		case FRAG_BYTE_HI:
		case FRAG_BYTE_LO:
		case FRAG_LITERAL: 
		case FRAG_BR_OFFSET:
		case FRAG_BSS:
			{
			segment_pc += frag_gen_size(this_frag, segment_pc);
			break;
			}
		case FRAG_LONG_BR:
			{
			/* see hair in size routine, above */
			tmp = frag_gen_size(this_frag, segment_pc);
#ifdef DEBUG
			printf("@%04X: setting long branch size to %d\n",
				segment_pc, tmp);
#endif
			this_frag->f_value |= (tmp << 8);
			segment_pc += tmp;
			break;
			}
		default:
			barf("ass_sym_values: unknown frag type %x", this_frag->type,0,0);
		}
	}
  seg_max_pc = segment_pc;
    
//  printf("assign sym val: max pc = %04X\n", segment_pc);
}
/*********************************************************************
   walk thru all syms assigning numbers to all those which are 
   ref'ed or def'ed in this module, and are global
**********************************************************************/   
void assign_next_num(SYM * sym)
{
  /* only count symbols that are
     a) declared global _and_ defined
     b) external labels which are actually used here
  */
  
  if (( (sym->flags & (GLOBAL|DEFINED)) == (GLOBAL|DEFINED) ) ||
      ( (sym->flags & (XREF|USED)) ==(XREF|USED) )
      )
  {

//printf("(%s) = %d\n",sym->name,sym_num);

	  sym->nbr = sym_num++;
    seg_nsyms++;
	  seg_nsbytes += strlen(sym->name) + 1 + 4;
  }
}
/*********************************************************************/
void asgn_sym_numbers()
{
  sym_num = 0;
  map_syms(assign_next_num);
}
/*********************************************************************/
void out_global_sym(SYM * sym)
{
  if (sym->nbr >= 0)
    outsym(sym);
}
/*********************************************************************/
void output_symtab()
{
  map_syms(out_global_sym);
}
/*********************************************************************/
void out_frags()
{
  struct frag * this_frag;
  struct frag_literal * l;
  int segment_pc;
  int i;
  
  segment_pc = 0;
  
  for (this_frag = all_frags ; this_frag ; this_frag = this_frag->f_next)
	{
#ifdef DEBUG
	if (this_frag->seg_pc != segment_pc)
		barf("Internal error! frag %x sez it's at pc %04x, computed %04x\n",
			(int)this_frag,(int)this_frag->seg_pc,(int)segment_pc);
#endif
			
	switch (this_frag->type)
		{
		case FRAG_SYMDEF:
			{
/* this is a no-op here.  maybe check validity? */
			break;
			}
		case FRAG_WORD:
			{
			i = this_frag->f_sym->nbr;
      if (this_frag->f_sym->flags & THIS_SEG)
      {
/* output "rel word, offset by this segment base" */
        int flags =0;
        switch (this_frag->f_sym->flags & 0xf0)
        {
          case SYMTEXT: flags = 0x0; break;
          case SYMDATA: flags = 0x4; break;
          case SYMBSS : flags = 0x8; break;
          case SYMBSSZP:flags = 0xc; break;
        }
				out8(OP_REL|flags);
				out16(this_frag->f_sym->value + this_frag->f_value);
      }
      else
      {
/* output "rel word, offset from symbol number foo" */
				if (i < 32)
					out8(OP_SYM | i);
        else
					{
					out8(OP_SYM | OP_SYM_EMASK | ( (i >> 8) & 0x1f));
					out8(i & 0xFF);
					}
				out16(this_frag->f_value);
      }
			segment_pc += 2;
			break;
			}
		case FRAG_BYTE_HI:
			{
			i = this_frag->f_sym->nbr;
			if (this_frag->f_sym->flags & THIS_SEG)
      {
/* output "rel hibyte, offset by this segment base" */
        int flags =0;
        switch (this_frag->f_sym->flags & 0xf0)
        {
          case SYMTEXT: flags = 0x0; break;
          case SYMDATA: flags = 0x4; break;
          case SYMBSS : flags = 0x8; break;
          case SYMBSSZP:flags = 0xc; break;
        }
				out8(OP_REL_HI|flags);
				out16(this_frag->f_sym->value + this_frag->f_value);
      }
      else
      {
/* output "rel hibyte, offset from symbol number foo" */
				if (i < 32)
					out8(OP_SYM_HI | i);
        else
        {
          out8(OP_SYM_HI | OP_SYM_EMASK | ((i >> 8) & 0x1f));
          out8(i & 0xFF);
        }
        out16(this_frag->f_value);
      }
			segment_pc += 1;
			break;
			}
		case FRAG_BYTE_LO:
			{
			i = this_frag->f_sym->nbr;
			if (this_frag->f_sym->flags & THIS_SEG)
				{
/* output "rel lobyte, offset by this segment base" */
        int flags =0;
        switch (this_frag->f_sym->flags & 0xf0)
        {
          case SYMTEXT: flags = 0; break;
          case SYMDATA: flags = 4; break;
          case SYMBSS : flags = 8; break;
          case SYMBSSZP:flags = 12;break;
        }
				out8(OP_REL_LO|flags);
				out16(this_frag->f_sym->value + this_frag->f_value);
				}
			    else
				{
/* output "rel lobyte, offset from symbol number foo" */
				if (i < 32)
					out8(OP_SYM_LO | i);
        else
        {
					out8(OP_SYM_LO | OP_SYM_EMASK | ((i >> 8) & 0x1f));
					out8(i & 0xFF);
        }
				out16(this_frag->f_value);
				}
			segment_pc += 1;
			break;
			}
		case FRAG_LITERAL: 
			{
			  
			l = (struct frag_literal *)this_frag->fl_ptr;
			out8(OP_LIT | (l->nbytes & 0x1f));
			for (i = 0 ; i < l->nbytes ; i++)
				out8(l->bytes[i]);
			segment_pc += l->nbytes;
			break;
			}
		case FRAG_BR_OFFSET: 
			{
/* should really collapse this frag into another literal one... */
			out8(OP_LIT | 1);	/* one literal byte */
			out8(this_frag->f_value + this_frag->f_sym->value - segment_pc - 1);
			segment_pc += 1;
			break;
			}
		case FRAG_LONG_BR:
			{
			i = this_frag->f_value >> 8;	/* get resolved size */
			if (!((i == 2) || (i == 5) || (i == 3) ))
				barf("bogus long branch size %d", i,0,0);
				
			if (i == 2)
				{
				/* do a short branch */
				out8(OP_LIT | 2);	/* 2 literal bytes */
				out8(this_frag->f_value);	/* out the branch code */
				segment_pc += 1;
				out8(this_frag->f_sym->value - segment_pc - 1);
							/* out the offset */
				segment_pc += 1;
				}
			else
				{
				/* do a long branch */
				if ( i == 3 )
				  out8(OP_LIT | 1); /* jmp instead bra */ /*42BS*/
				else
				{
				  out8(OP_LIT | 3);	/* 3 literal bytes */
				  /* out brach opcode, flipping sense */
				  out8(this_frag->f_value ^ 0x20);
				  out8(3);		/* branch offset, *+5 */
				 }
				out8(0x4C);		/* opcode for jmp */
				out8(OP_REL);
				out16(this_frag->f_sym->value);
				segment_pc += i; /* add 3 or 5 */
				}
			break;
			}
	  case FRAG_BSS:
	    {
	      segment_pc += this_frag->f_value;
	      break;
	    }
		default:
			barf("out_frags: unknown frag type %x", this_frag->type,0,0);
		}
	}
  if (segment_pc != seg_max_pc)
    	barf("segment output mismatch, expected %04X, got %04X",seg_max_pc, segment_pc,0);
/*  printf("out_frags: max pc = %04X\n", segment_pc); */
}

/***********************************************************/
/* figure out how many bytes this seg takes up in the file */
/***********************************************************/

void compute_seg_size()
{
  struct frag * this_frag;
  struct frag_literal * l;
  int i;

/* could probly also use this routine to collapse frags */

  for (this_frag = all_frags ; this_frag ; this_frag = this_frag->f_next)
	{
	switch (this_frag->type)
		{
		case FRAG_SYMDEF:
			{
/* this is a no-op here.  maybe check validity? */
			break;
			}
		case FRAG_WORD: case FRAG_BYTE_HI: case FRAG_BYTE_LO:
			{
			i = this_frag->f_sym->nbr;
			if (this_frag->f_sym->flags & THIS_SEG)
      {
/* output "rel word, offset by this segment base" */
//		  out8(OP_REL);
//			out16(this_frag->f_sym->value);

				seg_nbytes += 3;
				}
        else
				{
/* output "rel word, offset from symbol number foo" */
				if (i < 32)
				{
//        out8(OP_SYM | i);
          seg_nbytes += 1;
        }
        else
        {
//          out8(OP_SYM | OP_SYM_EMASK | (i >> 8));
//          out8(i & 0xFF);
					seg_nbytes += 2;
        }
//      out16(this_frag->f_value);
        seg_nbytes += 2;
      }
			break;
			}
		case FRAG_LITERAL: 
			{
			l = (struct frag_literal *)this_frag->fl_ptr;
//      out8(OP_LIT | (l->nbytes & 0x1F));
//      for (i = 0 ; i < l->nbytes ; i++)
//        out8(l->bytes[i]);	*/
      seg_nbytes += l->nbytes + 1;
      break;
			}
      case FRAG_BR_OFFSET: 
			{
        seg_nbytes += 2;
        break;
			}
		case FRAG_LONG_BR:
			{
			int tmp;

			tmp = this_frag->f_value >> 8;
			if (tmp == 2)
				seg_nbytes += 3;	/* lit + br + offset */
			  
			else if (tmp == 5)
				seg_nbytes += 7;
			
			else if (tmp == 3) /* jmp instead bra */ /*42BS*/
			 	seg_nbytes += 5;
			  
			 else
				barf("Bogus long branch size code %d\n", tmp,0,0);
			break;
			}
	  case FRAG_BSS:
	  {
	    // no-op here
	    break;
	  }
		default:
			barf("compute_seg_size: unknown frag type %x", this_frag->type,0,0);
		}
	}
}

void output_header()
{
  rf.header = NEW_OBJ_HEADER;
  rf.nb_sym = seg_nsbytes;
  rf.nb_seg = segment[0].size;
  rf.nb_segdata = segment[0].objsize;
  rf.n_sym = seg_nsyms;

  rf.obj_data_size = segment[1].objsize;
  rf.data_size = segment[1].size;
  rf.bss_size = segment[2].size;
  rf.bsszp_size = segment[3].size;
  
  out16(rf.header);
  out16(rf.nb_sym);
  out16(rf.nb_seg);
  out16(rf.nb_segdata);
  out16(rf.n_sym);
  out16(rf.obj_data_size);
  out16(rf.data_size);
  out16(rf.bss_size);
  out16(rf.bsszp_size);

  if (verbose)
  {
    int i;
    for (i = TEXT; i<=BSSZP; ++i)
	    printf("%5s: %d bytes\n",segment_names[i],segment[i].size);
  }
}
void init_gen()
{
  rf.header = NEW_OBJ_HEADER;
  rf.nb_sym = 0;
  rf.nb_seg = 0;
  rf.nb_segdata = 0;
  rf.n_sym = 0;
  rf.obj_data_size = 0;
  rf.data_size = 0;
  rf.bss_size = 0;
  rf.bsszp_size = 0;
  all_frags = last_frag = 0;
  segment[0].size =
  segment[1].size =
  segment[2].size =
  segment[3].size =
  segment[0].objsize =
  segment[1].objsize =
  segment[2].objsize =
  segment[3].objsize = (int)
  segment[0].base =
  segment[1].base =
  segment[2].base =
  segment[3].base = 0;
}

void gen_o_open(char ** name,char * source_name)
{
  char * ext;
 
/* printf("gen-output-open(%x, %s)\n", name, source_name); */

  if (*name && **name)
	  strcpy(outname_buf, *name);
  else
	  strcpy(outname_buf, source_name);

  *name = outname_buf;
  
  if ( (ext = (char *)strchr(outname_buf, '.')) ) 
    *ext = '\0';
    
	strcat(outname_buf, ".obj");
 
  output_fd = open(outname_buf, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644);
  if (output_fd <= 0)
  {
    fprintf(stderr, "Can't open output file, %d\n", output_fd);
    exit(9);
	}
// printf("output '%s'->%x\n", outname_buf, output_fd); 


}

void gen_o_close()
{
  short i;
  
  seg_nbytes = seg_max_pc = seg_nsyms =  0;
  seg_nsbytes = 2;  // offset for header
  close_segment(CurrSegment);
  
  asgn_sym_numbers();

  for(i = BSSZP; i >= TEXT; --i)
  {
    open_segment(i);
    ass_sym_values();
    close_segment(i);
  }

  for(i = TEXT; i <= BSSZP; ++i)
  {
    open_segment(i);
    compute_seg_size();
    close_segment(i);
  }
  
  output_header();
  output_symtab();
  for (i = TEXT; i <= DATA; ++i)
  {
    open_segment(i);
    written = 0;
    out_frags();
    close_segment(i);
  }
  close(output_fd);
}

int gen_n_passes()
{
  return(1);
}

