/* cc65 stdio.h */

/* say we're running cc65 */
#define M6502
#define __CC65__

/* Lynx doesn't do files (yet) */
#if 0
#define FILE char
extern FILE * stdin;
extern FILE * stdout;
extern FILE * stderr;
extern int errno;    /* error number from open, etc */
#endif

#define EOF -1
#define NULL 0
#define NEWLINE 0x9B  /* what is this crap? */
#define YES 1
#define NO 0
