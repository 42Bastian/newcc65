
/* Symbol table manipulation routines */

#include <stdio.h>
#include <string.h>

#include "cc65.h"
#include "error.h"
#include "code-65.h"

extern int debug;
extern int maxloc;
extern int tmpx, tmpy;
 
/* internal prototypes */
void addsfld(struct hashent * psym,char * tarray,int offset,struct hashent * sn,struct hashent *son);
int  addstag(struct hashent * psym,int sz);
struct hashent * addsym(char * sym);
struct hashent * findsym(char * sym);
void addglb(struct hashent * psym,char * tarray,int sc);
void addloc(struct hashent * psym,char * tarray,int sc,int offs);
void eraseloc();
int  hash(char * s);
char * Gmalloc(int nbytes);
char * Lmalloc(int nbytes);
int  SizeOf(char * tarray);
int  PSizeOf(char * tptr);
void dumpglbs();
void dumploc(struct hashent * pfunc);
void dumpnams();
void dumpstruct();
void encode(char *p,int w);
int  decode(char * p);

/* external prototypes */
/*main.c*/
void ptype(struct hashent * psym,char * tarray);

/*
  addsfld( psym, tarray, offset )
  Add a struct field definition to symbol table.
*/
void addsfld(struct hashent * psym,char * tarray,int offset,struct hashent * sn,struct hashent * son)
{
char * tptr;
//struct hashent * ps;

  if (psym->flag.g != 0 && psym->s_ptr == sn)
  {
    MultDef("component");
  }
  if (psym->flag.g)
  {
    struct hashent * psym2 = psym;
    struct hashent ** hptr;

printf("Multiused component\n");

    hashval = hash(psym->name);
    psym = (struct hashent *) Gmalloc(sizeof(struct hashent) + strlen(psym2->name));
    bzero(psym, sizeof(struct hashent));
    strcpy(psym->name, psym2->name);
  /*
    Add a symbol entry to the hash table.  hashval set by findsym above.
  */
    hptr = &htab[hashval];
    psym->ptr = *hptr;
    *hptr = psym;
    psym->typeptr = son;
  }

  tptr = Gmalloc(strlen(tarray) + 1);
  strcpy(tptr, tarray);

  psym->flag.g = SC_SFLD;
  psym->tptr.g = tptr;
  psym->data.g = offset;
  psym->s_ptr = sn;

//  printf("addsfld %s of struct %s of type ",psym->name,psym->s_ptr->name);
//  if ( psym->typeptr )
//    printf("%s\n",psym->typeptr->name);
//  else
//    printf(" standard.\n");
}
/*
  addstag( psym, sz )
  Define a struct/union tag to the symbol table.  Returns index into
  structs array.
*/
int addstag(struct hashent * psym,int sz)
{

  if (psym->flag.g)
  {
  /*
     tag already declared; check for multiple declare
  */ 
    if (((psym->flag.g & 0xFF) != SC_STAG) ||
        ((sz != 0) && (psym->data.g != 0)))
    {
	    MultDef("tag");
	  }
    /*
       define struct size if sz != 0
    */
    if (sz != 0)
	  {
	    psym->data.g = sz;
	  }
    return 0;
  }

  psym->flag.g = SC_STAG;
  psym->data.g = sz;
  return 0;
}

/*
  addsym( sym )
  Add a symbol to the symbol table.
*/
struct hashent * addsym(char * sym)
{
  struct hashent ** hptr;
  struct hashent * psym;

  if ( (psym = findsym(sym)) )
    {
      return (psym);
    }

  /*
    Allocate storage for hash table entry
  */
  psym = (struct hashent *) Gmalloc(sizeof(struct hashent) + strlen(sym));
  bzero(psym, sizeof(struct hashent));
  strcpy(psym->name, sym);

  /*
    Add a symbol entry to the hash table.  hashval set by findsym above.
  */
  hptr = &htab[hashval];
  psym->ptr = *hptr;
  return (*hptr = psym);
}

/*
  findsym( sym )
  Look up sym in hash table and return ptr to entry.
*/
struct hashent * findsym(char * sym)
{
struct hashent * p;

  hashval = hash(sym);
  for (p = htab[hashval]; p != NULL; p = p->ptr)
  {
    if (!strcmp(p->name, sym))
    {
	    return (p);
	  }
  }
  return (NULL);
}
/*
  findsfld( sym )
  Look up sym in hash table and return ptr to entry.
*/
struct hashent * findsfld(char * sym,struct hashent * parent)
{
struct hashent * p;

