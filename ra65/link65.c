/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.

   Modified for use on *nix machines -Intruder  1993
  
   Cleaned up by 42Bastian Schick

*/

/* changes :
   DD MM YY	who	what
   12 08 98	42BS	Added -B to define the start of the BSS.
   			Example : -B200 -b2000 
			Result : BSS _and_ Stack are before Code !
			
*/
/* Linker for .obj files produced by ra65 */

#define VERSION "2.3b"
#define RUNTIME "runtime.run" /*42BS*/
#define BLL                   /* generate BLL-header */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#ifdef UNIX
#  include <unistd.h>
#endif

#ifndef O_BINARY
#  define O_BINARY 0
#endif

/* local headers */

#include "symtab.h"
#include "obj.h"
#include "libr.h"

/* prototypes */

void print_copyleft();

unsigned char read8();
USHORT read16();
void readskip(int nbytes);
void write8(unsigned char ch);
void write16(USHORT sh);
int  open_carefully(char * name,int mode);
int  read_header();
int  iswhite(char c);
int  read_token(FILE * f,char * buf);
char * clone(char * s);
void add_file(char * name);
struct obj_module * add_object_module(char * name);
struct sym * intern(char *str,int flags,int value);
void barf(char *msg, char *arg1, char *arg2, char *arg3);
int  reference(struct sym * sym,struct obj_module * m);
struct obj_module * digest_object_file(char * name,int module);
int  map_syms(int (* fun)(),char * funarg);
int  count_undef_sym(struct sym * sym,char *  arg);
int  sym_needed(struct sym * sym,char *  sym_name);
int  describe_sym(struct sym * sym,char *  str);
int  search_lib(struct sym * sym,int  ignore);
void add_library(char * name);
void process_segdata(struct obj_module * mod,int seg);

char *ExpandLibPath();
int FileExists(char *file);
void print_usage();
int gethex(char *x);
void printmodules(int s,FILE * mapf);


char * myname;
int  noruntime = 0;
int  debug = 0;
int  total_files = 0;

#define MAX_MODULES 512
#define MAX_LIBS 4
#define MAX_FILES 32

struct segmentS
  {
  int start,size,objsize;
  };
  
struct obj_module
	{
  int seg_type;       /* either OBJ_HEADER or LIBR_HEADER */
  char * seg_name;    /* the name of this file */
  int lib_nbr;        /* the library number */
  int lib_idx;        /* if libr file, the module nbr in the lib */
  struct segmentS seg[4];
  int nsyms;
  USHORT * symflags;
  struct sym ** syms; /* vector of pointers to symbols */
	};

struct obj_module * module[MAX_MODULES];
int  nmodules;

struct libr_module
	{
  int used;       /* if this module used */
  int n_defs;     /* n symbols defined this obj module */
  char ** lsyms;  /* vector of pointers to syms defined */
	};
	
struct libr
	{
	char * l_name;            /* filename this libr */
	int n_mods;		             /* n modules this libr */
	struct librmod * dict;    /* module dictionary this libr */
	struct libr_module * lm;  /* vector of libr_module structs */
	};

struct libr * lib[MAX_LIBS];  /* the libraries we're using */
int n_libs;

char * input_file[MAX_FILES];
int n_input_files;

/* stuff for hashing syms */
#define HASHMOD 128
struct sym * symtab[HASHMOD];

/* stuff for the output file */
int exe_start = 0x400;		/* executable start addr */
int exe_high_water;

int data_start;
int data_high_water;

int bss_start;
int bss_high_water;

int bsszp_start = 0x80;
int bsszp_high_water;

int stack_size = 1024;

char output_name[32] = "foo.com";
int input_fd, output_fd;

char map_p = 0;			/* write a map */
char verbose = 0;		/* print debug msgs */

#ifdef AUTOSTART
char start_p = 1;		/* write an autostart vector */
#endif

struct relfile rf;
struct librfile lf;

/* flags for searching libraries */
int n_undef;        /* current number undefined symbols */
int satisfied;      /* flag if anything satisfied this pass */
int libs_searched;

char litbuf[32+2];
char symbuf[32+2];
int symflags, symval;

/*---------------------------------------------------------*/
unsigned char read8()
{
static  unsigned char foo;
  read(input_fd, &foo, 1);
  return(foo);
}

USHORT read16()
{
  return(read8() | (read8() << 8)); 
}

void readskip(int nbytes)
{
 lseek(input_fd,nbytes,SEEK_CUR);
}

