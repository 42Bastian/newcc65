#include <lynx.h>
#include <lynxlib.h>

extern char titelSCB[];
#asm
          xref _title
          
_titelSCB dc.b $c0,$10,$20
          dc.w 0,_title
          dc.w 0,0,$100,$100
          dc.b $01,$23,$45,$67,$89,$Ab,$cd,$ef
#endasm

#include "title.pal"

main()
{
register uchar x,y,c;

//  InitIRQ();
//  InstallUploader(_62500Bd);
  CLI;
  SetBuffers(0x9000,0,0);

  _SetRGB(pal);
  DrawSprite(titelSCB);
    
  for(;;);

}
