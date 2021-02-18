/*  optimizer for cc65
    written : Nov. 97 by 42Bastian Schick (elw5basc@gp.fht-esslingen.de)
    
    42BS  - 42Bastian Schick
    
    last change :
    YY/MM/DD
    
    97/11/14  42BS  changed the source from C to C++, mainly because of some
                    neat features of C++ and not because off OOP.
    97/11/17  42BS  removed some bugs, added more ...
    97/11/19  42BS  some very stupid bugs removed ( == != = :( )
                    removed most of the ldau-optimization, because it is very risky and
                    the gain is nearly zero
    97/12/08  42BS  had to remove the postinc/dec optimization because it causes trouble
                    with function-prototypes.
    97/12/10  42BS  Zero-Byte stored behind the read-file because of a possible CR LF => LF
                    conversion
*/
#define VERSION "1.06"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

// helper-functions
int is_power_of_two(int n);
int xtoi(char *);
void write(register char *line);
void writeldy(int n);
void writeconst(char *l,int n);
void writejsr(char *l);
char * nextline(register char * currentline);
bool matchstr(register char * line,register char *str);
bool matchjsr(char *line,char * str);

// global variables
bool  verbose = false;
char *out;
char *line;

char LDX0[]="\tldx\t#0\n";
char LDY[]="\tldy\t#$00\n";
char AND[]="\tand\t#$00\n";
char ORA[]="\tora\t#$00\n";
char EOR[]="\teor\t#$00\n";
#define HN 7
#define LN 8
char INCAX[] = "incax1";
char DECAX[] = "decax1";
char dec2hex[]="0123456789ABCDEF";

/*****************************************************************************/
/* optimizer-functions                                                       */
/*****************************************************************************/

//
// find
//        ldax #xxx;
//        jsr asltos
// change it to
//        ldy #
//        jsr aslaxy
//
bool opt_asl()
{
char * l1, *l2;
int num;

  if ( !(l1 = nextline(line)) ) return false;

  if ( !(l2 = nextline(l1)) ) return false;
  
  if ( ! matchjsr(line,"pushax") ) return false;
  
  if ( ! matchstr(l1,"\tldax\t#") ) return false;

  if ( ! matchjsr(l2,"asltos") ) return false;

  if ( !(isdigit(l1[7]) && (num = atoi(l1 + 7))>0) ) return false;

  line = nextline(l2);

// shift > 15 => replace by loading 0
  if ( num > 15 )
  {
    write("\tldax\t#0");
    return true;
  } else
// remove zero-shifts
  if ( num == 0 )
  {
    return true;
  } else 
// replace by LDY...
  {
    writeldy(num);
    writejsr("aslaxy");
    return true;
  }
}
//
// find
//        ldax #xxx;
//        jsr asrtos
// change it to
//        ldy #
//        jsr asraxy
//
bool opt_asr()
{
char * l1,*l2;
int num;

  if ( !(l1 = nextline(line)) ) return false;

  if ( !(l2 = nextline(l1)) ) return false;
  
  if ( ! matchjsr(line,"pushax") ) return false;
  
  if ( ! matchstr(l1,"\tldax\t#") ) return false;

  if ( ! matchjsr(l2,"asrtos") ) return false;

  if ( !(isdigit(l1[7]) && (num = atoi(l1 + 7))>0) ) return false;

  line = nextline(l2);
  
// shift > 15 => replace by loading either 0 or -1
  if ( num > 15 )
  {
    write("\tldx\t#0");
    write("\ttay");
    write("\tbpl\t*+3");
    write("\tdex");
    write("\ttxa");
    return true;
  } else
// remove zero-shifts
  if ( num == 0 )
  {
    return true;
  } else
// replace by LDY...
  {
    writeldy(num);
    writejsr("asraxy");
    return true;
  }
}
//
// find : lbeq Lx
//        lbra Ly
//       Lx
// replace by
//        lbne Ly
bool opt_lbeq()
{
register char *l1,*l2;

  if ( !(l1=nextline(line)) ) return false;

  if ( !(l2=nextline(l1)) )  return false;

  if (matchstr(line,"\tlbeq\t") && matchstr(l1,"\tlbra\t") && l2[0]== 'L')
    if ( atoi(l2+1) == atoi(line + 7)  )
    {
      l1[3] = 'n'; l1[4] = 'e';
      write(l1);
      line = l2;
      return true;
    }
  return false;
}
//
// find constat mul,div,mod,and,or,xor,add,sub
//
char *tos_func[]=
      {"multos","divtos","modtos","or_tos","xortos","andtos","addtos","subtos",0};