void write8(unsigned char ch)
{
  write(output_fd, &ch, 1);
}

void write16(USHORT sh)
{
  write8(sh & 0xFF);
  write8(sh >> 8);
}

int open_carefully(char * name,int mode)
{
  int fd;

  if (debug) printf("open('%s', %x)->", name, mode);
    
  fd = open(name, mode | O_BINARY , 0744);

  if (fd < 0)
  	barf("Can't open '%s'\n", name, "", "");

  if (debug) printf("%d\n", fd);
  
  return(fd);
}

/* read the header of a file into the appropriate data structure.
   return the header type word */
int read_header()
{
  USHORT typ;

  typ = read16();		/* get the header word */
  if (debug) printf("read_header: typ=%x\n", typ);
  switch (typ)
  {
  case NEW_OBJ_HEADER:
    rf.header = typ;
    rf.nb_sym = read16();
    rf.nb_seg = read16();
    rf.nb_segdata = read16();
    rf.n_sym = read16();
    rf.obj_data_size = read16();
    rf.data_size = read16();
    rf.bss_size = read16();
    rf.bsszp_size = read16();
   
//    printf("  obj: nb_sym %d. nb_seg %d. n_sym %d.\n", rf.nb_sym, rf.nb_seg, rf.n_sym);
    break;
	case OBJ_HEADER:
    rf.header = typ;
    rf.nb_sym = read16();
    rf.nb_seg = read16();
    rf.nb_segdata = read16();
    rf.n_sym = read16();
    rf.obj_data_size =
    rf.data_size =
    rf.bss_size =
    rf.bsszp_size = 0;
//    printf("  obj: nb_sym %d. nb_seg %d. n_sym %d.\n", rf.nb_sym, rf.nb_seg, rf.n_sym);
    break;
	case LIBR_HEADER:
    lf.l_header = typ;
    lf.type = read16();
    lf.n_modules = read16();
    lf.l_flags = read16();
// printf("  lib: type %d. n_mod %d. flg %x\n", lf.type, lf.n_modules, lf.l_flags);
    break;
  }
  return(typ);
}

int iswhite(char c)
{
  return((c == ' ') || (c == '\r') || (c == '\n') || (c == '\t'));
}

int read_token(FILE * f,char * buf)
{
  int c;

  for ( ; ((c = fgetc(f)) != EOF) && iswhite(c) ; )
    ;
  if (c == EOF) return(0);

  *buf++ = c;

  for ( ; ((c = fgetc(f)) != EOF) && !iswhite(c) ; )
    *buf++ = c;

  *buf = '\0';
  return(1);
}

char * clone(char * s)
{
  char * ss;

  ss = (char * )malloc(strlen(s) + 1);
  strcpy(ss, s);
  return(ss);
}

void add_file(char * name)
{
  char * buf;
  FILE * inf;

  if(debug) printf("add_file('%s')\n", name); 

  if (name[0] == '@')
  {
    buf = (char * )malloc(80); buf[0] = '\0';
    if ((inf = fopen(++name, "rb")) == NULL)
      barf("can't open indirect file %s\n", name, "", "");
    while (read_token(inf, buf) && (strlen(buf) > 0))
    {
      add_file(clone(buf));
    }
    fclose(inf);
  }
  else
  {
  /* maybe caononicalize name here? */
    input_file[n_input_files++] = name;
	}
}

struct obj_module * add_object_module(char * name)
{
  module[nmodules] = (struct obj_module * )malloc(sizeof(struct obj_module));

  bzero(module[nmodules],sizeof(struct obj_module));  

  module[nmodules]->seg_name = name;

  nmodules++;
  return(module[nmodules - 1]);
}

/* intern a symbol */
struct sym * intern(char *str,int flags,int value)
{
  int hash;
  char * p;
  struct sym * sym;

/*  printf("intern %s val %04x flags %04x\n", str, value, flags); */
  for (p = str, hash = 0 ; *p ; )
    hash = (hash + *p++) % HASHMOD;
    
  for (sym = symtab[hash] ; sym ; sym = sym->next)
    if (!strcmp(str, sym->name))
    {
      if (flags & DEFINED)
      {
        if ((sym->flags & DEFINED) && !(sym->flags & ABS))	/* kludge */
          printf("%s multiply defined\n", str);
        sym->value = value;
        sym->flags = flags;
      }
      return(sym);
    }
  sym = (struct sym *)malloc((sizeof (struct sym) + strlen(str) + 1));
  strcpy(sym->name, str);
  sym->nbr = -1;
  sym->flags = flags;
  sym->value = value;
  sym->next = symtab[hash];
  symtab[hash] = sym;
  return(sym);
}

