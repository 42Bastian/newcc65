// macro-handling
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "macro.h"
#include "global.h"
#include "parse.h"

/* xpseudo.c*/
extern int do_pseudo();

/* opcode.c */
extern int opcode_p(char * name,int *  code,int * valid);
extern void assemble_op(int basecode,int valid,char ** args);

int read_line(FILE *f,char *l);
void save_pos(FILE *f);
void restore_pos(FILE *f);

/* a65.c */
extern FILE * inf;
extern int file_nbr;
//extern int line_nbr;

extern void gen_label();
/* parse */
void parse_line(char * line,struct parse * p,int macrodef);

void barf(char * msg,int arg1,int arg2,int arg3);
void fatal(char *);
int string_equal(char *s1, char *s2);


int InitMacro(void);
int ExpandMacro(void);
int HashMacro(char *name);
void ExitMacro(void);

void macro_expand(char *line,char *macro_line,
                  char macro_para[32][80],int count,
                  struct macroS *m);
int macro_read_line(FILE *inf,char *line);

/***********************/
int InitMacro(void)
{
  if ( (macroHeapBase = macroHeapNext = (char *)malloc(MACROHEAPSIZE)) == NULL)
  {
    fatal("Could not allocate macro-memory !");
  }

  macroHeapEnd = macroHeapBase + MACROHEAPSIZE;
  
  bzero(macroHashTab,sizeof(macroHashTab));
  bzero(macroHeapBase,MACROHEAPSIZE);

   macroCurrFile      = 0;
  macroCurrLine       = 0;
  macroCurrParameters = 0;
  macroCurrName       = 0;
  macroCurrPtr        = 0;
   
  return 1;
}

void ExitMacro(void)
{
  if ( macroHeapBase ) free(macroHeapBase);
}

//
// compute - hashvalue
//
int HashMacro(char * name)
{
  int hash = 0;
  while ( *name )
  {
    hash += hash ^ *name++;
  }
  return hash & (MACROHASHSIZE-1);
}
//
// check if a string is a macro-call
//
struct macroS *CheckMacro(char *name)
{
  int hash;
  struct macroS *p_m;

  hash = HashMacro(name);
  p_m = macroHashTab[hash];

//printf("checkm(%p)\n",p_m);

  while ( p_m )
  {
//    printf("checkm:name(%s)\n",p_m->name);
    if ( !strcmp(p_m->name,name) ) return p_m;
    p_m = p_m->next;
  }

  return 0;
}
//
// define a macro
//


int DefineMacro(char *name)
{
  int hash;
  int i;
  struct macroS **p_m;
  int size,endm_notfound;
  char *ptr;

  if ( disabled )
    return 1;

  if ( macroHeapNext + sizeof(struct macroS) > macroHeapEnd )
  {
    fatal("macro-memory exausted !");
  }
  
  hash = HashMacro(name);
  p_m = &macroHashTab[hash];
  if ( *p_m )
  {
    while ( (*p_m)->next )
       p_m = &(*p_m)->next;
       
    p_m = &(*p_m)->next;
  }
// p_m points now to the address where the next macro-address
// is stored

  *p_m = (struct macroS *)macroHeapNext;
  strcpy((*p_m)->name,name);
  (*p_m)->filenum = file_nbr;
  (*p_m)->linenum = line_nbr;

  ptr = (*p_m)->macro;

  macroHeapNext += sizeof(struct macroS);

// now search for ENDM and compute MACRO-size !
  endm_notfound = 1;
  while (!end_file && endm_notfound && macro_read_line(inf, line) )
	{
//	printf("DefineMacro:");
		++line_nbr;
    if ( verbose )
        printf("%4d:'%s'\n",line_nbr,line);
        
		parse_line(line, &p,1);
//		
// get length of line
//		
		size = 4;
		size += p.label ? strlen(p.label) : 0;
		size += p.opcode ? strlen(p.opcode) : 0; // 4 - spaces
		
		for(i = 0; i<32 && p.arg[i] ; ++i)
	    size += 1+strlen(p.arg[i]);              // strlen + ","
	   
	   size += (size & 1);
//    
// check for macro/endm
//
//	    printf("def:op(%s %s,%d)\n",p.opcode,p.arg[0],endm_notfound);
		if ( string_equal(p.opcode,"endm") )
		{
      --endm_notfound;
		}else if ( string_equal(p.opcode,"macro") )
		{
		  ++endm_notfound;
		}
//		
// copy pre-parsed line, skip "endm"
//		
		if (endm_notfound)
		{
//
// check space
//	    
  	  if ( (macroHeapNext += size) > macroHeapEnd )
      {
        fatal("Macro-buffer exausted !");
      }      
		  if (p.label){ strcpy(ptr,p.label); ptr += strlen(p.label);}
		  if (p.label_suffix){ strcpy(ptr,p.label_suffix); ptr += strlen(p.label_suffix);}
		  *ptr++=' ';
		  if (p.opcode){ strcpy(ptr,p.opcode);ptr += strlen(p.opcode); *ptr++=' ';}
		
		  for(i = 0; p.arg[i] && i<32; ++i)
		  {
		    strcpy(ptr,p.arg[i]); ptr+= strlen(p.arg[i]);
		    if ( i<32 && p.arg[i+1] ) *ptr++ = ',';
		  }
  		*ptr++ = 0;
  		save_pos(inf);
		}
	}
  *ptr++ = 0;
  if ( endm_notfound )
  {
    barf("reached EOF ,missing endm",0,0,0);
    return 0; 
  }

//  printf("next :%p end %p\n",macroHeapNext,macroHeapEnd);
	return 1;
}
//
//
char macro_line[256];

