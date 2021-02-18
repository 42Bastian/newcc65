
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* obj file dumper */

#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#ifdef UNIX
#  include <unistd.h>
#endif

#include "symtab.h"
#include "obj.h"


#ifndef O_BINARY
#define O_BINARY 0
#endif

int obj_fd;
struct relfile rf;
SYM ** syms;
int eof = 0;
int nbytes_read;  

char litbuf[32];

USHORT read8()
{
  static unsigned char foo;
  if (read(obj_fd, &foo, 1) != 1)
	  eof = 1;
  else
	  nbytes_read++;
  return((USHORT)foo);
}

USHORT read16()
{
  return(read8() | (read8() << 8));
}

SYM * readsym()
{
  int len;
  struct sym * sym;
  len = (int)read8();

  sym = (SYM *)malloc(sizeof (SYM) + len);
  read(obj_fd, sym->name, len);
  sym->name[len] = '\0';
  sym->value = read16();
  sym->flags = read16();
  return(sym);
}


int main(argc, argv)
int argc;
char ** argv;
{
  int i, j, pc;
  char op;
  
  if (argc < 2)
	{
	printf("Usage: %s <object-file>\n", argv[0]);
	exit(1);
	}
  obj_fd = open(argv[1], O_RDONLY|O_BINARY);
  if (obj_fd < 0)
  	{
	printf("can't open %s\n", argv[1]);
	exit(1);
	}
  rf.header = read16();
  rf.nb_sym = read16();
  rf.nb_seg = read16();
  rf.nb_segdata = read16();
  rf.n_sym = read16();
  printf("header %X\n", rf.header);
  if (rf.header != OBJ_HEADER && rf.header != NEW_OBJ_HEADER)
  {
	  printf("%s is not an object file\n", argv[1]);
    close(obj_fd);
    exit(1);
	}
	if (rf.header == NEW_OBJ_HEADER)
	{
	  rf.obj_data_size = read16();
	  rf.data_size = read16();
	  rf.bss_size = read16();
	  rf.bsszp_size = read16();
	}
	else
	{
	  rf.obj_data_size =
	  rf.data_size = 
	  rf.bss_size  =
	  rf.bsszp_size = 0;
	}
	  
  printf("Object file %s:\n", argv[1]);
  printf("  %d symbols in %d bytes\n", rf.n_sym, rf.nb_sym);
  printf("  $%04x bytes text-segment in %d bytes\n", rf.nb_seg, rf.nb_segdata);
  printf("  $%04x bytes data-segment in %d bytes\n", rf.data_size, rf.obj_data_size);
  printf("text  size %5d bytes\n"
         "data  size %5d bytes\n"
         "bss   size %5d bytes\n"
         "bsszp size %5d bytes\n",rf.nb_seg,rf.data_size,rf.bss_size,rf.bsszp_size);
         
/* allocate symbol vector */
  syms = (struct sym ** )malloc(rf.n_sym * sizeof (struct sym *));
/* read them in */
  for (i = 0 ; i < rf.n_sym ; i++)
  {
	  syms[i] = readsym();
	  printf("sym:%-32s flags %02x value : $%04x\n",syms[i]->name,syms[i]->flags,syms[i]->value);
  }

/* now dump data */
  i = 0;			/* our index in seg data */
  pc = nbytes_read = 0;
  while (pc < rf.nb_seg + rf.data_size)
	{
	printf("%04x: ", pc);
	op = read8() & 0xff;
	
	switch (op & OP_GEN_MASK)
		{
		case OP_LIT:
			{
			if (op == 0)
				op = 32;
				
			printf("Literal: %2d bytes\n\t", op);
			read(obj_fd, litbuf, op);
			nbytes_read += op;
			for (j = 0 ; j < op ; j++)
				printf(" %02x", litbuf[j] & 0xFF);
			printf("\n");
			
			pc += op;
			break;
			}
		case OP_REL:
			{
			switch(op & 0x23)
				{
				case OP_REL:
					{
					printf("Word:    %04x + segment base", read16());
					pc += 2;
					break;
					}
				case OP_REL_HI:
					{
					printf("Hi byte: %04x + segment base", read16());
					pc += 1;
					break;
					}
				case OP_REL_LO:
					{
					printf("Lo byte: %04x + segment base", read16());
					pc += 1;
					break;
					}
				}
				switch (op & 0xc)
				{
				  case 0: printf(" in TEXT\n"); break;
				  case 4: printf(" in DATA\n"); break;
				  case 8: printf(" in BSS\n");  break;
				  case 12:printf(" in BSSZP\n");break;
				}
			break;
			}
		default:		/* must be a sym */
			{
			j = op & 0x1F;
			if (op & OP_SYM_EMASK)
				j = (j << 8) | read8();

		  if (j > rf.n_sym)
		  {
    		  printf(" wrong symbol #\n");
    		  break;
		  }
		
			switch (op & OP_SYM_MASK)
				{
				case OP_SYM:
					{
					printf("Word:    %04x + %s\n", read16(),
						syms[j]->name);
					pc += 2;
					break;
					}
				case OP_SYM_HI:
					{
					printf("Hi byte: %04x + %s\n", read16(),
						syms[j]->name);
					pc += 1;
					break;
					}
				case OP_SYM_LO:
					{
					printf("Lo byte: %04x + %s\n", read16(),
						syms[j]->name);
					pc += 1;
					break;
					}
				}
			break;
			}
		}
	}
  i = read8();
  if (!eof)
	{
	printf("Extra bytes at eof!");
	printf("  %02x\n", i);
	for ( ; !eof ; )
		{
		i = read8();
		if (!eof)
			printf("  %02x\n", i);
		}
	}
  if (nbytes_read != rf.nb_segdata+rf.obj_data_size)
  	printf("Byte count mismatch: header says %d bytes, read %d\n",
        		rf.nb_segdata+rf.obj_data_size, nbytes_read);
  close(obj_fd);
  return 0;
}