void barf(char *msg, char *arg1, char *arg2, char *arg3)
{
  fprintf(stderr, (char *)msg, arg1, arg2, arg3);
  fprintf(stderr, "\n");
  exit(1);
}

/* a little frob to make referencing symbols easier */
int reference(struct sym * sym, struct obj_module * m)
{
  if (!(sym->flags & DEFINED))
  {
    printf("Undefined symbol (%s) referenced in module %s\n",
    sym->name, m->seg_name);
    ++n_undef;
  }
  return(sym->value);
}

/* read all interesting data about an object file, open on input_fd */

struct obj_module *  digest_object_file(char * name,int module)  
{
  struct obj_module * m;
  int j, k;

  m = add_object_module(name);
  
  m->seg[0].start = exe_high_water;
  m->seg[0].size = rf.nb_seg;
  m->seg[0].objsize = rf.nb_segdata;
  exe_high_water += rf.nb_seg;
  
  m->seg[1].start = data_high_water;
  m->seg[1].size  = rf.data_size;
  m->seg[1].objsize = rf.obj_data_size;
  data_high_water += rf.data_size;
  
  m->seg[2].start = bss_high_water;
  m->seg[2].size  = rf.bss_size;
  m->seg[2].objsize  = rf.bss_size;
  bss_high_water += rf.bss_size;

  m->seg[3].start = bsszp_high_water;
  m->seg[3].size  = rf.bsszp_size;
  m->seg[3].objsize  = rf.bsszp_size;
  bsszp_high_water += rf.bsszp_size;
      
  m->seg_type = rf.header;
  m->nsyms = rf.n_sym;
  m->syms = (struct sym ** )malloc(m->nsyms * sizeof(struct sym *));
  m->symflags = (USHORT *)malloc(m->nsyms * sizeof(USHORT));
  
  for (j = 0 ; j < rf.n_sym ; j++)
  {
    k = (int) read8();		/* size of symbol name */
    read(input_fd, symbuf, k);
    symbuf[k] = '\0';
    symval = read16();
    symflags = read16();
    m->symflags[j]=symflags;
    m->syms[j] = intern(symbuf, symflags, symval);
    if (symflags & THIS_SEG)
    {
/* defined here.  adjust value by segment base */
      m->syms[j]->nbr = module;	/* zzz should check for multiple def */
//      m->syms[j]->flags &= ~THIS_SEG;
      m->syms[j]->flags |= DEFINED;
    }
  }
  
  return(m);
}

void adjust_abs_symbols(struct obj_module *m)
{
  int j;
  USHORT symflags;
  
  for (j = 0 ; j < m->nsyms ; j++)
  {
    symflags = m->symflags[j];
        
    if (symflags & THIS_SEG)
    {
/* defined here.  adjust value by segment base */
      m->symflags[j] &= ~THIS_SEG;
      m->syms[j]->flags |= DEFINED;
      if (!(symflags & ABS))
      {
      if (symflags & SYMDATA)
        m->syms[j]->value += m->seg[1].start;
      else if (symflags & SYMBSS)
        m->syms[j]->value += m->seg[2].start;
      else if (symflags & SYMBSSZP)
        m->syms[j]->value += m->seg[3].start;
      else
        m->syms[j]->value += m->seg[0].start;
      }
      
    }
  }
}

/* map over all syms, applying fun to sym and arg, until fun returns t */
int map_syms(int (* fun)(),char * funarg)
{
  int i, ret;
  struct sym * sym;

  for (i = 0 ; i < HASHMOD ; i++)
  {
/*  printf("map_syms: bucket %d %x\n", i, symtab[i]);*/
    for (sym = symtab[i] ; sym ; sym = sym->next)
    {
/*    printf("  sym %x\n", sym);*/
      if ( (ret = (*fun)(sym, funarg)) )
        return(ret);
    }
  }
  return (1); /* dummy, never reached ! */
}

int count_undef_sym(struct sym * sym,char *  arg)
{
  if (!(sym->flags & DEFINED))
    n_undef++;
  return(0);
}

int sym_needed(struct sym * sym,char *  sym_name)
{
  return((!(sym->flags & DEFINED)) &&
         (strlen(sym->name) == *sym_name) &&
         (!strncmp(sym->name, sym_name+1, *sym_name)));
}