  hashval = hash(sym);
  for (p = htab[hashval]; p != NULL; p = p->ptr)
  {
    if (!strcmp(p->name, sym))
    {
    printf("findsfld: %s",p->name);
      if ( parent == p->s_ptr )
      {
	      return (p);
	    }
	  }
  }
  return (NULL);
}

/*
  addglb( psym, tarray, sc )
  Add a global symbol to the symbol table
*/
void addglb(struct hashent * psym,char * tarray,int sc)
{
char * tptr;
int sz1;

  if (psym->flag.g != 0)
  {
  /*
  * symbol entered previously.
  */
    if (psym->flag.g & SC_STRUCT)
    {
      MultDef("global symbol");
      return;
    }

    if ((tarray[0] == T_ARRAY) &&
        (((sz1 = decode(psym->tptr.g + 1)) == 0) ||
        (decode(tarray + 1) == 0)))
    {
      if (strcmp(psym->tptr.g + 4, tarray + 4) != 0)
      {
        MultDef("global symbol");
        return;
      }
      if (sz1 == 0)
      {
        encode(psym->tptr.g + 1, decode(tarray + 1));
      }
    }
    else if (strcmp(psym->tptr.g, tarray) != 0)
    {
      MultDef("global symbol");
      return;
    }
/*
if (((tarray != NULL) && (strcmp(psym->tptr.g, tarray) != 0)) ||
  (psym->flag.g & SC_STRUCT)) {
  Error("multiply defined global symbol");
  return;
  }
*/
    psym->flag.g |= sc;
    return;
  }

  tptr = Gmalloc(strlen(tarray) + 1);
  strcpy(tptr, tarray);

  psym->flag.g = sc;
  psym->tptr.g = tptr;
  psym->data.g = (int) &psym->name;

  psym->link = glvptr;
  glvptr = psym;
}

/*
  Add a local symbol.
*/
void addloc(struct hashent * psym,char * tarray,int sc,int offs)
{
char * tptr;

  /*
    Process function declarations and externs inside of function.
  */
  if ((tarray != NULL) && (tarray[0] == T_FUNC))
    {
      if ((sc & (SC_DEFAULT | SC_GLOBAL | SC_EXTERN)) == 0)
	{
	  Warning("function must be extern");
	}
      sc = SC_GLOBAL;
    }
  if (sc & (SC_GLOBAL|SC_EXTERN))
    {
      addglb(psym, tarray, sc);
      return;
    }

  /*
  if ((tarray != NULL) && (tarray[0] == T_FUNC))
      {
      addglb(psym, NULL, SC_DEFINED | SC_GLOBAL);
      return;
      }
  */
  if (psym->flag.l != 0)
    {
      MultDef("local symbol");
      return;
    }
  if (tarray != NULL)
    {
      tptr = Lmalloc(strlen(tarray) + 1);
      strcpy(tptr, tarray);
    }
  else
    {
      tptr = NULL;
    }

  psym->flag.l = sc | SC_DEFINED;
  psym->tptr.l = tptr;
  psym->data.l = offs;

  lvtab[lovptr++] = psym;
}

/*
  eraseloc()
  Erase local variables.
*/
void eraseloc()
{
int i;

  i = 0;
  while (i < lovptr)
    {
      lvtab[i++]->flag.l = 0;
    }
  lovptr = 0;
  lsptr = locspace;
}

/*
  hash( s )
  Compute hash value of string s.
*/
 
int hash(register char * s)
{
register int h = 0;

  while (*s)
    {
      h = (h<<1) ^ (*s++);
    }
    
  h &= HTABSZ-1;
  
  return (h );
}

#ifdef help_1
/* helper fun for malloc'ers */
xmalloc(int nbytes,char ** ptr_p,char *  limit)
{
char * ptr;
char * new_ptr;

  if ((new_ptr = (ptr = *ptr_p) + nbytes) > limit)
    return (0);
  else
    {
      *ptr_p = new_ptr;
      return (ptr);
    }
}

#endif

/*
  Gmalloc( nbytes )
  Allocate memory from the global space table.
*/
char * Gmalloc(int nbytes)
#ifndef help_1
{
char * p;

 
  /* nbytes = (nbytes + sizeof(int) - 1) & (~(sizeof(int) - 1)); */
  nbytes += sizeof(char *) - (nbytes & (sizeof(char *) - 1));
 
  p = gsptr;
/*    if ((gsptr += nbytes) - glbspace > GSPACE) */
  if ((gsptr += nbytes) > gblend)
    {
      fatal("out of global memory");
    }
  return (p);
}