char *axy_func[]=
      {"mulaxy","divaxy","modaxy","or_axy","xoraxy","andaxy","addaxy","subaxy",
       "aslaxy","asraxy",0};
      
char Operator[]="*/%|^&+-";

bool opt_math()
{
char *l1,*l2,*l3;
int num,i;
bool is_test = false;
char **f;

  if ( !(l1 = nextline(line)) ) return false;

  if ( !(l2 = nextline(l1)) ) return false;
  
  if ( ! matchjsr(line,"pushax") ) return false;
  
  if ( ! matchstr(l1,"\tldax\t#") ) return false;

  if ( ! isdigit(l1[7]) ) return false;

  if ( (num = atoi(l1+7)) > 255 ) return false;

  is_test = (l3 = nextline(l2)) && (matchstr(l3,"\tlbeq") || matchstr(l3,"\tlbne"));
    
  for ( f = tos_func,i=0; *f ; ++f,++i)
    if (matchjsr(l2,*f)) break;

  if (!*f) return false;

  switch ( i )
  {
  case 0:   // multos
    line = nextline(l2);
    if ( ! num )
    {
      write("\tldax\t#0");
      return true;
    }
    if ( (i = is_power_of_two(num)) )
    {
      writeldy(i);
      writejsr("aslaxy");
    }
    else
    {
      writeldy(num);
      writejsr("mulaxy");
    }
    return true;
  case 1:   // divtos
    line = nextline(l2);

    if ( ! num )
    {
      write("\tldax\t#0");
      return true;
    }
    if ( (i = is_power_of_two(num)) )
    {
      writeldy(i);
      writejsr("asraxy");
    }
    else
    {
      writeldy(num);
      writejsr("divaxy");
    }
    return true;  
  case 2:   // modtos
    line = nextline(l2);
    if ( ! num )
    {
      write("\tldax\t#0");
      return true;
    }
    if ( (i = is_power_of_two(num)) )
    {
      write(LDX0);
      writeconst(AND,num-1);
    }
    else
    {
      writeconst(LDY,num);
      writejsr("modaxy");
    }
    return true;

  case 3:   // or_tos
    line = nextline(l2);
    if ( num )
      writeconst(ORA,num);
    return true;
  case 4:   // xortos
    line = nextline(l2);
    if ( num && ! is_test)
      writeconst(EOR,num);
    else
    {
      writeconst(EOR,num);
      writejsr("tstax");
    }
    return true;
  case 5:   // andtos
    line = nextline(l2);
    if ( ! num )
    {
      write("\tldax\t#0");
      return true;
    }
    write(LDX0);
    writeconst(AND,num);
    return true;
  case 6:   // addtos
    line = nextline(l2);
    if ( num )
    {
      writeconst(LDY,num);
      writejsr("addaxy");
    }
    return true;
  case 7:   // subtos
    line = nextline(l2);
    if ( num )
    {
      writeconst(LDY,num);
      writejsr("subaxy");
    }
    return true;
  }
}
//
// find constat mul,div,mod,and,or,xor,add,sub
//
bool opt_math2()
{
char *l1,*l2,*l3;
char **f;
int i,num,val;

  if ( !(l1 = nextline(line)) ) return false;

  if ( !(l2 = nextline(l1)) ) return false;
  
  if ( ! matchstr(line,"\tldax\t#") ) return false;

  if ( ! matchstr(l1,"\tldy\t#") ) return false;

  for ( f = axy_func,i = 0; *f ; ++f,++i)
    if (matchjsr(l2,*f)) break;

  if (!*f) return false;

  if ( l1[6] == '$' )
    num = xtoi(l1+7);
  else
    num = atoi(l1+6);

  if ( isdigit(line[7]))  
  {
    val = atoi(line+7);
    switch (i)
    {
      case  0: val *= num; break;
      case  1: val /= num; break;
      case  2: val %= num; break;
      case  3: val |= num; break;
      case  4: val ^= num; break;
      case  5: val &= num; break;
      case  6: val += num; break;
      case  7: val -= num; break;
      case  8: val <<= num; break;
      case  9: val >>= num; break;
    }
    sprintf(line+7,"%d\n",val);
    write(line);
    line = nextline(l2);
    return true;
  }

char help[80]="\tldax\t#("; // use parenthesis !!
  
  strncpy(help+8,line+7,l1-line-1);
  l3=strchr(help,'\n');
  l3[0]=')';
  
  if (i<8)
    l3[1]=Operator[i];
  else
  {
    if (i == 8)   // replace shifts by multyplication
      l3[1]='*';
    else
      l3[1]='/';  // or division
    num = 1<<num;
  }
  sprintf(l3+2,"%d",num);
  write(help);
  line = nextline(l2);
  return true;
}  
//
// find constant push
//
bool opt_push()
{
char *l1;
int num;

  if ( ! (l1=nextline(line)) ) return false;

  if ( ! matchstr(line,"\tldax\t#") ) return false;
  
  if ( ! matchjsr(l1,"pushax") ) return false;

  if ( ! isdigit(line[7]) || (num = atoi(line+7))> 255 ) return false;

  if ( num )
  {
    line[4] = ' ';
    write(line);
    writejsr("pusha");
  }
  else
    writejsr("push0");
    
  line = nextline(l1);
  return true;
}
// find unsigned byte load: ldau ==/ldx #0 
//                                 \lda ...
// replace by lda ... if possible
bool opt_ldau()
{
char * l1;
  if ( !(l1 = nextline(line)) ||
       !matchstr(line,"\tldau") )
    return false;

  if ( matchjsr(l1,"pushax") )
  {
    line[4]=' ';
    write(line);
    writejsr("pusha");
    line = nextline(l1);
    return true;
  }
  return false;
}
//
// find post-increment/decrement dead-code and remove it
//
bool opt_postincdec()
{
char *l1,*l2,*l3,*l4;
bool found = false;

  if ( matchstr(line,"\tpha") &&
       (l1 = nextline(line)) &&
       matchstr(l1,"\tphx") )
  {
    l2 = l1;
    while ( (l3 = nextline(l2)) )
      if ( matchstr(l3,"\tplx") &&
           (l2 = nextline(l3)) &&
           matchstr(l2,"\tpla"))
      {
        found = true;
        break;
      }
      else
        l2 = l3;
        
    if ( found )
    {
      if ( !(l4 = nextline(l2)) || !(matchstr(l4,"\tlbra") ))
         return false;
         
      l2[0]=l3[0]=';';
      line = nextline(l1);
      return true;
    }
  } return false;
}
//
// search for : ldy #$...
//              jsr ...axy
// and change it to a implied version if possible