/* for debugging */
int describe_sym(struct sym * sym,char *  str)
{
  printf("%s: sym %x name %s\n", str, sym->nbr,sym->name);
  return(0);
}

/* search all libr structs for syms that satisfy refs from our symtab */
int search_lib(struct sym * sym,int  ignore)
{
  int libnum, modnum, symnum;
  struct libr_module * mod;
  char ** defname;

/* describe_sym(sym, "    search"); */
  if (!(sym->flags & DEFINED))		/* sym not defined? */
    for (libnum = 0 ; libnum < n_libs ; libnum++)
      for (modnum = 0 ; modnum < lib[libnum]->n_mods ; modnum++)
      {
        mod = &lib[libnum]->lm[modnum];
        for (symnum = 0, defname = mod->lsyms ;
          symnum < mod->n_defs ;
          defname++, symnum++)
        {
          if (!(strcmp(sym->name, *defname)))
          {
	   if (debug)
    {
      char buf[14];
      
      bcopy(lib[libnum]->dict[modnum].m_name,buf,12);
      buf[12]=0;
      printf("%d: %12s %d\n", modnum, buf, lib[libnum]->dict[modnum].m_nbytes);
    }

            mod->used = 1;
            satisfied = 1;
            return(0);		/* keep scanning syms */
          }
        }
      }
  return(0);
}

char * xsyms = 0;	/* ptr to temp buffer for syms */

void add_library(char * name)
{
  char * sym_p;
  int s, m;		/* symbol number, module number */
  unsigned char * fp;
  int flags;		/* symbol flags */
  struct libr * l;
  struct libr_module * mod;
  int i;

  l = lib[n_libs] = (struct libr * )malloc(sizeof(struct libr));
  
  l->l_name = name;	/* record the name */
  l->n_mods = lf.n_modules;

  if (!xsyms)
    xsyms = (char * )malloc(65536);	/* kludge */

  l->dict = (struct librmod *)malloc(lf.n_modules * sizeof(struct librmod));

  if(debug) printf("lib contains %d modules\n", lf.n_modules);

  for (m = 0 ; m < lf.n_modules ; m++)
  {
    read(input_fd, l->dict[m].m_name, 12);	/* get the module name */  	
    l->dict[m].m_nbytes = read16();	     	/* get module size */

    if (debug)
    {
      char buf[14];
      
      bcopy(l->dict[m].m_name,buf,12);
      buf[12]=0;
      printf("%d: %12s %d\n", m, buf, l->dict[m].m_nbytes);
    }
  }

  l->lm = (struct libr_module * )malloc(lf.n_modules * sizeof(struct libr_module));

  for (m = 0 ; m < lf.n_modules ; m++)
	{
	  
    if (debug) printf("digest module %d\n", m);
    
    if ( ((i = read_header()) != OBJ_HEADER) && (i != NEW_OBJ_HEADER) )	/* read obj header */
      barf("internal error, lib contains module type %04x\n", (char *)i, "", "");

    mod = &l->lm[m];
    mod->used = mod->n_defs = 0;

    if (rf.nb_sym > 2)
      read(input_fd, xsyms, rf.nb_sym - 2);	/* read the whole symtab */
      
    sym_p = xsyms;
    for (s = 0 ; s < rf.n_sym ; s++)
    {
    /* construct the flags word */
      fp = (unsigned char * )sym_p + *sym_p + 3;
      flags = *fp | (fp[1] << 8);
  /* printf("    sym %d ptr %x flags %x\n", s, fp, flags); */

      if (flags & THIS_SEG)		/* defined here? */
      {
      /* count this one */
        mod->n_defs++;
      }
      sym_p = sym_p + *sym_p + 5;
    }
  /* make a vector to hold all the names */
    if (debug) printf("  %d syms defined here\n", mod->n_defs);
    if (mod->n_defs > 0)
    {
      mod->lsyms = (char ** )malloc(mod->n_defs * sizeof(char **));
      sym_p = xsyms;
      for (s = 0, i = 0 ; s < rf.n_sym ; s++)
      {
        fp = (unsigned char * )sym_p + *sym_p + 3;
        flags = *fp | (fp[1] << 8);

        if (flags & THIS_SEG)		/* defined here? */
        {
          sym_p[*sym_p + 1] = '\0';
          mod->lsyms[i++] = clone(sym_p + 1);
        }
        sym_p = sym_p + *sym_p + 5;
      }
    }
    readskip(rf.nb_segdata + rf.obj_data_size);	/* skip the segment data */
  }			/* loop for module... */
/*  free(syms); */
  close(input_fd);
  n_libs++;		/* count the library */
}

