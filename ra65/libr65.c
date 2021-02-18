/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.

   Modified for use on *nix machines -Intruder  1993

*/
/* 97/01/31	42BS	rmoved native stuff and O_BINARY */
/* 97/05/29	42BS	added O_BINARY for MSDOS again   */ 

/* object library maintainer for atari 8-bit .obj files.
   later, maybe random other kinds of files too. */

#define VERSION "1.1.1 [*nix]"

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#ifdef UNIX
#  include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h> 		/* for _dta, for file size */
 
#define USHORT unsigned short
#ifndef O_BINARY
#  define O_BINARY 0
#endif

#include "symtab.h"
#include "obj.h"
#include "libr.h"

void barf(char *msg, char *arg1, char *arg2, char *arg3);
int min(int x,int y);
unsigned char read8(int input_fd);
USHORT read16(int input_fd);
void write8(char ch);
void write16(USHORT sh);
int open_carefully(char *name,int mode,int must_exist);
void read_header(int fd);
void read_mdesc(int fd,struct librmod *  mod);
void build_modlist(int fd);
void string_upcase(char *str);
int string_equal(char * str1,char * str2);
void string_pad(char *target,char * source,int nbytes);
void string_trim(char * target,char * source,int nbytes);

char * lfname;			/* name of the file we've got open */
int in_lfd;			/* fd of the file we've got open */
int out_lfd;			/* fd output, if writing */
int obj_lfd;			/* fd for .obj files */

struct librfile lfile;		/* header of the file we've got open */

int input_fd;			/* fd of obj files we read */

/* defs for keeping track of what we're doing when maintaining a library */

/* we build a linked list of these when adding/deleting etc.  If we
   were writing this in a real language, we'd do this with included
   defstructs, but since we must use C, we make the first part of
   this look just like the def of a librmod, in libr.h.  */

struct entry
	{
	char e_name[12];	/* the file name of this entry */
	USHORT e_nbytes;	/* nbytes this entry */
	struct entry * e_next;	/* next entry */
	int new;		/* if this is a new entry */
	int skip;		/* this is an old entry being skipped */
	};

#define SKIP_REPLACE	1
#define SKIP_DELETE	  2

char * skip_names[] =
	{
	"", "Replacing", "Deleting"
	};

struct entry * lfile_components;

/* util routines */

int min(int x,int y)
{
  if (x < y) 
    return(x);
   else
    return(y);
}

unsigned char read8(int input_fd)
{
unsigned char foo;
  read(input_fd, &foo, 1);
  return(foo);
}

USHORT read16(int input_fd)
{
  return(read8(input_fd) | (read8(input_fd) << 8));
}

void write8(char ch)
{
  write(out_lfd, &ch, 1);
}

void write16(USHORT sh)
{
  write8(sh & 0xFF);
  write8(sh >> 8);
}

int open_carefully(char *name,int mode,int must_exist)
 
{
  int fd;
  
  /* printf("open('%s', %x)->", name, mode);*/
  fd = open(name, mode|O_BINARY, 0644);
  if ((fd < 0) && must_exist)
  	barf("Can't open '%s'\n", name, "", "");
 /* printf("%d\n", fd);*/

  return(fd);
}

void read_header(int fd)
{
  lfile.l_header = read16(fd);
  lfile.type = read16(fd);
  lfile.n_modules = read16(fd);
  lfile.l_flags = read16(fd);
 
}

void read_mdesc(int fd,struct librmod *  mod)
{
  read(fd, mod->m_name, 12);
  mod->m_nbytes = read16(fd);
}

/* build the module list for an existing file.  assume the next byte 
   to be read is the first one of the module structs.  assume lfile
   is valid */
void build_modlist(int fd)
{
  int i;
  struct entry * elt;
  struct entry * prev_elt;

  lfile_components = elt = (struct entry * )NULL;
 
  for (i = 0 ; i < lfile.n_modules ; i++)
	{
	prev_elt = elt;
 
	elt =(struct entry *)malloc(sizeof(struct entry));

	bzero(elt,sizeof(struct entry)); /* 42BS 97/12/11 */
  
	read(fd, elt->e_name, 12);
	elt->e_nbytes = read16(fd);

	elt->new = 0;
	elt->skip = 0;
	elt->e_next = NULL; /* 42BS 1997/05/01 */

	if (prev_elt)
		prev_elt->e_next = elt;
	    else
		lfile_components = elt;
	}	
}

void string_upcase(char *str)
{
  for ( ; *str ; str++)
	if (islower(*str))
		*str = toupper(*str);
}