bool opt_implied()
{
char * l1;
char **f;
int i,num;

  if ( !(l1 = nextline(line))) return false;

  if ( !(matchstr(line,"\tldy\t#"))) return false;

  for ( f = axy_func,i=0; *f ; ++f,++i)
    if (matchjsr(l1,*f)) break;

  if (!*f) return false;

  if ( line[6] == '$' )
    num = xtoi(line+7);
  else
    num = atoi(line+6);

  if ( ((i == 8) || (i == 9))&& (num == 1) )
  {
    if (i == 8)
      writejsr("aslax");
    else
      writejsr("asrax");
    line = nextline(l1);
    return true;
  }
  
  if ( (i < 6) || (i > 7) || (num > 8) ) return false;
  
  if ( i == 6 )   // addaxy
  {
    if ( num == 1)
    {
      write("\tina");
      write("\tbne *+3");
      write("\tinx");
    }
    else
    {
      INCAX[5]=dec2hex[num];
      writejsr(INCAX);
    }
  }else if ( i == 7 )
  {
    DECAX[5]=dec2hex[num];
    writejsr(DECAX);
  }
  line = nextline(l1);
  return true;
}

struct opt_func {
  bool (* opt)(); // function
  char *name;     // info-name
  };
  
// these optimizer-function may be called multiple times

opt_func opts[]=
  { {opt_asl,"asl"},
    {opt_asr,"asr"},
    {opt_math,"math"},
    {opt_math2,"math2"},
    {opt_push,"const push"},
    {0,0}
  };

// these only once and at the end

opt_func opts2[]={
    {opt_implied,"implied"},
    {opt_postincdec,"post inc/dec"},
    {opt_lbeq,"lbeq"},
    {opt_ldau,"ldau"},
    {0,0}
   };
  
