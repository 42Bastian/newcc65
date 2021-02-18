  
/* cclex.h - token definitions */

#define CEOF 0

/* #define ASM		40 */
#define CASE		   41
#define DEFAULT		42
#define BREAK		  43
#define CONTINUE	44
#define ELSE		   45
/* what the fuck's this? #define ENTRY		46 */

#define DO		     30
#define FOR		    31
#define GOTO		   32
#define IF		     33
#define RETURN		 34
#define SWITCH		 35
#define WHILE		  36

#define SIZEOF   100
#define IDENT    50
#define SEMI     51
/*
 * primary operators
 */
#define LBRACK		 52
#define LPAREN		 53
#define DOT		    54
#define PREF		   55

#define LCURLY		 56
#define RBRACK 		57
#define COMP		   58
#define INC		    59
#define PASGN		  60
#define PLUS		   61
#define COMMA		  62
#define DEC		    63
#define SASGN		  64
#define RCURLY		 65
#define MINUS		  66
#define MASGN		  67
#define STAR		   68
#define DASGN		  69
#define DIV 		   70
#define DAMP 		  71
#define AASGN 		 72
#define AMP 		   73
#define NE 		    74
#define BANG 		  75
#define DBAR 		  76
#define OASGN 		 77
#define BAR 		   78
#define EQ 		    79
#define ASGN 		  80
#define SLASGN 		81
#define ASL 		   82
/*
 * inequalities
 */
#define LE		    83
#define LT		    84
#define GE		    85
#define GT		    86

#define SRASGN    87
#define ASR       88
#define XOASGN    89
#define XOR       90
#define MOASGN    91
#define MOD       92
#define QUEST     93
#define COLON     94
#define RPAREN    95
#define SCONST    96
#define ICONST    97
#define CCONST    98
#define FCONST    99

#define CHAR		  20
#define INT		    21
#define DOUBLE		22
#define FLOAT		  23
#define LONG		  24
#define UNSIGNED	25
#define SHORT		  26
#define STRUCT		27
#define UNION		  28
#define VOID		  29
#define ENUM		  101

#define FNAME		  102
#define LINENO		103

#define AUTO		  10
#define EXTERN		11
#define REGISTER	12
#define STATIC		13
/* never used?
#define TYPEDEF		14
*/

#define IRQ		  104
#define ASM2		 105
#define AT     106
#define ZSTATIC 107

#define ERROR		(-1)