void process_segdata(struct obj_module * mod,int segment)
{
int j, k, op;

  readskip(rf.nb_sym - 2);			/* skip symtab */

  if ( segment )
  {
    readskip(mod->seg[0].objsize);    /* skip TEXT-segment */
//    printf("skipping:%d\n",mod->seg[0].objsize);
  }
  for (j = 0 ; j < mod->seg[segment].objsize ; )
  {
    op = read8();		/* get an opcode */ 
    j += 1;			/* count it */

//    if (segment)
//      printf("  op %x\n", op); 
 
    switch (op & OP_GEN_MASK)
    {
      case OP_LIT:
      {
        if (op == 0)
          op = 32;
          
        read(input_fd, litbuf, op);
        
        write(output_fd, litbuf, op);
        j += op;
        break;
      }
      case OP_REL:
      {
        int start = mod->seg[(op>>2)& 0x03].start;
        
        switch(op & 0x23)
        {
          case OP_REL:
          {
            write16(read16() + start);
            j += 2;
            break;
          }
          case OP_REL_HI:
          {
            write8((read16() + start) >> 8);
            j += 2;
            break;
          }
          case OP_REL_LO:
          {
            write8((read16() + start) & 0xFF);
            j += 2;
            break;
          }
        }
        break;
      }
      default:		/* must be sym */
      {
        k = op & 0x1F;
        if (op & OP_SYM_EMASK)
        {
          k = (k << 8) | read8();
          j++;
        }
        
        switch (op & OP_SYM_MASK)
        {
          case OP_SYM:
          {
            int foo;

            foo = read16();
/* printf("generate word %X + '%s' value %X\n", foo,
   mod->syms[k]->name, mod->syms[k]->value); */

            write16(foo + reference(mod->syms[k], mod));
            j += 2;
            break;
          }
          case OP_SYM_HI:
          {
/*write16((read16() + reference(mod->syms[k], mod))>> 8);*/
            write8((read16() + reference(mod->syms[k], mod))>> 8);
            j += 2;
            break;
          }
          case OP_SYM_LO:
          {
/*write16((read16() + reference(mod->syms[k], mod,base))& 0xFF);*/
            write8((read16() + reference(mod->syms[k], mod))& 0xFF);
            j += 2;
            break;
          }
        }
        break;
      }
    }
  }
}


/*--------------------------------------------------------------------------*/