int main(int argc,char **argv);    
int main(int argc, char **argv)
{ 
  if ( argc < 2 || argc > 3 )
  {
    printf("Usage : xopt [-v] infile\n");
    exit(-1);
  }

  if ( argc == 3 )
    if ( !(verbose = argv[1][0] == '-' && argv[1][1]=='v') )
    {    
      printf("Usage : xopt [-v] infile\n");
      exit(-1);
    }
    else
      printf("Xopt for cc65 V "VERSION" "__DATE__"\n"
             "written by 42Bastian Schick (elw5basc@gp.fht-esslingen.de)\n");

  --argc;
//
// now get the input file
//
  FILE * inf;
  char * in_buf,*out_buf;
  long in_len;
  
  if (!(inf = fopen(argv[argc], "r")))
  {
    printf("xopt :Can't open %s\n", argv[argc]);
    exit(-1);
  }
    
  fseek(inf,0,SEEK_END);
  in_len = ftell(inf);
  fseek(inf,0,SEEK_SET);
  
  if ( (in_buf = (char *)malloc(in_len<<1)) == (char *)NULL ||
       (out_buf= (char *)malloc(in_len<<1)) == (char *)NULL )
  {
    if ( in_buf ) free(in_buf);
    
    printf("xopt : No memory (needed %ld bytes)!\n",in_len<<2);
    exit(-1);
  }
  bzero(in_buf,in_len<<1);
  in_len = fread(in_buf,sizeof(char),in_len,inf);
  
  *(in_buf+in_len) = 0; /* needed for CR LF => LF conversion ! */
  
  fclose(inf);
//
// here we go
//  
  int total_opt = 0,
       pass_opt = 0,
            opt = 0;            

  opt_func * func;
  
  do{
    pass_opt = 0;
    
    for ( func = opts ; *func->opt ; ++func)
    {
      line = in_buf;
      out  = out_buf;
      opt  = 0;
      while ( line && *line )
      {
        if ( (*func->opt)() )
          ++opt;
        else
        {
          write(line);
          line = nextline(line);
        }
      }
      if ( opt )      
      {
        char * help = in_buf;
        in_buf = out_buf;
        out_buf = help;
        if (verbose) printf("Optimization :%12s (%d)\n",func->name,opt);
        pass_opt += opt;
      }      
    }
    total_opt += pass_opt;
  }while( pass_opt);
//  
// process single-time optimizations
//
  for (func = opts2; *func->opt; ++func )
  {
    line = in_buf;
    out = out_buf;
    opt = 0;
  
    while ( line && *line )
    {
      if ( (*func->opt)() )
      {
        ++opt;
      }
      else
      {
        write(line);
        line = nextline(line);
      }
    }
    if ( opt  )
    {
      char * help = in_buf;
      in_buf = out_buf;
      out_buf = help;
      
      if (verbose) printf("Optimization :%12s (%d)\n",func->name,opt);
      
      total_opt += opt;
    }
  }
  
  if ( verbose && total_opt ) printf("total          (%d)\n",total_opt);
  else if ( verbose ) printf("Nothing done on %s !\n",argv[argc]);
//
// open output file, but first a temp. file !
//
FILE * outf;
  if ( (outf = fopen("xopt.tmp", "w")) == NULL)
  {
    free(in_buf);
    free(out_buf);
    printf("xopt: Can't open 'xopt.tmp' !\n");
    exit(-1);
  }
//
// write out the file
//
  fwrite(in_buf,sizeof(char),strlen(in_buf),outf);
  fclose(outf);
//
// release memory
//
  free(in_buf);
  free(out_buf);
//
// now , replace original by temp-file
//
  unlink(argv[argc]);
  rename("xopt.tmp", argv[argc]);

  
  return false;
}
//
// helper functions
//
void write(register char *line)
{
  for( ; *line && *line != '\n'; ++line,++out )
    *out = *line;
    
  if ( *line == '\n' )
    *out++ = *line++;
  else
    *out++ = '\n';
    
  *out = 0;
}  
void writeldy(int n)
{
    LDY[HN]=dec2hex[(n>>4) & 15]; LDY[LN]=dec2hex[n & 15];
    write(LDY);
}
void writeconst(char *l,int n)
{
    l[HN]=dec2hex[(n>>4) & 15]; l[LN]=dec2hex[n & 15];
    write(l);
}
void writejsr(char *l)
{
  strcpy(out,"\tjsr\t");
  out += 5;
  write(l);
}
char * nextline(register char * currentline)
{
  while ( *currentline )
  {
    if ( currentline[0] == '\n' && currentline[1] )
      return (currentline+1);
    ++currentline;
  }
  return (char *)0;
}
bool matchstr(register char * line,register char *str)
{
  while( *str  )
    {
      if (*line != *str) return(false);
      ++line;
      ++str;
    }
  return(true);
}

bool matchjsr(char *line,char * str)
{
  return(matchstr(line, "\tjsr\t") && (matchstr(line+5, str)));
}

/* misc. */

int is_power_of_two(int n)
{
int c = 0;
  if ( n == 0)
    return 0;
    
  while ( (n & 1) == 0 )
    {
    n >>= 1;
    ++c;
    }
  if ( n == 1)
    return c;
  else
    return 0;
}

int xtoi(register char *s)
{
  register i;

  i = (*s < 'A' ? *s - '0' : *s - '0' - 7)<<4;
  ++s;
  return (i | (*s < 'A' ? *s - '0' : *s - '0' - 7));
}