int string_equal(char * str1,char * str2)
{
  for ( ; *str1 || *str2 ; str1++, str2++)
	if (*str1 != *str2)
		return(0);
	return(1);
}
/* not needed?
char * string_copy(str)
char * str;
{
  char * t = (char * )malloc(strlen(str) + 1);
  strcpy(t, str);
  return(t);
}
*/

void string_pad(char *target,char * source,int nbytes)
{
  int i;
  
  for (i = nbytes ; (i > 0) && (*source) ; i--)
	*target++ = *source++;
  for ( ; i > 0 ; i--)
	*target++ = ' ';
}

void string_trim(char * target,char * source,int nbytes)
 
{
  for ( ; ((nbytes > 0) && (*source != ' ')) ; nbytes--)
	*target++ = *source++;
  *target = '\0';
}

/* maintaining entry list */

struct entry * last_entry()
{
  struct entry * elt;
  
  for (elt = lfile_components ; elt && elt->e_next ; elt = elt->e_next)
	;
  return(elt);
}

/* delete an entry.  return 1 if found it */

int del_entry(name, why)
char * name;
int why;
{
  struct entry * elt;
  char buf[80];
  int found;

  found = 0;
  /* string_upcase(name); */
  for (elt = lfile_components ; elt ; )
	{
	string_trim(buf, elt->e_name, 12);
	if (string_equal(name, buf))
		{
		/* set skip bit in this one; we always add new 
		   entries at the end */
		elt->skip = why;
		found++;
		}
	if (elt)
		elt = elt->e_next;
	}
  return(found);
}

/* return 1 if replaced old entry */
int add_entry(name, nbytes)
char * name;
int nbytes;
{
  struct entry * elt;
  struct entry * prev_elt;
  int replaced;

  prev_elt = (struct entry *)NULL;

  replaced = del_entry(name, SKIP_REPLACE);
  prev_elt = last_entry();

  /* now make the new one */
  elt =	(struct entry *)malloc(sizeof(struct entry));

  bzero(elt, sizeof(struct entry)); /* 42BS 97/12/11 */

/*  elt->e_name = string_copy(name); */
  string_pad(elt->e_name, name, 12);
  elt->skip = 0; 			/* 42BS */
  elt->new = 1;				/* get this from file, not libr */
  elt->e_nbytes = nbytes;
  elt->e_next = NULL; 			/* 42BS */
  if (prev_elt)
    prev_elt->e_next = elt;
  else
    lfile_components = elt;
  return(replaced);
}

/* on 800, this will have to read and count the whole file,
   unless we're using Spartados */

int file_nbytes(name)
char * name;
{

  struct stat st;
  
  if (stat(name, &st) < 0)
    return(-1);
  else
    return(st.st_size);
}

/* zzz */

 
void barf(char *msg, char *arg1, char *arg2, char *arg3)
{
  fprintf(stderr, (char *)msg, arg1, arg2, arg3);
  fprintf(stderr, "\n");
  exit(1);
}
 

/* open libr and generate the initial components list */
void read_library(must_exist)
int must_exist;
{

  lfile_components = (struct entry * )NULL;
 
  in_lfd = open_carefully(lfname, O_RDONLY, must_exist);

  if (in_lfd > 0)			/* open it ok? */
	{
	read_header(in_lfd);		/* read header */
	build_modlist(in_lfd);
	return;
	}
    else
	{
	if (must_exist)
		barf("Can't open %s", lfname, "", "");
	    else
		{
		fprintf(stderr, "Creating library %s\n", lfname);
		in_lfd = 0;
/*		lfile_components = (struct entry * )NULL;*/
		}
	}
}

/* using the components_list, write a new library */

