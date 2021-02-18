# slidly changed to work with gmake 42BS

LD = link65
CC = cc65
AS = ra65
RM = rm -f

.SUFFIXES: .c65 .obj .m65

.m65.obj:	
	$(AS) -o $@ $<


all : demo0.com demo1.com demo2.com demo3.com mazedemo.com sprite.com 
	echo
	echo "Don't forget to press the joypad to begin the demos."
	echo

demo0.com : demo0.obj
	$(LD) -b200 -s512 -o demo0.com demo.obj

demo1.com : demo1.obj random.obj pixel.obj
	$(LD) -b200 -s512 -o demo1.com demo.obj random.obj pixel.obj

demo2.com : demo2.obj random.obj pixel.obj
	$(LD) -b200 -s512 -o demo2.com demo.obj random.obj pixel.obj

demo3.com : demo3.obj random.obj pixel.obj
	$(LD) -b200 -s1024 -o demo3.com demo.obj random.obj pixel.obj

mazedemo.com : mazedemo.obj random.obj pixel.obj maze.obj
	$(LD) -b200 -s512 -o mazedemo.com mazedemo.obj random.obj pixel.obj maze.obj

sprite.com : sprite.obj
	$(LD) -b200 -s512 -o sprite.com sprite.obj

clean :  
	$(RM) *.obj
	$(RM) *.com

demo0.obj : demo.c65
	$(CC) -DDEMO0 -O demo.c65
	$(AS) demo.m65
	$(RM) demo.m65

demo1.obj : demo.c65
	$(CC) -DDEMO1 -O demo.c65
	$(AS) demo.m65
	$(RM) demo.m65

demo2.obj : demo.c65
	$(CC) -DDEMO2 -O demo.c65
	$(AS) demo.m65
	$(RM) demo.m65

demo3.obj : demo.c65
	$(CC) -DDEMO3 -O demo.c65
	$(AS) demo.m65
	$(RM) demo.m65
	
sprite.obj : sprite.c65 sprite.h test.h
	$(CC) -O sprite.c65
	$(AS) sprite.m65
	$(RM) sprite.m65

mazedemo.obj : mazedemo.c65
	$(CC) -O mazedemo.c65
	$(AS) mazedemo.m65
	$(RM) mazedemo.m65
 
