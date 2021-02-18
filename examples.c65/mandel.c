#include <lynx.h>
#include <lynxlib.h>
#include <stdlib.h>


uchar screen[8160];

int mathe2 at 0xfc61;
/* fix-point with 8.8 */

uchar iter(r0,i0)
int r0,i0
{
  zstatic uchar h;
  zstatic int r1,r2,i1,i2;

  r1=r0;
  i1=i0;
  h = 15;
  do{
/* don't reverse this, because mathb hase to be written last ! */
    mathb = matha = r1;
    WAITSUZY; /* asm-macro, see lynx.h */
    r2 = mathe2;

    mathb = matha = i1;
    WAITSUZY;
    i2 = mathe2;

    if ( (r2 + i2) > 1024 ) return h;

/* start multiplication */
    matha = r1; mathb = i1;

/* let suzy do */
    r1 = r2 - i2 + r0;

/* ok, suzy should be ready now */
    i1 = (mathe2<<1) + i0;
  }while ( --h );
  return 0;
}

char pal[]={
0x00,0x0A,0x00,0x04,0x00,0x08,0x0A,0x0F,0x07,0x0F,0x00,0x0F,0x05,0x09,0x05,0x0F,
0x00,0xAA,0x0A,0x0F,0xA0,0xF0,0x0F,0x0F,0x00,0x00,0xFF,0xF0,0x0A,0x0E,0x50,0xFF
};

main()
{
 zstatic uchar x,y; // placed in BSS

 zstatic int rmin,imin,idelta,rdelta;

  InitIRQ();
  InstallUploader(_62500Bd);
  CLI;

/* set screen-base */
  SetBuffers(screen,(char *) 0,(char *) 0);

  sprsys = _sprsys |= 0x80; /* signed math */

  idelta = rdelta = 6;
  
  _SetRGB(pal); /* asm-macro not lib-function */

  for(;;)
  {
    DrawFBox(0,0,160,102,0);

    for( y = 0 , imin = -300 ;  102 > y ; ++y,imin += idelta )
                            /*  ^^^^^^^^ strange,uh ?. But xopt likes it ! */
      for ( x = 0 , rmin = -600;  160 > x ; ++x,rmin += rdelta )
        SetPixel(x,y,iter(rmin,imin));
        
    while (!joystick) ;

    --idelta;
  }
}


