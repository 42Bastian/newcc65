
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* defs for library files

#ifndef _TYPES_
#include "types.h"
#endif

A library file consists of a header lump, as described below, some number
of module descriptors, and images of files.

magic number identifies library files */
#define LIBR_HEADER 0xFCFC

struct librfile
	{
	USHORT l_header;	/* magic num, as above */
	USHORT type;		/* type of library, values below */
	USHORT n_modules;	/* number modules this library */
	USHORT l_flags;		/* (l_flags) library flags, defined below */
	};

/* library types */
#define LT_OLB	1			/* an object library */

/* flags for object libraries */
#define LOLB_DEF	0x0001		/* library contains a symbol dictionary */

/* a module of the library */
struct librmod
	{
	char m_name[12];		/* entry name, typically file name */
	USHORT m_nbytes;		/* n bytes in entry */
	};

