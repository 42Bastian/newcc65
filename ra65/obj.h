
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* defs for relocatable object files */

#ifndef _TYPES_
#include "types.h"
#endif

#define OBJ_HEADER	0xFDFD
#define NEW_OBJ_HEADER	0xF0F0

struct relfile
	{
	USHORT header;		/* the above header value */
	USHORT nb_sym;		/* n bytes of symbol table this file */
	USHORT nb_seg;		/* nbytes this segment occupies */
	USHORT nb_segdata;	/* n bytes of segment data this file */
	USHORT n_sym;		/* n symbols */
	
	USHORT obj_data_size;
	USHORT data_size;
	USHORT bss_size;
	USHORT bsszp_size;
	};

/* mask for looking at op bytes */
#define OP_GEN_MASK	0xE0
/* values for op flds */

#define OP_LIT		0x00	/* literal bytes follow */
#define OP_REL		0x20	/* rel word follows */
#define OP_REL_HI	0x21	/* rel byte hi follows */
#define OP_REL_LO	0x22	/* rel byte lo follows */
// rel. to data-seg
#define OP_DREL	  	0x24	/* rel word follows */
#define OP_DREL_HI	0x25	/* rel byte hi follows */
#define OP_DREL_LO	0x26	/* rel byte lo follows */
// rel. to bss-seg
#define OP_BREL		  0x28	/* rel word follows */
#define OP_BREL_HI	0x29	/* rel byte hi follows */
#define OP_BREL_LO	0x2a	/* rel byte lo follows */

#define OP_SYM_MASK	0xC0
#define OP_SYM_EMASK	0x20	/* if sym num is extended in next byte */
#define OP_SYM		0x40	/* next word is relative to sym num */
#define OP_SYM_HI	0x80	/* gen hi byte of resultant value */
#define OP_SYM_LO	0xC0	/* .. lo byte */