int main(int argc ,char *argv[])
{
  int i,j, k,segment;

  int total_libs = 0;
  char *Tmp;
  struct obj_module * m;

  struct sym * high_water;		/* special sym for top of executable */
  struct sym * stack_symbol;            /* special sym for stack start */
  struct sym * bss_symbol;    /* bss start */
  struct sym * bss_symbol2;    /* bss len */
  myname=argv[0];

/* init stuff */
  exe_high_water = exe_start;
  data_start = 0;
  data_high_water = 0;
  bss_start = 0;
  bss_high_water = 0;         /* first offset */
  bsszp_high_water = 0;

  bzero(symtab,HASHMOD * sizeof(struct sym *));

  n_input_files = nmodules = n_libs = 0;

 add_file(RUNTIME); /* always add runtime.obj  -Intruder.. */

/* grok args */
  argv++;
  while (--argc)
	{
  	if ((*argv)[0] == '-')
  	  switch ((*argv)[1])
      {
        case 'v':
        {
          ++verbose;
          break;
        }
        case 'b' :
        {
          exe_start = exe_high_water = gethex(*argv+2);
          break;
        }
        case 'z': case 'Z':
          bsszp_start = gethex(*argv + 2);
          break;
	case 'B':
	   bss_start = gethex(*argv +2);
	   break;
	   
        case 's':
          stack_size = atoi((*argv)+2);
          break;
        case 'o': case 'O':
        {
          strcpy(output_name,*++argv);
          --argc;
          break;
        }
        case 'm':
        {
          map_p = 1;
          break;
        }
//        case 'n': case 'N':
//        {
//          start_p = 0;
//          break;
//        }
        case 'r': case 'R':	/* -Intruder.. */
        {
          ++noruntime;
          input_file[0] = 0;
          break;
        }
        case 'h': case '?':
        {
          print_usage();
          exit(0);
        }
        case 'd':
        {
          ++debug;
          break;
        }
        /* more later... */
        case 'c':
        {
          print_copyleft();
          exit(0);
        }
  			default:
  			  printf("don't grok arg %s\n", *argv);
      }
  	else
  	{
  	  add_file(*argv);
  	  total_files++;
  	}
  	argv++;
	}

  if (verbose)
    fprintf(stderr, "Link65 v %s of %s\n", VERSION,__DATE__);

  if(!total_files)
  {
    fprintf(stderr, "%s: No input files specified\n", myname);
    exit(0);
  }

  

/* intern special symbols */
  high_water = intern("__freemem", DEFINED | ABS, 0);
  stack_symbol = intern("__stacktop", DEFINED | ABS, 0);
  bss_symbol = intern("__bss",DEFINED|ABS,0);
  bss_symbol2 = intern("__bsslen",DEFINED|ABS,0);

/* Read each input file, classifying file type.  For obj files, read each
   object header, recording segment size, start addr etc, intern symbols
   for this module.  For librs, digest each module as though it were a
   distinct obj file, but leave the 'used' bit in the struct clear.
*/

  for (i = 0 ; i < n_input_files ; i++)
  {			/* search in lib dir -Intruder */
    if(!input_file[i])
      continue;
      
    input_file[i] = ExpandLibPath(input_file[i]);
    input_fd = open_carefully(input_file[i], O_RDONLY);
    j = read_header();
    if (j == OBJ_HEADER || j == NEW_OBJ_HEADER)
    {
      m = digest_object_file(input_file[i], i);
    }
    else if (j == LIBR_HEADER)
    {
      ++total_libs; /* count # of libs */
      /* just record the name */
      add_library(input_file[i]);
    }
    else
      barf("%s is not an object file or library: header word %04x\n",
            module[i]->seg_name, (char *) &j, "");
    close(input_fd);
  }

/* If no lib specified, use std-C library - Added by Intruder - Oct 1993 */
  if(!total_libs)
  {
    Tmp = (char *)ExpandLibPath("c.olb");
    if(FileExists(Tmp))
    {
      if( debug )
        fprintf(stderr, "Tmp: %s\n", Tmp);
      input_fd = open_carefully(Tmp, O_RDONLY);
	    j = read_header();
	    if (j != LIBR_HEADER) barf("Error !",0,0,0);
	    add_library(Tmp);
	  }
  }
/**/
/* if there are any undefined symbols, search libraries for them */
/**/
  satisfied = 1;		/* hot-wire loop */
  for( ; (satisfied != 0) ; )
  {
    satisfied = 0;
    map_syms(search_lib,(char *)NULL);

    if (satisfied)
		/* walk all librs, digesting modules that are used */
    {
      for (i = 0 ; i < n_libs ; i++)
      {
        input_fd = open_carefully(lib[i]->l_name, O_RDONLY);
        read_header();

      /* skip dictionary */
        readskip(lf.n_modules * 14);

        for (j = 0 ; j < lf.n_modules ; j++)
        {
          struct libr_module * lmod;
				
          if ((k = read_header()) != OBJ_HEADER && k != NEW_OBJ_HEADER)
            barf ("internal error, type %04x",(char *) &k, "", "");
            
          lmod = &lib[i]->lm[j];
          
          if (lmod->used)
          {
            m = digest_object_file(lib[i]->l_name, 0);

            m->seg_type = LIBR_HEADER;
            m->lib_idx = j;    // module-index
	           m->lib_nbr = i;   // library-index
            readskip(rf.nb_segdata+rf.obj_data_size);
            lmod->used = 0;
/*            free(lmod->syms);	*/
            lmod->lsyms = NULL;
            lmod->n_defs = 0;
          }
          else
          {
            readskip(rf.nb_segdata + rf.obj_data_size + rf.nb_sym - 2);
          }
        }
        close(input_fd);
      }
    }
  }

/* set values for special symbols */
  data_start = exe_high_water;
  if ( ! bss_start )
    bss_start  = data_start + data_high_water;
  
  stack_symbol->value = bss_start + bss_high_water + stack_size;
  high_water->value   = stack_symbol->value + 1; 
  bss_symbol->value   = bss_start;
  bss_symbol2->value  = bss_high_water;

  if (verbose)
  {
    printf("TEXT : %04x (%5d bytes)\n"
           "DATA : %04x (%5d bytes)\n"
           "BSS  : %04x (%5d bytes)\n"
           "ZERO : %04x (%5d bytes)\n"
           "STACK: %04x (%5d bytes)\n",
           exe_start, exe_high_water-exe_start,
           data_start, data_high_water,
           bss_start, bss_high_water,
           bsszp_start,bsszp_high_water,
           stack_symbol->value, stack_size);
  }

  if ( bsszp_start + bsszp_high_water > 0xff )
  {
    if (bsszp_start == 0x80)
      barf("Zeropage overun !\nTry using -z%2x !",(char *)(0xff-bsszp_high_water),0,0);
    else
      barf("Zeropage overun !","","","");
  }

/* all modules now digested.  read each seg data, interpreting fragment types */

  output_fd = open_carefully(output_name, O_WRONLY | O_CREAT | O_TRUNC);
/* write the atari executable header */
#ifdef BLL
  write16(0x0880);
  write8(exe_start >>8);
  write8(exe_start);
  write8((exe_high_water - exe_start + data_high_water + 10) >> 8);
  write8( exe_high_water - exe_start + data_high_water + 10);
  write16(0x5342);
  write16(0x3339);
#else
  write16(0xFFFF);
  write16(exe_start);
  write16(exe_high_water /*- 1 42BS */);
#endif
  
  for (i = 0 ; i < nmodules ; i++)
	{
	  m = module[i];
    m->seg[1].start += data_start;
    m->seg[2].start += bss_start;
    m->seg[3].start += bsszp_start;
    if ( verbose)
      printf("module:%-34s TEXT %04x DATA %04x BSS %04x BSSZP %02x\n",m->seg_name,
              m->seg[0].start,m->seg[1].start,m->seg[2].start,m->seg[3].start);
            
    adjust_abs_symbols(m);
	}
	
  for(segment = 0; segment < 2; ++segment)
  {
    char * curr_lib = NULL;        /* current library file open */
    int curr_idx = -1;             /* current lib idx, if lib */
  
    if (debug) printf("Modules :%d\n",nmodules);
  
    for (i = 0 ; i < nmodules ; i++)
    {
      m = module[i];
      
      if ((m->seg_type == LIBR_HEADER) &&
           curr_lib &&
          (!strcmp(m->seg_name, curr_lib)) &&
          (curr_idx < m->lib_idx))
      {
    	/* skip modules til find the place in lib where this one starts */
        for ( ; curr_idx < m->lib_idx ; ++curr_idx)
        {
          readskip(lib[m->lib_nbr]->dict[curr_idx].m_nbytes);
        }
      }
      else
      {
        if (curr_lib)		/* we have a library open? */
        {
          close(input_fd);
          curr_lib = NULL;
        }
    	/* just open the file */
    	  input_fd = open_carefully(m->seg_name, O_RDONLY);
        if (m->seg_type == LIBR_HEADER)
        {
          curr_lib = m->seg_name;
          read_header();
          readskip(lf.n_modules * sizeof(struct librmod));
          curr_idx = 0;
    	/* skip modules til find the place in lib where this one starts */
          for ( ; curr_idx < m->lib_idx ; curr_idx++)
          {
	           if (debug) printf("Skipping %d\n",lib[m->lib_nbr]->dict[curr_idx].m_nbytes);
            readskip(lib[m->lib_nbr]->dict[curr_idx].m_nbytes);
          }
        }
      }
      read_header();
      process_segdata(m,segment);
      
      if (curr_lib)
        ++curr_idx;
      if (!curr_lib)			/* if a libr, leave it open for now */
        close(input_fd);
  	}
  }
#ifdef AUTOSTART
  if (start_p)				/* write autostart vec? */
  {
    write16(0x02E0);		/* autostart vec goes at #x2E0 */
    write16(0x02E1);
    write16(exe_start);		/* write the address */
  }
#endif
  close(output_fd);
  

/* print map, if desired */

  if (map_p)
  {
    FILE * mapf;
    char * p;
    struct sym * sym;

    if ( (p = strchr(output_name,'.') ) )
      *p = 0;
      
    strcat(p, ".map");
	
    mapf = fopen(output_name, "w");
    
    if (debug)
      printf("map '%s'->%x\n", output_name,(unsigned int)mapf);
      
     fprintf(mapf,"R - relative A - absolute\n"
                  "T - text     Z - zeropage\n");
/**********************/
    fprintf(mapf, "text-segment base address %04x\n", exe_start); 
    printmodules(0,mapf);
/**************/
    fprintf(mapf, "data-segment base address %04x\n", data_start);
    printmodules(1,mapf);
/**************/
    j = 99;

    for (i = 0 ; i < HASHMOD ; i++)
    {
      char c;
      
      for( sym = symtab[i]; sym ; sym = sym->next)
      {
        if (j > 1)
        {
          fprintf(mapf, "\n");
          j = 0;
        }
        switch ( sym->flags & 0xf0 )
        {
          case SYMTEXT :c='T'; break;
          case SYMDATA :c='D'; break;
          case SYMBSS  :c='B'; break;
          case SYMBSSZP:c='Z'; break;
          default:
            c='?';
        }
        fprintf(mapf, "%-24s%04x %c%c ", sym->name, sym->value,
        sym->flags & DEFINED ? (sym->flags & ABS ? 'A' : 'R') : 'U',c);
//	fprintf(mapf, "%-24s%04x %02x %c ", sym->name, sym->value,
//       sym->flags ,c);

        j++;
      }
    }
    fclose(mapf);
  }
    
/* 42BS */
  if (n_undef)
    return 1;
  else
    return 0;
/********/
}

