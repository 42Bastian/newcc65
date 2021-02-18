
LD = link65 -m 
CC = cc65
AS = ra65
RM = rm -f
CFLAGS=

LIBS = lynx.olb c.olb

.SUFFIXES: .c65 .obj .m65

.m65.obj:	
	$(AS) -o $@ $<


all : demo0.com demo1.com demo2.com demo3.com mazedemo.com sprite.com 
	@echo
	@echo "Don't forget to press the joypad to begin the demos."
	@echo

demo0.com : demo0.obj image.obj
	$(LD) -b200 -s512 -o demo0.com demo.obj image.obj $(LIBS)

demo1.com : demo1.obj
	$(LD) -b200 -s512 -o demo1.com demo.obj $(LIBS)

demo2.com : demo2.obj
	$(LD) -b200 -s512 -o demo2.com demo.obj $(LIBS)

demo3.com : demo3.obj
	$(LD) -b200 -s1024 -o demo3.com demo.obj $(LIBS)

mazedemo.com : mazedemo.obj maze.obj
	$(LD) -b200 -s512 -o mazedemo.com mazedemo.obj maze.obj $(LIBS)

sprite.com : sprite.obj
	$(LD) -b200 -s512 -o sprite.com sprite.obj $(LIBS)

clean :  
	$(RM) *.obj
	$(RM) *.com
	$(RM) *.spr
	$(RM) *.bak
	$(RM) *.map

demo0.obj : demo.c65
	$(CC) $(CFLAGS) -DDEMO0 demo.c65
	xopt demo.m65
	$(AS) demo.m65
	$(RM) demo.m65

demo1.obj : demo.c65
	$(CC) $(CFLAGS) -DDEMO1 demo.c65
	xopt demo.m65
	$(AS) demo.m65
	$(RM) demo.m65

demo2.obj : demo.c65
	$(CC) $(CFLAGS) -DDEMO2 demo.c65
	xopt demo.m65
	$(AS) demo.m65
	$(RM) demo.m65

demo3.obj : demo.c65
	$(CC) $(CFLAGS) -DDEMO3 demo.c65
	xopt demo.m65
	$(AS) demo.m65
	$(RM) demo.m65
	
sprite.obj : sprite.c65 sprite.h test.h
	$(CC) $(CFLAGS)  sprite.c65
	xopt sprite.m65
	$(AS) sprite.m65
	$(RM) sprite.m65

mazedemo.obj : mazedemo.c65
	$(CC) $(CFLAGS) mazedemo.c65
	xopt mazedemo.m65
	$(AS) mazedemo.m65
	$(RM) mazedemo.m65
	
image.obj : image.raw
	sprpck -t0 -p0 -i160206 image.raw image
