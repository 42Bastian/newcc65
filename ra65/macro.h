// header-file for the macro-features of ra65
// macro-parameters:
// \0 .. \f      - normal
// \#            - number of parameters passed
// \?            - name of the macro
// \^            - expansion-counter

#define MACROHEAPSIZE (1024*128)
#define MACROHASHSIZE 128           // must be power of 2 !
#define MACROEXPANDBUFFER (1024*2)

struct macroS
{
  struct macroS * next; // next macro for the hash-table
  short filenum;     // where is the definition : file
  short linenum;     //                         : line
  short paranum;     // how many parameters are expected
  short count;       // how many expansions
  char name[32+2];
  char macro[1];      // dummy, macro-text follows
};

struct macroS * macroHashTab[MACROHASHSIZE];

char *macroCurrName;
short macroCurrFile;
short macroCurrLine;
short macroCurrParameters;
char ** macroCurrPtr;      // ptr to ptr to current macro-line

char * macroHeapBase;
char * macroHeapNext;
char * macroHeapEnd;

int processing_macro;
