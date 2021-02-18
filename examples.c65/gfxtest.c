/* simple program to test the GFX-lib                   */
/* written by 42Bastian Schick (42BS)                   */
/* last modified :                                      */
/* 97/07/09     42BS    created                         */
/* 97/10/12     42BS    changed for optimized code      */
/* 98/08/15     42BS    replace register by zstatic     */
/*                                                      */

#include <lynx.h>
#include <lynxlib.h>

/* __ALWAYS__ use both defines but using is recommended for shorter code */
#define FIXARGC         /* be sure you don't use <8 arguments ! */
#define NOARGC

#define SCREEN (MEMTOP-8160)

/* fill the screen with a pattern */

struct point
  {
  uchar x,y;
  };
  
struct box
  {
  struct point a;  
  uchar w,h;
  };

MakePattern()
{
zstatic struct point p;
                
  for (p.x = 0; 160 > p.x ; ++p.x)
    for (p.y = 0; 102 > p.y ; ++p.y)
      SetPixel(p.x,p.y,p.x^p.y);
}

Lines()
{
zstatic uchar x,y;

  for (x = 0; 160 > x ; ++x)
    DrawLine(x,0,159-x,101,x);
  for (y = 0; 102 > y ; ++y)
    DrawLine(0,y,159,101-y,y); 
}

DrawBox(b,c)
struct box *b;
uchar c;
{
  DrawFBox(b->a.x,b->a.y,b->w,b->h,c);
}

Boxes()
{
  zstatic uchar x;
  zstatic struct box b;
  for ( x = 0 ; 51 > x  ; ++x)
  {
    b.a.x = x;
    b.a.y = x;
    b.w = 160-(x<<1);
    b.h = 102-(x<<1);
    DrawBox(&b,x & 0xf);
   }
}

/* wait for a button */

char WaitButton()
{
zstatic uchar key;
  while ( !(key = joystick))
   ;
/* wait for the button to be released */
  while ( joystick )
    ;
  return key;
}


doit()
{
zstatic uchar c;

  do{
  c = (c+1) & 3;
  }while (3 == c);

  switch (c)
  {
  case 0:
    MakePattern(); break;
  case 1:
    Lines(); break;
  case 2:
    Boxes(); break;
  }
  WaitButton();
  DrawFBox(0,0,160,102,0);
}

main()
{
 InitIRQ();
 InstallUploader(_62500Bd);
 CLI;

/* set up buffer : display buffer == render buffer, no collision buffer */
  SetBuffers(SCREEN,(char *)0,(char *)0 );

/* clear the screen */
  DrawFBox(0,0,160,102,15);

/* set the palette */
  _SetRGB(DefaultPAL);

 for ( ; ; )
   doit();

}
