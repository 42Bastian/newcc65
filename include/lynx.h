#include <global.h>
#include <runtime.h>

/* lynx specifc definitions */

#ifndef __LYNX__

#define __LYNX__

#define uchar unsigned char
#define uint  unsigned int
#define void  

#define MEMTOP		0xfe00
/* Note: MEMTOP is 0xfff0 if you don't use uploader */


/* ram shadows */
extern uchar _viddma,
             _sprsys,
             _iodat,
             _iodir;

/* hardware */

/* Suzy */
int   matha     at 0xfc52,
      mathb     at 0xfc54,
      mathc     at 0xfc56,
      mathd     at 0xfc52,
      mathe     at 0xfc60,
      mathakku  at 0xfc6c,
      mathrem   at 0xfc6c,
/*
  e = a * b
  e / b = (a:b) + mathrem
*/
      hoff      at 0xfc04,
      voff      at 0xfc06;
uchar *sprbase  at 0xfc08;   
uchar *collbase at 0xfc0a,
      *scbnext  at 0xfc10;
int   colloff   at 0xfc24;

uchar sprsys    at 0xfc92,
      joystick  at 0xfcb0,
      switches  at 0xfcb1,
      cart0     at 0xfcb2;
        
/* mikey */

struct _timer{ 
  int d1;
  uchar reload;
  uchar control;
  uchar count;
  uchar control2;
  };

struct _timer timer0    at 0xfd00, hbl_timer at 0xfd00,
              timer1    at 0xfd04,
              timer2    at 0xfd08, vbl_timer at 0xfd08,
              timer3    at 0xfd0c,
              timer4    at 0xfd10, uart_timer at 0xfd14,
              timer5    at 0xfd18,
              timer6    at 0xfd1c,
              timer7    at 0xfd20,
              timer[8]  at 0xfd00;

struct _audio{
  uchar volume;
  uchar feedback;
  uchar dac;
  uchar shiftlo;
  uchar reload;
  uchar control;
  uchar count;
  uchar other;
  };
  
struct _audio channelA    at 0xfd20,
              channelB    at 0xfd28,
              channelC    at 0xfd30,
              channelD    at 0xfd38,
              channels[4] at 0xfd20; 
             
uchar  attenA   at 0xfd40,
       attenB   at 0xfd41,
       attenC   at 0xfd42,
       attenD   at 0xfd43,
       atten[4] at 0xfd40,
       panning  at 0xfd44,
       mstereo  at 0xfd50,
       
       audin    at 0xfd86,
       sysctl1  at 0xfd87,
       mikeyrev at 0xfd88,
      mikeysrev at 0xfd89,
       iodir    at 0xfd8a,
       iodat    at 0xfd8b,
       serctl   at 0xfd8c,
       serdat   at 0xfd8d,
       sdoneack at 0xfd90,
       cpusleep at 0xfd91,
       dispctl  at 0xfd92, viddma at 0xfd92,
       pkbkup   at 0xfd93,
      *scrbase  at 0xfd94;
      
uchar *irqvec at 0xfffe;


/**************/
/* misc stuff */
/**************/

#define POKE(a,b) (*(char *)(a))=(b)
#define POKEW(a,b) (*(int *)(a))=(b)

#define PEEK(a) (*(char *)(a))

/* scb-macros */

#define SCBCTL0(a) (*(uchar *)(a))
#define SCBCTL1(a) (*(uchar *)((a)+1))
#define SCBCOLL(a) (*(uchar *)((a)+2))
#define SCBNEXT(a) (*(uint *)((a)+3))
#define SCBDATA(a) (*(uint *)((a)+5))
#define SCBX(a)    (*(int *)((a)+7))
#define SCBY(a)    (*(int *)((a)+9))
#define SCBHS(a)   (*(uint *)((a)+11))
#define SCBVS(a)   (*(uint *)((a)+13))
            
/* some assembler-macros */

#define CLI asm("\tcli")	/* enable Mikeys interrupt response */
#define SEI asm("\tsei")	/* disable it */

#define EnableIRQ(n)\
  asm(" lda #$80\n"\
      " tsb $fd01+"#n"*4\n")	/* enable interrupt of timer n */

#define DisableIRQ(n)\
  asm(" lda #$80\n"\
      " trb $fd01+"#n"*4\n") /* disable it */

#define BRK(n) asm(" brk "#n) /* 65C02-brk */

#define _SetRGB(adr)          /* set palette, faster then lynx.olb fn*/\
  asm(" ldy #31\n"\
      " lda _"#adr",y\n"\
      " sta $fda0,y\n"\
      " dey\n"\
      " bpl *-7")

#define WAITSUZY\
      asm(" bit\t$fc92\n"\
          " bmi *-3")

#define _POKE(a,b)\
      asm(" lda "#b"\n"\
          " sta "#a"\n");

#define _POKEW(a,b)\
      asm(" ldax "#b"\n"\
          " sta "#a"\n"\
          " stx "#a"+1");


#endif  /* __LYNX__ */
          