#else
{
char * np;

  if (!(np = xmalloc(nbytes, &gsptr, glbspace + GSPACE)))
    fatal("out of global memory");
  return (np);
}

#endif

/*
  Lmalloc( nbytes )
  Allocate nbytes from the local space table.
*/
char * Lmalloc(int nbytes)
#ifndef help_1
{
char * p;

 
  nbytes = (nbytes + sizeof(int) - 1) & (~(sizeof(int) - 1));
 
  p = lsptr;
  if ((lsptr += nbytes) - locspace > LSPACE)
    {
      fatal("out of local memory");
    }
 
  if (lsptr - locspace > maxloc)
    maxloc = lsptr - locspace;
 
  return (p);
}

#else
{
char * np;

  if (!(np = xmalloc(nbytes, &lsptr, locspace + LSPACE)))
    fatal("out of local memory");
  return (np);
}

#endif

/*
  SizeOf( tarray )
  Compute size of object represented by type array.
*/
int SizeOf(char * tarray)
{
  struct hashent * p;

  switch (*tarray)
    {
    case T_CHAR:
    case T_UCHAR:
      return (1);
    case T_INT:
    case T_UINT:
    case T_PTR:
      return (2);
    case T_ARRAY:
      return (decode(tarray + 1) * SizeOf(tarray + 4));
    default:
      if (*tarray & T_STRUCT)
	{
	  /* jrd hacked this */

	  p = (struct hashent *) (((int) decode(tarray)) + (int) gblspace);
	  /* use offset, not addr */
 
	  return (p->data.g);
	}

      printf("unknown type: %02x\n", *tarray);
 
      fatal("unknown type code");
      return (0);
    }
}
/*
  PSizeOf(tptr)
  Compute size of pointer object.
*/
int PSizeOf(char * tptr)
{
  if (tptr[0] == T_ARRAY)
    tptr += 3;
  return (SizeOf(++tptr));
} 
/*
  dumpglbs()
  Dump global symbol table, for debugging.
*/
void dumpglbs()
{
struct hashent * psym;

  if (debug)
  {
    printf("\nGlobal Symbol Table\n");
    printf("===================\n");
    for (psym = glvptr; psym; psym = psym->link)
	  {
	    if (psym->flag.g & SC_STRUCT)
	      continue;
	    printf("%02x, ", psym->flag.g);
	    ptype(psym, psym->tptr.g);
	  }
  }
}

/*
  dumploc( psym )
  Dump local symbol table, for debugging.
*/
void dumploc(struct hashent * pfunc)
{
int i;
struct hashent * psym;

  if (!debug)
    return;
  printf("\nLocal Symbol Table for '%s'\n", pfunc->name);
  printf("==================================\n");
  for (i = 0; i < lovptr; ++i)
    {
      psym = lvtab[i];
      printf("%2d, %4d, ", psym->flag.l,psym->data.l);
      ptype(psym, psym->tptr.l);
    }
}


/*
  dumpnams()
  dump names of globals
*/
void dumpnams()
{
  struct hashent * psym;
  int sc;

  for (psym = glvptr; psym; psym = psym->link)
    {
      if ((sc = psym->flag.g) & SC_STRUCT)
	      continue;
      if (sc & SC_GLOBAL)
	      outgoe(psym->name);
	    else if (sc & SC_EXTERN)
	      outext(psym->name);
    }
}

 
/*
  dumpstruct()
  Dump structs/unions.
*/
void dumpstruct()
{
/*int i;*/
/* struct hashent * psym;*/

  if (!debug)
    return;
  printf("\nStruct/union Table\n");
  printf(  "==================\n");
}

 

/*
  encode( p, w )
  Encode p[0] and p[1] so that neither p[0] nore p[1] is zero
*/
void encode(char *p,int w)
{
 
/*  if (w & 0xFFFF0000)*/
  if ( w & 0xffe00000 ) // max. 21 bit
    {
      fprintf(stderr, "w = %X\n", w);
      fatal("information lost on encode");
    }
 
  p[2] = w | 0x80;
  p[1] = (w >> 7) | 0x80;
  p[0] = (w >> 14) | 0x80;
}

/*
  decode( p )
  Decode.
*/
int decode(register char * p)
{
  return ((p[2] & 0x7F) | ((p[1] & 0x7F) << 7) | ((p[0] & 0x03) << 14));
}