/**
*** things added by Intruder below here.....
**/
char *ExpandLibPath(char *name)
{
  char *path;
  char *libpath;
  struct stat st;

  if(!stat(name, &st)) /* search current dir first.. */
   return name;

  path = (char *)malloc(512);
  if(!path)
   {
     fprintf(stderr, "ExpandLibPath(): can't allocate memory\n");
     return name;
   }

  libpath = (char *)getenv("CC65LIB");
  if(libpath)
   {
   
     strncpy(path, libpath, 512 - (strlen(name) + 5));

#ifndef MESSDOS
     if(path[strlen(path)-1] != '/')
       strcat(path, "/");
#else
     if(path[strlen(path)-1] != '\\')
       strcat(path, "\\");
#endif    
   }

  strcat(path, name);
  if(!stat(path, &st))
   return (char *)path;
 
/*  fprintf(stderr, "ExpandLibPath(): file not found (%s)\n", path); */
  return (char *)name;
}

int FileExists(char *file)
{
 struct stat st;
 if(!stat(file, &st))
  return 1;
 return 0;
}

void print_usage()
{
  fprintf(stderr, "%s %s of %s\n", myname, VERSION,__DATE__);
  fprintf(stderr, "Usage: %s [-vbszor] [<runtime>] <object-file(s)> [<lib(s)>]\n", myname);
  fprintf(stderr, "	-v	   verbose\n");
  fprintf(stderr, "	-b<addr>   set base address (hex, no space)\n");
  fprintf(stderr, "     -B<addr>   set BSS-base (hex, no space)\n"
                  "                Note : stack is alway behind the BSS !!!\n");
  fprintf(stderr, "	-s<addr>   set stack size (decimal, no space)\n");
  fprintf(stderr, "	-z<addr>   set start of zeropage BSS (hex)\n");
  fprintf(stderr, "	-o output  output name\n");
  fprintf(stderr, "	-r	   don't use standard runtime (%s)\n", RUNTIME);
  fprintf(stderr, "	-c         print copyright notice\n");
}
/*********************/
int gethex(char *x)
{
  int base;
  int d;

  d = base = 0; /* 42BS */

  while ( *x == ' ')
    ++x;
   
  while ( (d = *x++) )
  {
    if (d < 'A')
      d = d - '0';
    else
      d = (d & 0xDF) - 'A' + 10; /*42BS*/
    base = (base << 4) + d; /*42BS*/
  }
  return base;
}
/*******/
void printmodules(int s,FILE * mapf)
{
  int i,j;
  struct obj_module * m;
  char *p,*q;
  char buff[512];
  
  for (i = 0 ; i < nmodules ; i++)
  {
    m = module[i]; 
       
    if ( m->seg[s].size )
    {
       strcpy(buff, m->seg_name);
  /* if library, append libr element name */
      if (m->seg_type == LIBR_HEADER)
      {
        p = buff + strlen(buff);
        *p++ = '(';

        q = (char * )&lib[m->lib_nbr]->dict[m->lib_idx].m_name;

        for (j = 0 ; ((j < 12) && (*q != ' ')) && *q ; j++)
          *p++ = *q++;

        *p++ = ')';
        *p='\0';
      }
      fprintf(mapf, " %-48s (%04x to %04x = %5d bytes)\n",
                    buff, m->seg[s].start, 
                    m->seg[s].start + m->seg[s].size-1,
                    m->seg[s].size);
    }
  }
}
