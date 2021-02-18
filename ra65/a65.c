
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.

   Modified for use on *nix machines -Intruder  1993
*/
/* 97/01/31 removed native stuff 42BS*/

/*
   A65:  a macro assembler for 6502's.
 
   a65 [-o foo.obj] [-l foo.l] f1.m65 f2.m65...
*/

//#define VERSION "1.0p4 [*nix port by Intruder and 42BS]"
#define VERSION "1.2c"

/* #define LIST ... turn this on for listing code */
/* #define EITHER .. turn this if ever get around to making the 
                     assembler produce either rel or abs files */
 
#include <stdio.h>
#include <unistd.h>

#define MAIN 1

#include "symtab.h"
#include "parse.h"
#include "util.h"
#include "global.h"

char * arg_value(char * args[],int * arg_idx, int n_args);

/* copyleft.c */
extern void print_copyleft();

/* xgen.c*/
void gen_o_close();
int  gen_n_passes();
void init_gen();
void gen_o_open(char ** name,char * source_name);
void gen_label();

/* xpseudo.c*/
int do_pseudo();

/* opcode.c */
int  opcode_p(char * name,int *  code,int * valid);
void assemble_op(int basecode,int valid,char ** args);

/* symtab.c */
void init_sym();
void dump_syms();
int  find_udef();

/* error.c */
void barf(char * msg,int arg1,int arg2,int arg3);

/* list.c */
void init_list();
void list_line(int override);

/* parse.c */
void parse_line(char * line,struct parse * p,int macrodef);

/* macro.c */
int  InitMacro(void);
void ExitMacro(void);
struct macroS *CheckMacro(char *name);
int  do_macro();

/* version.c */
void version(char *,char *);


/************/
FILE * inf;
FILE * outf;
char * in_name[64];
int n_files;
char * out_name;

#ifdef DEBUG
char * sym_name;
#endif

extern int _start;

/* util fun for grokking arglist */
char * arg_value(char * args[],int * arg_idx, int n_args)
{
  int i;
  
  i = *arg_idx;
  if (args[i][2])               /* was there something after the '-x'? */
    return(args[i] + 2);        /* yes, just return that */

  i++;                          /* try the next arg */
  if (i >= n_args)              /* no next arg? */
    return(NULL);

  if (*args[i] == '-')          /* next arg not a value? */
    return(NULL);

  *arg_idx = i;                 /* put updated value back */
  return(args[i]);              /* and return this arg */
}


int main(int argc,char ** argv)
{
  int i, local_pass, fidx, code, class;

  InitMacro();
/*
  printf("base = %X\n", &_start);
*/

  n_files = 0;                  /* no files yet */
#ifdef LIST
  list_p = 0;                   /* no listing */
#endif
#ifdef DEBUG
  sym_name = 0;                 /* no sym file name yet */
#endif
#ifdef EITHER
  rel_p = 0;                    /* no relocatable output */
#endif
  verbose = 0;
  

  for (i = 1 ; i < argc ; i++)
  {
/*  printf("considering arg %d '%s'\n", i, argv[i]); */
    if (argv[i][0] == '-')
    {
      switch (argv[i][1])
      {
#ifdef LIST
        case 'l':
        {
          list_p = 1;
          list_name = arg_value(argv, &i, argc);
          break;
        }
#endif
        case 'o':
        {
          out_name = arg_value(argv, &i, argc);
          break;
        }
#ifdef DEBUG
        case 's':
        {
          sym_name = arg_value(argv, &i, argc);
          if (!sym_name)
            sym_name = "foo.sym";
          break;
        }
#endif
#ifdef EITHER
/* this is always on for this version of the assembler */
        case 'r': case 'R':
        {
          rel_p = 1;
          if (verbose)
            printf("Relocatable output\n");
          break;
        }
#endif
        case 'v':
        {
          verbose++;
          break;
        }
        case 'n':
        {
          print_copyleft();
          exit(0);
        }
      }
    }
    else
    {
      in_name[n_files] = frob_name(argv[i]);
      n_files++;
      if (verbose)
        printf("Input file %d %s\n", n_files, argv[i]);
     }
  } 

  if (verbose)
    version("ra65",VERSION);

  if (n_files < 1)
  {
    printf("Usage: ra65 [-losvn] <file>\n");
    exit(1);
  }


  init_sym();
#ifdef LIST
  init_list();
#endif
  init_gen();

  for (local_pass = 0 ; local_pass < gen_n_passes() ; local_pass++)
  {
    errcount = 0;
    pass = local_pass;
    
    pc = 0;
    disabled = 0;
    CurrSegment = TEXT;
    
    output_p = (pass == (gen_n_passes() - 1));
    if ( output_p )
    {
      gen_o_open(&out_name, in_name[0]);
      
    }
    for (fidx = 0 ; fidx < n_files ; fidx++)
    {
      end_file = 0;
      inf = fopen(in_name[fidx], "r");
      CurrentFile = in_name[fidx];
// printf("open('%s')->%X\n", CurrentFile, inf);
      line_nbr = 0;
      file_nbr = fidx;
      while (!end_file && read_line(inf, line))
      {
        line_nbr++;

        if (verbose)
          if (disabled)
            printf("%4d:<%s>\n", line_nbr, line);
          else
            printf("%4d:'%s'\n", line_nbr, line);
#ifdef LIST
        line_listed_p = 0;
#endif
        obj_count = 0;
        parse_line(line, &p,0);
        if (p.opcode)
        {
          if (opcode_p(p.opcode, &code, &class))
          {
            if (!disabled)  assemble_op(code, class, &p.arg[0]);
          }
          else if ( CheckMacro(p.opcode) )
          {
            if (!disabled) do_macro();
          }
          else if (!do_pseudo())
            barf("Bogus line",0,0,0);
        }
        else
          if (!disabled)  gen_label();

#ifdef LIST
        list_line(0);
#endif
      }
      fclose(inf);
    }
#ifdef DEBUG
    if (sym_name) dump_syms();
#endif
    if (output_p) gen_o_close();

    if ( (i = find_udef()) )
      printf("%d undefinded local label(s) !\n",i);

    if (errcount) printf("%d error(s) !\n",errcount);
  }
  if ( errcount )
  {
    unlink(out_name);
    return -1;
  }
  ExitMacro();
  return 0;
}