//
// execute macro
//
int do_macro()
{
  struct macroS *m ;
  char *ptr;
  int macro_line_nbr;
  int i,para_count;
  char macro_para[32][80];

  if ( verbose>1 ) printf("%4d:expanding(%s)\n",line_nbr,p.opcode);
  
  if (!(m = CheckMacro(p.opcode)) ) return 0; // this should not happen !!

  ++processing_macro;

  para_count = 0;
  for (i = 0; i<32 ; ++i)
  {
    if ( p.arg[i] )
    {
      strcpy(macro_para[i],p.arg[i]);
      ++para_count;
    }
    else
      macro_para[i][0]=0;
  }

  ++m->count;

  ptr = m->macro;
  macro_line_nbr = m->linenum;
  while( *ptr )
  {
    int code,class;
    int save_verbose = verbose;
    
    macro_expand(line,ptr,macro_para,para_count,m);
		ptr += strlen(ptr)+1;	  

    ++macro_line_nbr;

    if ( verbose > 1 ) // check  
      if (disabled)
        printf("%4d:<%s>\n",macro_line_nbr,line);
      else
        printf("%4d:'%s'\n",macro_line_nbr,line);

    parse_line( line ,&p,1);
//    
// this is for error-messages
//   
    macroCurrName = m->name;
    macroCurrFile = m->filenum;
    macroCurrLine = macro_line_nbr;
    macroCurrPtr  = &ptr;
//
// assemble line
//
    --verbose;
    
    if (p.opcode)
		{
		  if (opcode_p(p.opcode, &code, &class))
			{
			  if (!disabled)
			    assemble_op(code, class, &p.arg[0]);
			}
			else if ( CheckMacro(p.opcode) )
			{
			  if (!disabled)
			  {
 			    do_macro();
 			  }
			}
			else if (!do_pseudo())
		    barf("do_macro:Bogus line",0,0,0);
		}
	  else
	 	  if (!disabled)
			  gen_label();

	  verbose = save_verbose;
  }
//  printf("domacro:leavin..\n");
  if ( disabled )
  {
    barf("Missing endif befor endm !",0,0,0);
  }
  --processing_macro;
  return 1;
}

#define is_hex(a) (((a)>='0' && (a)<='9')||\
                   ((a)>='a' && (a)<='f')||\
                   ((a)>='A' && (a)<='F'))
                  
void macro_expand(char *line, char *macro_line,
                  char macro_para[32][80], int count,
                  struct macroS *m)
{
  int c;
  
  while( *macro_line )
  {
//
// define this, if you don't want parameter-replacement inside strings
//
#ifdef PROTECT_STRING  
    if (*macro_line == '"' )
    {
      *line++ = *macro_line++;
      while (*macro_line && *macro_line != '"' )
        *line++ = *macro_line++;
        
      if ( ! *macro_line )
      {
        barf("Unexpected end-of-line !",0,0,0);
      }
      *line++ = *macro_line++;
    }
    else
#endif
    
    if ( *macro_line != '\\' )
    {
      *line++ = *macro_line++;
    }
    else
    {
      ++macro_line;
      if (*macro_line == '#' )
      {
        sprintf(line,"%02d",count);
        line += 2;
      }
      else if( *macro_line == '?')
      {
        strcpy(line,m->name);
        line += strlen(m->name);
      }
      else if ( *macro_line == '^')
      {
        sprintf(line,"%03d",m->count);
        line += 2;
      }
      else if ( is_hex(*macro_line) )
      {
        if ( (c = *macro_line) > 'F')
          c &= 0xdf;
          
        if ( (c -= '0')  > 9 )
          c -= 7;
          
        strcpy(line,macro_para[c]);
        line += strlen(macro_para[c]);
      }
      else if ( *macro_line == '\\' )
      {
        *line++ = *macro_line++;
      }
      else
      { 
        barf("Unknown macro-parameter-type",0,0,0);
      }
      ++macro_line;
    } 
  }
  *line = 0;
}

//
// read a line either from input-file or macro-buffer
//
int macro_read_line(FILE *inf,char *line)
{
  if ( processing_macro )
  {
    strcpy(line,*macroCurrPtr);
    *macroCurrPtr += strlen(*macroCurrPtr)+1;
    return (*line != 0);
  }
  return read_line(inf,line);
}