void write_library()
{
  struct entry * elt;
  int copy_lfd;
  char *buf;

  if ( (buf = malloc(65536)) == NULL)
  {
    printf("Out of Memory !\n");
    exit(-1);
  }
  
  out_lfd = open_carefully("libr65.tmp", O_WRONLY | O_CREAT | O_TRUNC, 0);

  lfile.l_header = LIBR_HEADER;
  lfile.type = LT_OLB;		/* fixed, for now */
  lfile.l_flags = 0;
/* refresh module count */
  lfile.n_modules = 0;
  for (elt = lfile_components ; elt != NULL ; elt = elt->e_next)
	  if (!elt->skip)			/* not skipping this one? */
	  	lfile.n_modules++;	/* then count it */
	  	
/* write the header */
  write16(lfile.l_header);
  write16(lfile.type);
  write16(lfile.n_modules);
  write16(lfile.l_flags);

/* write the dictionary */
  for (elt = lfile_components ; elt != NULL ; elt = elt->e_next)
	if (!elt->skip)
		{
		write(out_lfd, elt->e_name, 12);
		write16(elt->e_nbytes);
		}

/* write the modules themselves */
  for (elt = lfile_components ; elt != NULL ; elt = elt->e_next)
	{
	string_trim(buf, elt->e_name, 12);
	if (elt->skip)
		{
		/* skip this entry in the original library */
		fprintf(stderr, "%s %s\n", skip_names[elt->skip], buf);
		lseek(in_lfd,elt->e_nbytes,SEEK_CUR);
		}
	    else
		{
		if (elt->new)
    {
			/* new file.  open the obj file */
			fprintf(stderr, "Adding %s\n", buf);
			obj_lfd = open_carefully(buf, O_RDONLY, 1);
			copy_lfd = obj_lfd;
			}
      else
    {
			fprintf(stderr, "Copying %s\n", buf);
			copy_lfd = in_lfd;
    }
		/* run the copy loop */
		  bzero(buf,65536);
		  read(copy_lfd,buf,elt->e_nbytes);
		  write(out_lfd,buf,elt->e_nbytes);
		/* if have an obj file open, close it */
		if (obj_lfd > 0)
			{
			close(obj_lfd);
			obj_lfd = 0;
			}
		}
	}
  /* copy writing.  close libr file */
  close(out_lfd);
/*  printf("closing in_lfd %d\n", in_lfd); */
  if (in_lfd > 0)
  	close(in_lfd);
  unlink(lfname);
  rename("libr65.tmp", lfname); 
/*  printf("del '%s'->%x\n", lfname, unlink(lfname)); */
/*  printf("ren '%s','%s'->%x\n", "libr65.tmp", lfname,rename("libr65.tmp", lfname)); */

  out_lfd = 0;
/*  printf("New library written\n"); */
  free(buf);
}
	
/* routines to really do things */

void add_files(argc, argv)
int argc;
char ** argv;
{
  int i, nbytes, found;
  
  if (argc <= 3)
	barf("Add what files?", "", "", "");
  found = 0;
  read_library(0);		/* read the library */
  for (i = 3 ; i < argc ; i++)
	{
	nbytes = file_nbytes(argv[i]);
	if (nbytes < 0)
		fprintf(stderr, "%s not found", argv[i]);
	    else
		{
		found++;
		add_entry(argv[i], nbytes);
		}
	}
  if (found == 0)
	fprintf(stderr, "No files added");
    else
	write_library();
/*  printf("Finished adding files\n");*/
}

/* delete some library members */
void del_files(argc, argv)
int argc;
char ** argv;
{
  int i, deleted;
  
  if (argc <= 3)
	barf("Delete what files?", "", "", "");
  read_library(1);
  for (i = 3, deleted = 0 ; i < argc ; i++)
	if (del_entry(argv[i], SKIP_DELETE))
		deleted++;
	    else
		fprintf(stderr, "Library contains no member %s\n", argv[i]);
  if (deleted)
	write_library();
    else
	fprintf(stderr, "No members matched");
   close(in_lfd);
}

void list_library()
{
  struct entry * elt;
  char buf[14];
  
  read_library(1);
  close(in_lfd);
  printf("  Library %s\n", lfname);
  printf("  module        size\n");
  printf("  ------------  -----\n");
  for (elt = lfile_components ; elt  ; elt = elt->e_next)
	{
	  bcopy(elt->e_name,buf,12);
    buf[12] = '\0';
    printf("  %12s  %5d\n", buf, elt->e_nbytes);
	}
}

void print_version()
{
  printf("Libr65 v %s\n", VERSION);
}

/* main body */


int main(argc, argv)
int argc;
char ** argv;
{
  char op;			/* what we're doing to a file */


  if (argc < 3)
	barf("Try libr65 <A D L V> <file>", "", "", "");

  op = *argv[1];
  lfname = argv[2];

  switch (op)
	{
	case 'a': case 'A':
		add_files(argc, argv);
		break;
	case 'd': case 'D':
		del_files(argc, argv);
		break;
	case 'l': case 'L':
		list_library();
		break;
	case 'v': case 'V':
		print_version();
		break;
	default:
		barf("I don't understand option '%c'", (char *)(int)op, "", "");
	}
  exit(0);
}

